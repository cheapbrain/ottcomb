#include <windef.h>

extern "C" {
#define bool BOOL
#include "mongoose.h"
#undef bool
}

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <istream>
#include <memory>
#include <iomanip>

#include <stdio.h>
#include <windows.h>

#define main _main1
#include "hopcroft.cpp"
#undef main

using namespace std;

vector<string> split(const string& str, char delim = ' ') {
	vector<string> out;
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		out.push_back(token);
	}
	return out;
}

unordered_map<string,string> parseQuery(const string &query) {
	unordered_map<string,string> out;
	const vector<string> args = split(query, '&');

	for (auto &arg: args) {
		const vector<string> kv = split(arg, '=');
		if (kv.size() != 2) continue;
		out[kv[0]] = kv[1];
	}
	return out;
}

bool checkArgs(const unordered_map<string,string> &args, const vector<string> &necessary) {
	for (auto &s: necessary) {
		if (args.find(s) == args.end()) {
			return false;
		}
	}
	return true;
};

std::string format(const char *fmt, ...)
{
    char buf[256];

    va_list args;
    va_start(args, fmt);
    const auto r = std::vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

    if (r < 0) return {};

    const size_t len = r;
    if (len < sizeof buf) return { buf, len };

    auto vbuf = std::unique_ptr<char[]>(new char[len+1]);
    va_start(args, fmt);
    std::vsnprintf(vbuf.get(), len+1, fmt, args);
    va_end(args);

    return { vbuf.get(), len };
}

static struct mg_serve_http_opts server_opts;

#define ARGS_NONE ""
#define ARGS_JSON "Content-Type: application/json\r\nCache-Control: no-store, must-revalidate"

static void render(struct mg_connection *conn, const string &data, const string &headers = ARGS_JSON, int status = 200) {
	mg_send_head(conn, status, data.size(), headers.size() ? headers.c_str() : NULL);
  mg_send(conn, data.c_str(), data.size());
}

static void event_handler(struct mg_connection *conn, int ev, void *p) {
	if (ev == 0) return;

	if (ev == MG_EV_HTTP_REQUEST) {
		auto msg = (struct http_message *)p;

		string uri(msg->uri.p, msg->uri.len);
		string query(msg->query_string.p, msg->query_string.len);

		if (uri == "/api") {
			auto args = parseQuery(query);

			if (!checkArgs(args, {"action", "s", "t", "e"})) {
				render(conn, "Missing arguments", ARGS_NONE, 400);
				return;
			}

			auto action = args["action"];
			int S = stoi(args["s"]);
			int T = stoi(args["t"]);
			int E = stod(args["e"]);

			Graph g;
			initRandom(g, S, T, E);

			vector<int> m = hopcroft(g);

			ostringstream out;
			out << format("{\"nodes\":[");
			for (int i = 0; i < g.N; i++) {
				if (i > 0) out << ",";
				bool left = !g.nodes[i].side;
				int x = left ? 0 : max(g.S, g.T);
				int y = left ? i : i - g.S;
				const char *color = left ? "#0000FF" : "#FF0000";
				out << format("{\"id\":\"%d\",\"x\":%d,\"y\":%d,\"size\":%d,\"color\":\"%s\"}",
					i, x, y, 1, color);

			}
			out << format("],\"edges\":[");
			bool first = true;
			for (int i = 0; i < S; i++) {
				for (auto e: g.nodes[i].e) {
					if (i >= e.dest) continue;
					if (first) {
						first = false;
					} else {
						out << ",";
					}

					float size = m[i] == e.dest ? 1 : 0.5;
					const char *color = m[i] == e.dest ? "#1e1" : "#777";

					string id = format("%d-%d", i, e.dest);

					out << format("{\"id\":\"%s\",\"source\":\"%d\",\"target\":\"%d\",\"size\":%.2f,\"color\":\"%s\"}",
						id.c_str(), i, e.dest, size, color);
				}
			}
			out << "]}";

			string data = out.str();
			render(conn, data);

		} else {
			mg_serve_http(conn, msg, server_opts);
		}

	}
}

bool running = true;

BOOL WINAPI consoleHandler(DWORD signal) {

	if (signal == CTRL_C_EVENT) {
		running = false;
	}

	return TRUE;
}

int main() {
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		printf("ERROR: Could not set control handler\n"); 
		return 1;
	}

	const char *port = "8888";

	struct mg_mgr mgr;
	struct mg_connection *conn;

	mg_mgr_init(&mgr, NULL);
	printf("APP START: %s\n", port);

	conn = mg_bind(&mgr, port, event_handler);
	if (!conn) {
		printf("Failed to bind\n");
		return 1;
	}

	mg_set_protocol_http_websocket(conn);
	server_opts.document_root = "web";
	server_opts.index_files = "index.html";
	server_opts.enable_directory_listing = "yes";

	while (running) {
		mg_mgr_poll(&mgr, 1000);
	}

	mg_mgr_free(&mgr);

	printf("APP STOP\n");

	return 0;
}
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

#include "hopcroft.cpp"

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

    if (r < 0)
        // conversion failed
        return {};

    const size_t len = r;
    if (len < sizeof buf)
        // we fit in the buffer
        return { buf, len };

#if __cplusplus >= 201703L
    // C++17: Create a string and write to its underlying array
    std::string s(len, '\0');
    va_start(args, fmt);
    std::vsnprintf(s.data(), len+1, fmt, args);
    va_end(args);

    return s;
#else
    // C++11 or C++14: We need to allocate scratch memory
    auto vbuf = std::unique_ptr<char[]>(new char[len+1]);
    va_start(args, fmt);
    std::vsnprintf(vbuf.get(), len+1, fmt, args);
    va_end(args);

    return { vbuf.get(), len };
#endif
}

static struct mg_serve_http_opts server_opts;

static void event_handler(struct mg_connection *conn, int ev, void *p) {
	if (ev == 0) return;

	if (ev == MG_EV_HTTP_REQUEST) {
		auto msg = (struct http_message *)p;

		string uri(msg->uri.p, msg->uri.len);
		string query(msg->query_string.p, msg->query_string.len);

		if (uri == "/api") {
			auto args = parseQuery(query);

			if (!checkArgs(args, {"action", "s", "t", "p"})) {
				mg_printf(conn, "%s",
                    "HTTP/1.0 400 Bad Request\r\n"
                    "Content-Length: 0\r\n\r\n");
				return;
			}

			auto action = args["action"];
			int S = stoi(args["s"]);
			int T = stoi(args["t"]);
			double p = stod(args["p"]);

			Graph g;
			initRandom(g, S, T, 0.2);

			ostringstream out;
			out << format("{\"nodes\":[");
			for (int i = 0; i < g.e.size(); i++) {
				if (i > 0) out << ",";
				bool left = !(i&1);
				int x = left ? 0 : 10;
				int y = i / 2;
				const char *color = left ? "#0000FF" : "#FF0000";
				out << format("{\"id\":\"%d\",\"x\":%d,\"y\":%d,\"size\":%d,\"color\":\"%s\"}",
					i, x, y, 3, color);

			}
			out << format("],\"edges\":[");
			bool first = true;
			for (int i = 0; i < g.e.size(); i++) {
				for (auto e: g.e[i]) {
					if (first) {
						first = false;
					} else {
						out << ",";
					}

					string id = format("%d-%d", i, e.dest);

					out << format("{\"id\":\"%s\",\"source\":\"%d\",\"target\":\"%d\"}",
						id.c_str(), i, e.dest);
				}
			}
			out << "]}";

			string data = out.str();
			mg_printf(conn, 
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: application/json\r\n"
				"Content-Length: %d\r\n\r\n%s",
				data.size(), data.c_str());

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
		printf("\nERROR: Could not set control handler"); 
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
	server_opts.enable_directory_listing = "no";

	while (running) {
		mg_mgr_poll(&mgr, 5);
	}

	mg_mgr_free(&mgr);

	printf("APP STOP\n");

	return 0;
}
#include <vector>
#include <deque>
#include <cinttypes>
#include <random>
#include <iostream>

using namespace std;

typedef uint64_t u64;
typedef uint8_t u8;

#define NONE -1

struct Edge {
	int dest;
	int weight;
};

struct Node {
	vector<Edge> e;
	u8 side;

	Edge& operator[] (int x) {
		return e[x];
	} 

	void add(Edge edge) {
		e.push_back(edge);
	}
};

struct Graph {
	int N, S, T;
	vector<Node> nodes;

	Graph() {
		N = S = T = 0;
	}

	Graph(int pS, int pT) {
		S = pS;
		T = pT;
		N = S+T;
		nodes = vector<Node>(N);
		for (int i = 0; i < N; i++) {
			nodes[i].side = i >= S;
		}
	}

	void clear() {
		N = S = T = 0;
		nodes.clear();
	}
};

random_device rd;
mt19937 mtgen(rd());
uniform_real_distribution<double> fdis01(0.0, 1.0);

double drand() {
	return fdis01(mtgen);
}

void initRandom(Graph &g, int S, int T, int count) {
	int N = S+T;
	g = Graph(S, T);

	double prob = (double)count / ((double)S*(double)T);

	for (int s = 0; s < S; s++) {
		for (int t = S; t < N; t++) {
			if (drand() < prob) {
				int weight = 1;
				g.nodes[s].add({t, weight});
				g.nodes[t].add({s, weight});
			}
		}
	}
}

bool hop_bfs(Graph const &g, vector<int> &m, int start, bool flip = false) {
	vector<int> pred(g.N, NONE);

	deque<int> q;

	q.push_back(start);

	while (!q.empty()) {
		int i = q.front();
		q.pop_front();

		if ((i < g.S) == flip) {
			q.push_back(m[i]);
			pred[m[i]] = i;
		} else {
			for (Edge e: g.nodes[i].e) {
				int j = e.dest;
				if (m[j] == NONE) {
					while (j != NONE) {
						m[j] = pred[j];
						m[pred[j]] = j;
						j = pred[pred[j]];
					}
					return true;
				} else {
					pred[j] = i;
					q.push_back(j);
				}
			}
		}

	}

	return false;
}

vector<int> hopcroft(Graph const &g) {
	auto m = vector<int>(g.N, NONE);

	for (int i = 0; i < g.S; i++) {
		hop_bfs(g, m, i, false);
	}

	return m;
}


int main() {

	cout << "test" << endl;

	return 0;
}


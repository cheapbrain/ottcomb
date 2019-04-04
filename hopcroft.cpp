#include <vector>
#include <queue>
#include <unordered_set>
#include <cinttypes>
#include <random>
#include <iostream>
#include <chrono>
#include <climits>

using namespace std;

typedef uint64_t u64;
typedef int64_t i64;
typedef uint8_t u8;

#define NONE -1
#define INF INT_MAX

struct Edge {
	int dest;
	float weight;
};

struct Node {
	vector<Edge> e;

	Edge& operator[] (int x) {
		return e[x];
	} 

	void add(Edge edge) {
		e.push_back(edge);
	}
};

struct Graph {
	int N;
	vector<Node> nodes;

	Graph(int N = 0) {
		this->N = N;
		nodes = vector<Node>(N);
	}

	void clear() {
		N = 0;
		nodes.clear();
	}

	void addEdge(int a, int b, float w = 1.f) {
		nodes[a].add({b, w});
		nodes[b].add({a, w});
	}

	bool hasEdge(int a, int b) const {
		for (Edge e: nodes[a].e) {
			if (e.dest == b) return true;
		}
		return false;
	}
};

struct BiGraph : Graph {
	int S, T;

	BiGraph(int S = 0, int T = 0) : Graph(S+T) {
		this->S = S;
		this->T = T;
	}

	void clear() {
		Graph::clear();
		S = T = 0;
	}

};

double drand() {
	static random_device rd;
	static mt19937 mtgen(rd());
	static uniform_real_distribution<double> fdis01(0.0, 1.0);
	return fdis01(mtgen);
}

void initRandom1(BiGraph &g, int S, int T, int out_degree) {
	int N = S+T;
	g = BiGraph(S, T);

	double prob = (double)out_degree / (double)T;

	for (int s = 0; s < S; s++) {
		for (int t = S; t < N; t++) {
			if (drand() < prob) {
				g.addEdge(s, t);
			}
		}
	}
}

void initRandom2(BiGraph &g, int S, int T, int out_degree) {
	unordered_set<u64> used;

	g = BiGraph(S, T);

	int count = out_degree * S;

	for (int i = 0; i < count; i++) {
		int s = (int)(drand()*S);
		int t = (int)(drand()*T) + S;
		u64 edge = ((u64)s << 32 ) | (u64)t;

		if (used.count(edge)) continue;
		used.insert(edge);

		g.addEdge(s, t);
	}

}

void initRandom3(BiGraph &g, int S, int T, double out_degree) {
	double p = (double)out_degree / (double)T;
	double logp = log(1.0 - p);

	g = BiGraph(S, T);
	i64 i = -1;
	i64 E = (i64)S * (i64)T;
	while (i < E) {
		i64 k = max(0, (int)ceil(log(drand()) / logp) - 1);
		i = i + k + 1;
		int s = (int)(i / g.T);
		int t = (int)(i % g.T) + g.S;
		float w = (float)(2*drand()+0.0001);
		g.addEdge(s, t, w);
	}
}

bool bi_max_match_bfs(BiGraph const &g, vector<int> &m, int start, vector<int> &pred) {
	fill(pred.begin(), pred.end(), NONE);
	queue<int> q;
	q.push(start);

	while (!q.empty()) {
		int i = q.front();
		q.pop();

		for (Edge e: g.nodes[i].e) {
			int j = e.dest;
			if (m[i] != j) {
				if (pred[j] == NONE) {
					pred[j] = i;
					if (m[j] == NONE) {
						while (j != NONE) {
							m[j] = pred[j];
							m[pred[j]] = j;
							j = pred[pred[j]];
						}
						return true;
					} else {
						pred[j] = i;
						pred[m[j]] = j;
						q.push(m[j]);
					}
				}
			}

		}
	}

	return false;
}

vector<int> bi_max_match(BiGraph const &g) {
	vector<int> m (g.N, NONE);
	vector<int> pred (g.N, NONE);

	for (int k = 0; k < g.S; k++) {
		bi_max_match_bfs(g, m, k, pred);
	}

	return m;
}

bool hop_bfs(BiGraph const &g, vector<int> &m, vector<int> &level, vector<int> &reached, int start, int end) {
	bool found = false;
	reached.clear();
	queue<int> q;

	fill(level.begin(), level.end(), INF);
	for (int i = start; i < end; i++) {
		if (m[i] == NONE) {
			q.push(i);
			level[i] = 0;
		}
	}

	while (!q.empty()) {
		int i = q.front(); q.pop();

		for (Edge e: g.nodes[i].e) {
			int j = e.dest;
			if (level[j] == INF) {
				level[j] = level[i] + 1;
				if (m[j] != NONE) {
					level[m[j]] = level[i] + 2;
					q.push(m[j]);
				} else {
					reached.push_back(j);
					found = true;
				}
			}
		}	
	}

	return found;
}

bool hop_dfs(BiGraph const &g, vector<int> &m, vector<int> &level, int i) {
	if (i == NONE) return true;
	for (Edge e: g.nodes[i].e) {
		int j = e.dest;
		if (level[i] == level[j] + 1) {
			level[j] = INF;
			if (hop_dfs(g, m, level, m[j])) {
				m[i] = j; m[j] = i;
				return true;
			}
		}
	}
	return false;
}

typedef vector<int> (*p_hopfunc)(BiGraph const&);
vector<int> hopcroft(BiGraph const &g) {
	bool flip = g.S > g.T;
	vector<int> m(g.N, NONE);
	vector<int> level(g.N);
	vector<int> reached;
	reached.reserve(flip ? g.S : g.T);
	int start = flip ? g.S : 0;
	int end = flip ? g.N : g.S;

	while(hop_bfs(g, m, level, reached, start, end)) {
		for (int i: reached) hop_dfs(g, m, level, i);
	}

	return m;
}

int check_match(BiGraph const &g, vector<int> const &m) {
	int count = 0;

	for (int i = 0; i < (int)m.size(); i++) {
		if (m[i] == -1) continue;
		if (i != m[m[i]] || !g.hasEdge(i, m[i])) {
			return -1;
		} else count++;
	}

	return count / 2;
}

void measure(p_hopfunc func, BiGraph const &g) {
	auto start = chrono::high_resolution_clock::now(); 

	auto m = func(g);

	auto stop = chrono::high_resolution_clock::now(); 
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start); 

	int c = check_match(g, m);

	cout << "match: " << c << " " << duration.count()/1000.f << "s" << endl;

}

int main(int argc, char **argv) {

	int S = atoi(argv[1]);
	int T = atoi(argv[2]);
	int out_degree = atoi(argv[3]);
	int generator = atoi(argv[4]);


	BiGraph g;

	{
		cout << "generating..." << endl;
		auto start = chrono::high_resolution_clock::now(); 


		switch(generator) {
			case 1:
			initRandom1(g, S, T, out_degree);
			break;
			case 2:
			initRandom2(g, S, T, out_degree);
			break;
			case 3:
			initRandom3(g, S, T, out_degree);
			break;
		}

		cout << "S: " << g.S << endl;
		cout << "T: " << g.T << endl;
		i64 E = 0;
		for (Node &n: g.nodes) E += n.e.size();
		cout << "E: " << E/2 << endl;

		auto stop = chrono::high_resolution_clock::now(); 
		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start); 
		cout << "done. " << duration.count()/1000.f << "s" << endl;
	}

	measure(bi_max_match, g);
	measure(hopcroft, g);

	return 0;
}


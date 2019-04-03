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
	int weight;
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
	}

	void clear() {
		N = S = T = 0;
		nodes.clear();
	}

	void addEdge(int a, int b, int w = 1) {
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

double drand() {
	static random_device rd;
	static mt19937 mtgen(rd());
	static uniform_real_distribution<double> fdis01(0.0, 1.0);
	return fdis01(mtgen);
}

void initRandom1(Graph &g, int S, int T, int out_degree) {
	int N = S+T;
	g = Graph(S, T);

	double prob = (double)out_degree / (double)T;

	for (int s = 0; s < S; s++) {
		for (int t = S; t < N; t++) {
			if (drand() < prob) {
				g.addEdge(s, t);
			}
		}
	}
}

void initRandom2(Graph &g, int S, int T, int out_degree) {
	unordered_set<u64> used;

	g = Graph(S, T);

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

void initRandom3(Graph &g, int S, int T, double out_degree) {
	double p = (double)out_degree / (double)T;
	double logp = log(1.0 - p);

	g = Graph(S, T);
	i64 i = -1;
	i64 E = (i64)S * (i64)T;
	while (i < E) {
		i64 k = max(0, (int)ceil(log(drand()) / logp) - 1);
		i = i + k + 1;
		int s = (int)(i / g.T);
		int t = (int)(i % g.T) + g.S;
		g.addEdge(s, t);
	}
}

bool bi_max_match_bfs(Graph const &g, vector<int> &m, int start, vector<int> &pred) {
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

vector<int> bi_max_match(Graph const &g) {
	vector<int> m (g.N, NONE);
	vector<int> pred (g.N, NONE);

	for (int k = 0; k < g.S; k++) {
		bi_max_match_bfs(g, m, k, pred);
	}

	return m;
}

void hop_init(Graph const &g, vector<int> &m, vector<int> &level, vector<int> &avail, vector<int> &reached) {
	bool flip = g.S > g.T;
	m = vector<int>(g.N, NONE);
	level = vector<int>(g.N);
	avail = vector<int>(flip ? g.T : g.S);
	iota(avail.begin(), avail.end(), flip ? g.S : 0);
	reached.reserve(flip ? g.S : g.T);
}

bool hop_bfs(Graph const &g, vector<int> &m, vector<int> &level, vector<int> &avail, vector<int> &reached, bool deep = false, bool useavail = true) {
	bool found = false;
	reached.clear();
	queue<int> q;

	if (useavail) {
		fill(level.begin(), level.end(), INF);
		for (vector<int>::size_type i = 0; i < avail.size(); i++) {
			if (m[avail[i]] == NONE) {
				q.push(avail[i]);
				level[avail[i]] = 0;
			} else {
				avail[i--] = avail.back();
				avail.pop_back();
			}
		}
	} else {
		for (int i = 0; i < g.S; i++) {
			if (m[i] == NONE) {
				q.push(i);
				level[i] = 0;
			} else level[i] = INF;
		}
	}

	int last_level = 0;
	while (!q.empty()) {
		int i = q.front(); q.pop();

		int curr_level = level[i];
		if (!deep && found && curr_level > last_level) break;
		else last_level = curr_level;

		for (Edge e: g.nodes[i].e) {
			int j = e.dest;
			if (level[j] == INF) {
				level[j] = curr_level + 1;
				if (m[j] != NONE) {
					level[m[j]] = curr_level + 2;
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

bool hop_dfs(Graph const &g, vector<int> &m, vector<int> &level, int i) {
	if (i == NONE) return true;
	for (Edge e: g.nodes[i].e) {
		int j = e.dest;
		if (level[i] == level[j] + 1 && hop_dfs(g, m, level, m[j])) {
			level[j] = INF;
			m[i] = j; m[j] = i;
			return true;
		} else level[j] = INF;
	}
	return false;
}

typedef vector<int> (*p_hopfunc)(Graph const&, bool, bool, bool);
vector<int> hopcroft(Graph const &g, bool deep = false, bool useavail = true, bool swapside = false) {
	vector<int> m, level, avail, reached;
	hop_init(g, m, level, avail, reached);

	while(hop_bfs(g, m, level, avail, reached, deep)) {
		for (int i: reached) {
			hop_dfs(g, m, level, i);
		}

		if (swapside) {
			if (reached.size() < avail.size()) {
				swap(reached, avail);
			}
		}
	}

	return m;
}

int check_match(Graph const &g, vector<int> const &m) {
	int count = 0;

	for (int i = 0; i < (int)m.size(); i++) {
		if (m[i] == -1) continue;
		if (i != m[m[i]] || !g.hasEdge(i, m[i])) {
			return -1;
		} else count++;
	}

	return count / 2;
}

int main1() {
	while(true) {
		Graph g;
		initRandom1(g, 5, 5, 2);
		auto m1 = bi_max_match(g);
		auto m2 = hopcroft(g);

		int c1 = check_match(g, m1);
		int c2 = check_match(g, m2);
		if (c1 != c2) {
			cout << "error" << endl;
			while(true);
		}
		cout << "done" << endl;
	}

	return 0;
}

int main2() {
	Graph g (5, 5);
	g.addEdge(0, 5);
	g.addEdge(0, 8);
	g.addEdge(0, 9);
	g.addEdge(1, 7);
	g.addEdge(1, 8);
	g.addEdge(1, 9);
	g.addEdge(2, 6);
	g.addEdge(2, 7);
	g.addEdge(2, 8);
	g.addEdge(3, 5);
	g.addEdge(3, 8);
	g.addEdge(4, 8);

	auto m = hopcroft(g);

	return 0;
}

void measure(p_hopfunc func, Graph const &g, bool deep, bool useavail, bool swapside) {
	auto start = chrono::high_resolution_clock::now(); 

	auto m = func(g, deep, useavail, swapside);

	auto stop = chrono::high_resolution_clock::now(); 
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start); 

	int c = check_match(g, m);

	cout << (int)deep << (int)useavail << (int)swapside << " " << c << " " << duration.count()/1000.f << "s" << endl;

}

int main(int argc, char **argv) {

	int size = atoi(argv[1]);
	int out_degree = atoi(argv[2]);
	int generator = atoi(argv[3]);


	Graph g;

	{
		cout << "generating..." << endl;
		auto start = chrono::high_resolution_clock::now(); 


		switch(generator) {
			case 1:
			initRandom1(g, size, size, out_degree);
			break;
			case 2:
			initRandom2(g, size, size, out_degree);
			break;
			case 3:
			initRandom3(g, size, size, out_degree);
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

	measure(hopcroft, g, false, false, false);
	measure(hopcroft, g, true, false, false);
	measure(hopcroft, g, false, true, false);
	measure(hopcroft, g, true, true, false);
	measure(hopcroft, g, true, true, true);

	return 0;
}


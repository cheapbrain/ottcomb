#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <cstdlib>

using namespace std;

typedef uint64_t u64;

template<typename A>
struct Vec2D {
	size_t S, T, size;
	A *v;

	Vec2D(int S, int T): T(T), S(S), size(T * S) { v = new A[size]; }
	Vec2D(int S, int T, A value): Vec2D(S, T) { fill(v, v + size, value); }
	Vec2D(Vec2D<A> const &o): Vec2D(o.S, o.T) { copy(o.v, o.v + size, v); }
	~Vec2D() { delete[] v; }
	A const operator()(size_t y, size_t x) const { return v[x + y * T]; }
	A& operator()(size_t y, size_t x) { return v[x + y * T]; }
	void init(size_t &y, size_t &x) const { y = 0; x = -1; }
	bool next(size_t &y, size_t &x) const {
		if (++x >= T) {
			x = 0;
			return (++y < S);
		} else return true;
	}
};

double drand() {
	static random_device rd;
	static mt19937 mtgen(rd());
	static uniform_real_distribution<double> fdis01(0.0, 1.0);
	return fdis01(mtgen);
}

Vec2D<int> randGraph(int N) {
	Vec2D<int> g(N, N);
	for (int i = 0; i < N * N; i++) g.v[i] = (int)(1000 * drand());
	return g;
}

void print(Vec2D<int> g) {
	for (size_t s = 0; s < g.S; s++) {
		for (size_t t = 0; t < g.T; t++) {
			cout << g(s, t) << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void print(vector<int> v) {
	for (int x : v) {
		cout << x << " ";
	}
	cout << endl;
}

int matchCost(vector<int> const &u, vector<int> const &v) {
	int m = 0;
	for (int x : u) m += x;
	for (int x : v) m += x;
	return m;
}

bool check(Vec2D<int> const &g, vector<int> const &m1, vector<int> const &m2, vector<int> const &u, vector<int> const &v) {
	u64 w = 0;
	for (int x: u) w += x;
	for (int x: v) w += x;

	int card = 0;
	u64 cost = 0;
	for (size_t s = 0; s < m1.size(); s++) {
		if (m1[s] == -1 or m2[m1[s]] == s) {
			cost += g(s, m1[s]);
			card++;
		} else {
			cout << "Error: infeasible primal solution" << endl;
			return false;
		}
	}

	size_t s,t;
	for (g.init(s,t); g.next(s,t);) {
		if (u[s] + v[t] > g(s,t)) {
			cout << "Error: infeasible dual solution" << endl;
			return false;
		}
	}

	if (card < min(g.T, g.S)) {
		cout << "Error: cardinality too small" << endl;
		return false;
	}

	if (w != cost) {
		cout << "Error: primal and dual solutions are different" << endl;
		return false;
	} 

	return true;
}

int minMatch(Vec2D<int> const &c) {
	int S = c.S; int T = c.T;
	vector<int> u(S, INT_MAX);
	vector<int> v(T, INT_MAX);
	vector<int> m1(S, -1);
	vector<int> m2(T, -1);
	int card = 0;
	size_t s, t;

// step 1 dual init
	for (c.init(s, t); c.next(s, t);) {
		u[s] = min(c(s, t), u[s]);
	}
	for (c.init(s, t); c.next(s, t);) {
		v[t] = min(c(s, t) - u[s], v[t]);
	}

// step 2 primal init
	for (c.init(s, t); c.next(s, t);) {
		if (c(s, t) == u[s] + v[t] and m1[s] == -1 and m2[t] == -1) {
			m1[s] = t; m2[t] = s;
			card++;
			t += T;
		}
	}

	int target = min(S, T);
	while (card < target) {
// step 3.1 path init
		vector<int> l1, l2;
		vector<int> label1(S, -1);
		vector<int> label2(T, -1);
		vector<int> p(T, INT_MAX);
		vector<int> h(T, -1);
		for (s = 0; s < S; s++) {
			if (m1[s] == -1) {
				label1[s] = -2; // -2 == S
				l1.push_back(s);
				for (t = 0; t < T; t++) {
					if (label2[t] == -1 and c(s, t) - u[s] - v[t] < p[t]) {
						p[t] = c(s, t) - u[s] - v[t];
						h[t] = s;
					}
				}
			}
		}

		int path = -1;
		while (path == -1) {
			while (path == -1) {
// step 3.2 labeling procedure
				if (l1.size() > 0) {
// step 3.2.A labl propagation from S to T
					s = l1.back(); l1.pop_back();
					for (t = 0; t < T; t++) {
						if (label2[t] == -1 and c(s, t) == u[s] + v[t]) {
							label2[t] = s;
							l2.push_back(t);
						}

					}
				} else if (l2.size() > 0) {
					int k = l2.back(); l2.pop_back();
					s = m2[k];
					if (s == -1) {
						path = k;
					} else if (label1[s] == -1) {
// step 3.2.B label propagation from T to S
						label1[s] = k;
						l1.push_back(s);
						for (t = 0; t < T; t++) {
							if (c(s, t) - u[s] - v[t] < p[t]) {
								p[t] = c(s, t) - u[s] - v[t];
								h[t] = s;
							}
						}
					}
				} else break;
			}

			if (path == -1) {
// step 4 dual iteration
				int delta = INT_MAX;
				for (t = 0; t < T; t++) {
					if (label2[t] == -1) delta = min(delta, p[t]);
				}
				for (s = 0; s < S; s++) {
					if (label1[s] != -1) u[s] += delta;
				}
				for (t = 0; t < T; t++) {
					if (label2[t] != -1) v[t] -= delta;
					else p[t] -= delta;
				}
				for (t = 0; t < T; t++) {
					if (label2[t] == -1 and p[t] == 0) {
						label2[t] = h[t]; //TODO (h(t), +) cosa significa??
						l2.push_back(t);
					}
				}
			}
		}

// step 5 primal iteration
		t = path;
		do {
			s = label2[t];
			m1[s] = t; m2[t] = s;
			card++;
			t = label1[s];
			if (t != -2) {
				card--;
			}
		} while (t != -2);

	}

	if (check(c, m1, m2, u, v)) {
		return matchCost(u, v);
	} else return -1;
}


int brute(Vec2D<int> const &g) {
	vector<int> m(g.T);
	for (size_t i = 0; i < g.T; i++) { m[i] = i; }

	int best = INT_MAX;
	do {
		int cost = 0;
		for (size_t i = 0; i < m.size(); i++) { cost += g(i,m[i]); }
		best = min(best, cost);
	} while(next_permutation(m.begin(), m.end()));
	return best;
}

int main(int argc, char **argv) {

	if (argc == 2) {

		int N = atoi(argv[1]);
		while (true) {
			Vec2D<int> g = randGraph(N);
			int m1 = minMatch(g);

			cout << m1 << endl;

			while (m1 == -1);
		}

	} else {

		int N;
		cin >> N;

		Vec2D<int> g(N, N);

		for (int i = 0; i < N * N; i++) {
			cin >> g.v[i];
		}

		int m = minMatch(g);

		cout << m << endl;
	}

	return 0;
}
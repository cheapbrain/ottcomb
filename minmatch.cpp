#include <vector>
#include <iostream>
#include <random>

using namespace std;

template<typename T>
struct Vec2D {
 size_t w, h, size;
 T *v;

 Vec2D(int h, int w): w(w), h(h), size(w*h) { v = new T[size](); }
 Vec2D(Vec2D<T> const &other): Vec2D(other.h, other.w) { copy(other.v, other.v+size, v); }
 ~Vec2D() { delete v; }
 T const operator()(size_t y, size_t x) const { return v[x+y*w]; }
 T& operator()(size_t y, size_t x) { return v[x+y*w]; }
 void init(int &y, int &x) const { y = 0; x = -1; }
 bool next(int &y, int &x) const {
    if (++x >= w) {
        x = 0;
        return (++y < h);
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

    for (int i = 0; i < N*N; i++) {
        g.v[i] = (int)(10*drand());
    }

    return g;
}

void print(Vec2D<int> g) {
    for (int s = 0; s < g.h; s++) {
        for (int t = 0; t < g.w; t++) {
            cout << g(s,t) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void print(vector<int> v) {
    for (int x: v) {
        cout << x << " ";
    }
    cout << endl;
}

int matchCost(vector<int> const &u, vector<int> const &v) {
    int m = 0;
    for (int x: u) m += x;
    for (int x: v) m += x;
    return m;
}

int minMatch(Vec2D<int> const &g) {
    int S = g.h; int T = g.w; int N = S+T;
    Vec2D<int> c = g;
    Vec2D<bool> x(S, T);
    vector<int> u(S, INT_MAX);
    vector<int> v(T, INT_MAX);
    vector<int> m1(S, -1);
    vector<int> m2(T, -1);
    int card = 0;
    int z = 0;
    int s, t;

    // step 1
    for (c.init(s,t); c.next(s,t);) {
        u[s] = min(c(s,t), u[s]);
    }
    for (c.init(s,t); c.next(s,t);) {
        v[t] = min(c(s,t) -= u[s], v[t]);
    }
    for (c.init(s,t); c.next(s,t);) {
        c(s,t) -= v[t];
    }

    // step 2
    for (s=0, t=-1; c.next(s,t);) {
        if (c(s,t) == u[s]+v[t] and m1[s] == -1 and m2[t] == -1) {
            x(s,t) = true;
            m1[s] = t; m2[t] = s;
            z += c(s,t);
            card++;
        }
    }

    int target = min(S, T);

    while (card < target) {
        // step 3.1
        vector<int> l;
        vector<int> l1(S, -1);
        vector<int> l2(T, -1);
        vector<int> p(T, INT_MAX);
        vector<int> h(T, -1);
        for (s = 0; s < S; s++) {
            if (m1[s] != -1) continue;

            l1[s] = -2; // -2 == S
            l.push_back(s);
            for (t = 0; t < T; t++) {
                if (l2[t] != -1) continue;
                if (c(s,t)-u[s]-v[t] < p[t]) {
                    p[t] = c(s,t)-u[s]-v[t];
                    h[t] = s;
                }
            }
        }

        int path = -1;
        while (path == -1) {
            while (path == -1 and l.size() > 0) {
                // step 3.2
                int k = l.back(); l.pop_back();
                if (k < S) {
                    // step 3.2.A
                    s = k;
                    for (t = 0; t < T; t++) {
                        if (l2[t] != -1 or c(s,t) != u[s]+v[t]) continue;

                        l2[t] = s;
                        l.push_back(t+S);
                    }
                } else {
                    k -= S;
                    s = m2[k];
                    if (s == -1) {
                        path = k;
                    } else if (l1[s] == -1) {
                        // step 3.2.B
                        l1[s] = k;
                        l.push_back(s);
                        for (t = 0; t < T; t++) {
                            if (c(s,t)-u[s]-v[t] >= p[t]) continue;
                            p[t] = c(s,t)-u[s]-v[t];
                            h[t] = s;
                        }
                    }
                }
            }

            if (path == -1) {
                // step 4 dual iteration
                int delta = INT_MAX;
                for (t = 0; t < T; t++) {
                    if (l2[t] == -1) delta = min(delta, p[t]);
                }
                for (s = 0; s < S; s++) {
                    u[s] += delta;
                }
                for (t = 0; t < T; t++) {
                    v[t] -= delta;
                    p[t] -= delta;
                    if (l2[t] == -1 and p[t] == 0) {
                        l2[t] = h[t]; //TODO (h(t), +) cosa significa??
                        l.push_back(t+S);
                    }
                }
            }
        }

        // step 5 primal iteration
        t = path;
        do {
            s = l2[t];
            m2[t] = s; m1[s] = t;
            x(s,t) = true;
            z += c(s,t);
            card++;
            t = l1[s];
            if (t != -2) {
                x(s,t) = false;
                z -= c(s,t);
                card--;
            }
        } while(t != -2);

    }

    print(c);

    return z;
}

int main() {

    int N;
    cin >> N;

    Vec2D<int> g(N, N);
    print(g);

    for (int i = 0; i < N*N; i++) {
        cin >> g.v[i];
    }

    print(g);

    int m = minMatch(g);

    cout << m << endl;
    print(g);

    return 0;
}
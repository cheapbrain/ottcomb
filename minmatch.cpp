#include <vector>
#include <iostream>
#include <random>

using namespace std;

double drand() {
    static random_device rd;
    static mt19937 mtgen(rd());
    static uniform_real_distribution<double> fdis01(0.0, 1.0);
    return fdis01(mtgen);
}

vector<int> randGraph(int N) {
    vector<int> g(N*N);

    for (int i = 0; i < N*N; i++) {
        g[i] = (int)(10*drand());
    }

    return g;
}

void print(vector<int> g, int N) {
    for (int s = 0; s < N; s++) {
        for (int t = 0; t < N; t++) {
            cout << g[s*N+t] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

int minMatch(vector<int> const &g, int S, int T) {
    int N = S+T;
    vector<int> c(g);
    vector<bool> x(S*T, false);
    vector<int> u(S, INT_MAX);
    vector<int> v(T, INT_MAX);
    vector<int> m1(S, -1);
    vector<int> m2(T, -1);
    int card = 0;

    for (int s = 0; s < S; s++) {
        for (int t = 0; t < T; t++) {
            u[s] = min(c[s*T+t], u[s]);
        }
    } 

    for (int s = 0; s < S; s++) {
        for (int t = 0; t < T; t++) {
            c[s*T+t] -= u[s];
            v[t] = min(c[s*T+t], v[t]);
        }
    }

    for (int s = 0; s < S; s++) {
        for (int t = 0; t < T; t++) {
            c[s*T+t] -= v[t];
        }
    }

    for (int s = 0; s < S; s++) {
        for (int t = 0; t < T; t++) {
            if (c[s*T+t] == 0 and m1[s] == -1 and m2[t] == -1) {
                x[s*T+t] = true;
                m1[s] = t; m2[t] = s;
                card++;
            }
        }
    }

    if (card == min(S, T)) return 0; // TODO

    while (true) {
        vector<int> l;
        vector<int> l1(S, -1);
        vector<int> l2(T, -1);
        vector<int> p(T, INT_MAX);
        vector<int> h(T, -1);
        for (int s = 0; s < S; s++) {
            if (m1[s] != -1) continue;

            l1[s] = 0;
            l.push_back(s);
            for (int t = 0; t < T; t++) {
                if (l2[t] != -1) continue;
                if (c[s*T+t] < p[t]) {
                    p[t] = c[s*T+t];
                    h[t] = s;
                }
            }
        }

        int path = -1;
        while (path == -1) {
            while (path == -1 and l.size() > 0) {
                int k = l.back(); l.pop_back();
                if (k < S) {
                    int s = k;
                    for (int t = 0; t < T; t++) {
                        if (l2[t] != -1 or c[s*T+t] != 0) continue;

                        l2[t] = s;
                        l.push_back(t+S);
                    }
                } else {
                    k -= S;
                    if (m2[k] == -1) {
                        path = k;
                    } else if (l1[m2[k]] == -1) {
                        l1[m2[k]] = k;
                        l.push_back(m2[k]);
                        for (int t = 0; t < T; t++) {
                            if ()

                        }
                    }
                }
            }

            if (path == -1) {
                // dual iteration
            }
        }

        // primal iteration

    }

    return 0;
}

int main2() {

    int N = 5;
    vector<int> g = randGraph(N);
    print(g, N);
    minMatch(g, N, N);
    print(g, N);

    return 0;
}

int main() {

    int N;
    cin >> N;

    vector<int> g(N*N);

    for (int i = 0; i < N*N; i++) {
        cin >> g[i];
    }

    minMatch(g, N, N);

    return 0;
}
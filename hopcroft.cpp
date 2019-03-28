#include <vector>
#include <cinttypes>

using namespace std;

typedef uint64_t u64;
typedef uint8_t u8;

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
};

u64 rand64() {
#if RAND_MAX == 0xFFFF
    return (u64)rand() << 60 | (u64)rand() << 45 | (u64)rand() << 30 | (u64)rand() << 15 | (u64)rand();
#else
    return (u64)rand() << 62 | (u64)rand() << 31 | (u64)rand();
#endif
}

double drand() {
    return (double)rand64() / -1ULL;
}

void initRandom(Graph &g, int S, int T, double prob) {
    int N = S+T;
    g = Graph(S, T);

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


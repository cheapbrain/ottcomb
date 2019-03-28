#include <vector>
#include <cinttypes>
 
using namespace std;
 
typedef uint64_t u64;
 
struct Edge {
    int dest;
    int weight;
};

typedef vector<Edge> Node;
 
struct Graph {
    vector<Node> e;
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
    g.e = vector<Node>(N);
 
    for (int s = 0; s < N; s += 2) {
        for (int t = 1; t < N; t += 2) {
            if (drand() < prob) {
                int weight = 1;
                g.e[s].push_back({t, weight});
                g.e[t].push_back({s, weight});
            }
        }
    }
}
 

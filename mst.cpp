#include <iostream>
#include <vector>
#include <climits>
#include <cassert>
#include <tuple>
#include <algorithm>

using namespace std;

struct PairingHeap {

	pair<int,int> e;
	PairingHeap *child = NULL;
	PairingHeap *next = NULL;
	PairingHeap *prev = NULL;

	PairingHeap(int val, int data) {
		e.first = val;
		e.second = data;
	}

};

pair<int,int> top(PairingHeap *h) {
	assert(h != NULL);
	return h->e;
}

PairingHeap *merge(PairingHeap *h1, PairingHeap *h2) {
	if (h1 == NULL) return h2;
	if (h2 == NULL) return h1;
	if (h1->e > h2->e) swap(h1, h2);
	if (h1->child != NULL) h1->child->prev = h2;
	h2->next = h1->child;
	h2->prev = h1;
	h1->child = h2;
	h1->next = NULL;
	h1->prev = NULL;
	return h1;
}

PairingHeap *mergePairs(PairingHeap *h) {
	if (h == NULL) return NULL;
	if (h->next == NULL) return h;
	return merge(merge(h, h->next), mergePairs(h->next->next));
}

PairingHeap *pop(PairingHeap *h) {
	assert(h != NULL);
	PairingHeap *out = mergePairs(h->child);
	if (out != NULL) out->prev = NULL;
	return out;
}

PairingHeap *decreaseKey(PairingHeap *h, PairingHeap *p, int val) {
	assert(p != NULL);
	if (p->e.first <= val) return h;

	p->e.first = val;
	if (h == p) return h;
	if (h == NULL) return p;

	if (p->prev != NULL) {
		if (p == p->prev->next) p->prev->next = p->next;
		else p->prev->child = p->next;
	}
	if (p->next != NULL) p->next->prev = p->prev;
	p->prev = NULL;
	p->next = NULL;

	return merge(h, p);
}

struct Set {
	int rank;
	int parent;

	Set(int x) {
		parent = x;
		rank = 0;
	}
};

vector<Set> setInit(int N) {
	vector<Set> v;
	v.reserve(N);
	for (int i = 0; i < N; i++) {
		v.push_back(Set(i));
	}
	return v;
}

int find(vector<Set> &set, int x) {
	if (x != set[x].parent) {
		set[x].parent = find(set, set[x].parent);
	}
	return set[x].parent;
}

void join(vector<Set> &set, int a, int b) {
	Set *aa = &set[find(set, a)];
	Set *bb = &set[find(set, b)];
	if (aa->rank < bb->rank) swap(aa, bb);
	if (aa->rank == bb->rank) aa->rank++;
	bb->parent = aa->parent;
}

typedef pair<int,int> Edge;
typedef vector<Edge> Node;
typedef vector<Node> Graph;

vector<pair<int,int>> prim(Graph &g) {

	int N = g.size();
	const int INF = INT_MAX;
	vector<PairingHeap> cost;
	cost.reserve(N);
	for (int i = 0; i < N; i++) {
		cost.push_back(PairingHeap(INF, i));
	}

	vector<int> parent(N, -1);
	vector<bool> visited(N, false);
	cost[0].e.first = 0;
	PairingHeap *h = cost.data() + 0;

	while(h != NULL) {
		int node = top(h).second;
		h = pop(h);
		visited[node] = true;
		for (Edge e: g[node]) {
			int val = e.first;
			int i = e.second;
			if (val < cost[i].e.first and !visited[i]) {
				h = decreaseKey(h, &cost[i], val);
				parent[i] = node;
			}
		}
	}

	vector<pair<int,int>> edges;
	for (int i = 1; i < N; i++) {
		edges.push_back({i, parent[i]});
	}

	return edges;
}

vector<pair<int,int>> kruskal(Graph &g) {
	int N = g.size();

	vector<tuple<int,int,int>> edges;
	for(int i = 0; i < N; i++) {
		for (int j = 0; j < (int)g[i].size(); j++) {
			if (i < g[i][j].second) {
				edges.push_back(make_tuple(g[i][j].first, i, g[i][j].second));
			}
		}
	}

	sort(edges.begin(), edges.end());

	vector<pair<int,int>> parent;
	vector<Set> set = setInit(N);

	int card = 0;
	for (auto edge: edges) {
		int w, a, b; tie(w, a, b) = edge;
		if (find(set, a) != find(set, b)) {
			card++;
			join(set, a, b);
			parent.push_back({a, b});
		}

		if (card == N-1) break;
	}

	return parent;
}

int main() {

	Graph g = {
		{{1, 1}},
		{{1, 0}, {15, 2}, {9, 3}, {1, 5}},
		{{15, 1}, {6, 4}, {18, 5}},
		{{9, 1}, {23, 4}, {4, 5}},
		{{6, 2}, {23, 3}, {11, 5}},
		{{1, 1}, {18, 2}, {4, 3}, {11, 4}}
	};

	auto tree = prim(g);

	for (auto p: tree) {
		cout << p.first << " -- " << p.second << endl;
	}

	auto tree2 = kruskal(g);

	for (auto p: tree2) {
		cout << p.first << " -- " << p.second << endl;
	}

	return 0;
}

#include <cstdlib>
#include <iostream>

using namespace std;

template<typename T>
struct array2d {
	size_t w, h;
	T *p;
	array2d(size_t w, size_t h) : w(w), h(h) { p = (T*) malloc(w*h*sizeof(T)); }
	~array2d() { free(p); }
	T& operator()(size_t y, size_t x) { return p[x + y * w]; }
	T const& operator()(size_t y, size_t x) const { return p[x + y * w]; }
};

int main() {

	array2d<int> v(20, 20);

	v(2, 3) = 23;

	cout << v.w << " " << v.h << endl;
	cout << v(2, 3) << endl;

	return 0;
}
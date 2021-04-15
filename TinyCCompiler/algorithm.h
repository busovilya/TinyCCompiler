#include <set>

template <class T>
std::set<T> setUnion(std::set<T> first, std::set<T> second) {
	std::set<T> result = first;
	for (T item : second) {
		result.insert(item);
	}

	return result;
}
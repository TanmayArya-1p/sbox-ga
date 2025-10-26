#include <bits/stdc++.h>
#include <bitset>
#include <cstddef>
#include <execution>
#include <iterator>
#include <ranges>
#include <sys/types.h>

namespace {
	template<typename T>
	concept Iterable = requires(T& t) {
	    std::begin(t);
	    std::end(t);
	};

	template<std::ranges::input_range T>
	using elem_t = std::remove_cvref_t<std::ranges::range_value_t<T>>;
}


namespace utils {

template<std::size_t N>
constexpr bool validate_permutation(std::array<uint, N> perm) {
	std::bitset<N> found = {0};
	for(int i = 0; i < N ; i++) {
		if(perm[i] > N || perm[i] < 1) return false;
		found[perm[i]] ^= 1;

		if(!found[perm[i]]) {
			return false;
		}
	}
	return true;
}



template<std::ranges::input_range T>
constexpr bool validate_upper_bound(const T& array, elem_t<T> bound) {

	return *std::max_element(std::execution::par_unseq ,array.begin(), array.end()) <= bound;
}


}

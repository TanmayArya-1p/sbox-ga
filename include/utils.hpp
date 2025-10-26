#pragma once
#include <bits/stdc++.h>
#include <cstddef>
#include <ranges>
#include <sys/types.h>

namespace {

template<std::ranges::input_range T>
using elem_t = std::remove_cvref_t<std::ranges::range_value_t<T>>;

}


namespace utils {

template<std::size_t N>
constexpr bool validate_permutation(std::array<uint, N> perm);



template<std::ranges::input_range T>
constexpr bool validate_upper_bound(const T& array, elem_t<T> bound);

}

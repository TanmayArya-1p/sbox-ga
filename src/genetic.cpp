#include <bits/stdc++.h>
#include <cstddef>
#include "../include/sbox.hpp"
#include <random>

namespace {
	std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
}


namespace genetic {

template<std::size_t N, std::size_t M>
	defs::SBox<N, M> mutateSBox(defs::SBox<N,M>& sbox) {
		defs::SBox<N, M> otpt;
	 	std::array<uint, N> parent_sub = sbox.getSubstitution();
		auto distr = std::uniform_int_distribution<std::size_t>(0, N-1);

		// TODO: put mutation intensity in config somewhere
		for(int i =0 ; i < 2; i++) {
			parent_sub[distr(rng)] = parent_sub[distr(rng)];
		}
		otpt = {parent_sub};
		return otpt;
	}


template<std::size_t N, std::size_t M>
defs::SBox<N, M>& crossOverSBox(defs::SBox<N, M>& p1, defs::SBox<N, M>& p2) {
	defs::SBox<N, M> otpt;
	// using one point crossover
	std::array<uint, N> p1_sub = p1.getSubstitution();
	std::array<uint, N> p2_sub = p2.getSubstitution();
	auto distr = std::uniform_int_distribution<std::size_t>(1, N-1);
	auto pivot = distr(rng);


	std::array<uint, N> child_sub;
	for(int i =0 ; i < N; i++) {
		if(i < pivot) {
			child_sub[i] = p1_sub[i];
		} else {
			child_sub[i] = p2_sub[i];
		}
	}
	otpt = {child_sub};
	return otpt;
}


}

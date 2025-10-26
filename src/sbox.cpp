#include <array>
#include <bits/stdc++.h>
#include <stdexcept>
#include <sys/types.h>
#include <random>
#include <chrono>
#include "../include/utils.hpp"
#include "../include/sbox.hpp"


namespace {
	std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
}


namespace defs {

template<std::size_t N , std::size_t M>
SBox<N,M>::SBox(std::array<uint, N>&& substitution) {
	if(!utils::validate_upper_bound(substitution , N-1)) {
		throw std::invalid_argument("Invalid Substitution array");
	}
	this->input_ub = 1<<N;
	this->output_ub = 1<<M;
	this->sub = std::move(substitution);
}


template<std::size_t N , std::size_t M>
SBox<N,M>::SBox(std::array<uint, N>& substitution) {
	if(!utils::validate_upper_bound(substitution , N-1)) {
		throw std::invalid_argument("Invalid Substitution array");
	}
	this->input_ub = 1<<N;
	this->output_ub = 1<<M;
	this->sub = substitution;
}

template<std::size_t N , std::size_t M>
std::bitset<M> SBox<N,M>::encrypt(std::bitset<N> input) {
	std::bitset<M> output = {0};
	for(int i = 0 ;i < M ;i++) {
		output[i] = input[this->sub[i]];
	}
	return output;
}


template<std::size_t N , std::size_t M>
std::bitset<N> SBox<N,M>::decrypt(std::bitset<M> output) {
	std::bitset<N> input = {0};
	for(int i = 0 ;i < N ;i++) {
		input[i] = output[this->sub[i]];
	}
	return input;
}


template<std::size_t N , std::size_t M>
std::array<uint, N> SBox<N,M>::getSubstitution() const {
	return this->sub;
};

template<std::size_t N , std::size_t M>
SBox<N,M>::~SBox() {
};



template<std::size_t N , std::size_t M>
defs::SBox<N,M>& randomSBOX() {
	std::array<uint, M> sub;
	auto distr = std::uniform_int_distribution<int>(0, N-1);
	for(uint i = 0 ;i < N ;i++) {
		sub[i] = distr(rng);
	}
	return SBox(sub);
}



}

#pragma once

#include <array>
#include <bits/stdc++.h>
#include <stdexcept>
#include <sys/types.h>
#include <random>
#include <chrono>
#include "utils.cpp"


namespace {
	std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
}


namespace defs {

template<std::size_t N , std::size_t M>
class SBox {
	static_assert(N>=M, "Invalid SBox size arguments");

	public:
		SBox(std::array<uint, N>&& substitution) {
			if(!utils::validate_upper_bound(substitution , N-1)) {
				throw std::invalid_argument("Invalid Substitution array");
			}
			this->input_ub = 1<<N;
			this->output_ub = 1<<M;
			this->sub = std::move(substitution);
		}


		SBox(std::array<uint, N>& substitution) {
				if(!utils::validate_upper_bound(substitution , N-1)) {
					throw std::invalid_argument("Invalid Substitution array");
				}
				this->input_ub = 1<<N;
				this->output_ub = 1<<M;
				this->sub = substitution;
		}

		std::bitset<M> encrypt(std::bitset<N> input) {
			std::bitset<M> output = {0};
			for(int i = 0 ;i < M ;i++) {
				output[i] = input[this->sub[i]];
			}
			return output;
		}


		std::bitset<N> decrypt(std::bitset<M> output) {
				std::bitset<N> input = {0};
				for(int i = 0 ;i < N ;i++) {
					input[i] = output[this->sub[i]];
				}
				return input;
		}


		std::array<uint, N> getSubstitution() const {
				return this->sub;
		};

		~SBox() {
		};




		std::vector<std::vector<uint>>& generate_ddt(bool flush=false);

	private:
		std::array<uint, M> sub;
		std::vector<std::vector<uint>> ddt;
		uint input_ub;
		uint output_ub;
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

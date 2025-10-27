#pragma once

#include <algorithm>
#include <array>
#include <bits/stdc++.h>
#include <numeric>
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

		friend std::ostream& operator<<(std::ostream& os, const SBox<N,M>& sb) {
			for(int i = 0 ;i < sb.input_ub ;i++) {
				for(int j = 0 ;j < sb.output_ub ;j++) {
					os << sb.ddt[i][j] << " ";
				}
				os << std::endl;
			}
			return os;
		}

		SBox(std::array<ulong, 1<<N>&& substitution) {
			this->output_ub = 1<<M;
			this->input_ub = 1<<N;

			if(!utils::validate_upper_bound(substitution , this->output_ub)) {
				throw std::invalid_argument("Invalid Substitution array");
			}
			this->sub = std::move(substitution);
			this->is_invertible();
		}


		SBox(std::array<ulong, 1<<N>& substitution) {

			this->input_ub = 1<<N;
			this->output_ub = 1<<M;
			if(!utils::validate_upper_bound(substitution , this->output_ub)) {
				throw std::invalid_argument("Invalid Substitution array");
			}

			this->sub = substitution;
			this->is_invertible();
		}

		std::bitset<M> encrypt(std::bitset<N> input) {
			ulong ilong = input.to_ulong();
			std::bitset<M> output = {this->sub[ilong]};
			return output;
		}

		std::bitset<N> decrypt(std::bitset<M> output) {
			ulong olong = output.to_ulong();
			std::bitset<M> input = {this->inv_map[olong]};
			return input;
		}


		std::array<ulong, 1<<N> getSubstitution() const {
			return this->sub;
		};


		bool is_invertible() {
			if (this->invertible) return true;

			if(N==M && utils::validate_permutation<1<<N>(this->sub)) {
				this->invertible=true;
				this->inv_map.resize(this->output_ub);
				for (size_t i = 0; i < this->output_ub; i++) {
					this->inv_map[this->sub[i]] = i;
				}
			}
			return this->invertible;
		}


		std::vector<std::vector<uint>>& generate_ddt(bool flush=false);

	private:
		std::array<ulong, 1<<M> sub;
		std::vector<ulong> inv_map;
		std::vector<std::vector<uint>> ddt;
		ulong input_ub;
		ulong output_ub;
		bool invertible=false;
};


template<std::size_t N , std::size_t M>
defs::SBox<N,M> randomSBox() {
	std::array<ulong, 1<<N> sub;
	auto distr = std::uniform_int_distribution<int>(0, (1<<M)-1);
	for(uint i = 0 ;i < 1<<N ;i++) {
		sub[i] = distr(rng);
	}
	return SBox<N,M>(sub);
}


template<std::size_t N>
defs::SBox<N,N> randomInvertibleSBox() {
	std::array<ulong, 1<<N> sub;
	std::iota(sub.begin(), sub.end(), 0);
	std::shuffle(sub.begin(), sub.end() , rng);
	return SBox<N,N>(std::move(sub));
}

}

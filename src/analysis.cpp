#pragma once
#include <bits/stdc++.h>
#include <bitset>
#include <cstddef>
#include <memory>
#include "sbox.cpp"


template<std::size_t N , std::size_t M>
std::vector<std::vector<uint>>& defs::SBox<N,M>::generate_ddt(bool flush) {
	if(!flush && !ddt.empty())
		return this->ddt;

	this->ddt = std::vector<std::vector<uint>>(input_ub,std::vector<uint>(output_ub,0));

	//need to go through every single inp diff
	for(uint idiff = 0; idiff < input_ub ;idiff++) {
		for(uint i1 = 0 ;i1 < input_ub ; i1++) {
			uint i2 = i1^idiff;
			std::bitset<M> bitset = this->encrypt(i1) ^ this->encrypt(i2);
			ddt[idiff][(uint)(bitset.to_ulong())]++;
		}
	}
	return this->ddt;
}



namespace analysis {


	template<std::size_t N, std::size_t M>
	struct SBoxStatistics {
		uint zero_count;
		uint delta;
		uint cnt_delta;
		std::shared_ptr<defs::SBox<N,M>> sbox;
		std::optional<int> score;


		SBoxStatistics() = default;
		SBoxStatistics(std::shared_ptr<defs::SBox<N, M>> sbox_shared_ptr, std::function<int(const SBoxStatistics<N,M>&)> score_statistic) {
			*this = sbox_analyze(sbox_shared_ptr, score_statistic);
		}

		auto operator<=>(const SBoxStatistics<N,M>& other) const {
			return this->score <=> other.score;
		}
	};


template<std::size_t N, std::size_t M>
struct SBoxStatistics<N,M> sbox_analyze(std::shared_ptr<defs::SBox<N,M>> sbox, std::function<int(const SBoxStatistics<N,M>&)> score_statistic) {

	SBoxStatistics<N,M> stats;
	stats.sbox = sbox;
	stats.zero_count = 0;
	stats.delta = 0;
	stats.cnt_delta = 0;

	auto ddt = sbox->generate_ddt();

	// TODO: MAKE THIS MULTITHREADED
	for(auto& i : ddt) {
		for(auto& j : i) {
			if(j == 0)
				stats.zero_count++;
			if(j == stats.delta)
				stats.cnt_delta++;
			if(j > stats.delta)
				stats.cnt_delta=1;
		}
	}


	// stats.score = -stats.delta*256 + stats.zero_count;
	stats.score = score_statistic(stats);
	return stats;
}

}



namespace score {

	template<std::size_t N , std::size_t M>
	int test_score(const analysis::SBoxStatistics<N,M>& stats) {
		return -stats.delta*256 + stats.zero_count;
	}
}

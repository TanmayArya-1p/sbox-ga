#include <bits/stdc++.h>
#include <cstddef>
#include <memory>
#include <optional>
#include "../include/sbox.hpp"




template<std::size_t N , std::size_t M>
std::vector<std::vector<uint>>& defs::SBox<N,M>::generate_ddt(bool flush) {
	if(!flush && !ddt.empty())
		return this->ddt;

	this->ddt = std::vector<std::vector<uint>>(input_ub,std::vector<uint>(output_ub,0));

	//need to go through every single inp diff
	for(uint idiff = 0; idiff < input_ub ;idiff++) {
		for(uint i1 = 0 ;i1 < input_ub ; i1++) {
			uint i2 = i1^idiff;
			ddt[idiff][this->encrypt(i1) ^ this->encrypt(i2)]++;
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
};


template<std::size_t N, std::size_t M>
struct SBoxStatistics<N,M> sbox_analyze(std::shared_ptr<defs::SBox<N,M>> sbox, std::function<int(const SBoxStatistics<N,M>&)> score_statistic) {

	SBoxStatistics<N,M> stats;
	stats.sbox = sbox;
	stats.zero_count = 0;
	stats.delta = 0;
	stats.cnt_delta = 0;

	sbox->generate_ddt();

	// TODO: MAKE THIS MULTITHREADED
	for(auto& i : sbox->ddt) {
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
	stats.score = score_func(stats);
	return stats;
}

}


namespace score {

	template<std::size_t N , std::size_t M>
	int test_score(const analysis::SBoxStatistics<N,M>& stats) {
		return -stats.delta*256 + stats.zero_count;
	}
}

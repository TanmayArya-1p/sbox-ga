#include <bits/stdc++.h>
#include <cstddef>
#include <memory>
#include "sbox.cpp"


template<std::size_t N , std::size_t M>
std::vector<std::vector<uint>>& defs::SBox<N,M>::generate_ddt(bool flush) {
	if(!flush && !ddt.empty())
		return this->ddt;

	this->ddt = std::vector<std::vector<uint>>(input_ub,std::vector<uint>(output_ub,0));
	//need to go through every single inp diff
	for(ulong idiff = 0; idiff < input_ub ;idiff++) {
		for(ulong i1 = 0 ;i1 < input_ub ; i1++) {
			ulong i2 = i1^idiff;

			ulong y1 = this->encrypt(i1).to_ulong();
			ulong y2 = this->encrypt(i2).to_ulong();

			ulong od = y1 ^ y2;
			ddt[idiff][od]++;
		}
	}
	return this->ddt;
}



namespace analysis {


	template<std::size_t N, std::size_t M>
	struct SBoxStatistics;


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
				if(j!=16 && j > stats.delta) {
					stats.delta = j;
					stats.cnt_delta=1;
				}
			}
		}

		stats.score = score_statistic(stats);

		return stats;
	}

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




}



namespace score {

	template<std::size_t N , std::size_t M>
	std::function<int(const analysis::SBoxStatistics<N,M>&)> test_score = [](const analysis::SBoxStatistics<N,M>& stats) {
		return -stats.delta*256 + stats.zero_count;
	};
}

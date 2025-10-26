#pragma once
#include <bits/stdc++.h>
#include "sbox.hpp"
#include "analysis.hpp"

namespace genetic {
    template<std::size_t N, std::size_t M>
    defs::SBox<N, M>& mutateSBox(defs::SBox<N, M>&);

    template<std::size_t N, std::size_t M>
    defs::SBox<N, M>& crossOverSBox(const defs::SBox<N, M>& p1, const defs::SBox<N, M>& p2);



namespace selection {
	template<std::size_t N , std::size_t M>
	std::vector<analysis::SBoxStatistics<N,  M>>& eliteK(std::vector<analysis::SBoxStatistics<N,  M>>& stats, uint k);
}


template<std::size_t N, std::size_t M>
class Population {
	public:
		Population(std::size_t size);
		Population(std::vector<std::shared_ptr<defs::SBox<N, M>>> sboxes);

		void init_random();

		std::vector<analysis::SBoxStatistics<N, M>>& population_statistics();

		void evolve();


		analysis::SBoxStatistics<N,M> best_sbox();


	private:
		std::size_t size;
		std::vector<std::shared_ptr<defs::SBox<N, M>>> sboxes;
		analysis::SBoxStatistics<N, M> best_cand;
};

}

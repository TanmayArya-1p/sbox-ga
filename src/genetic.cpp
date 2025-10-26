#include <bits/stdc++.h>
#include <cstddef>
#include "../include/sbox.hpp"
#include "../include/analysis.hpp"
#include "../include/genetic.hpp"


#include <memory>
#include <queue>
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
defs::SBox<N, M>& crossOverSBox(const defs::SBox<N, M>& p1, const defs::SBox<N, M>& p2) {
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

namespace selection {

	template<std::size_t N , std::size_t M>
	std::vector<analysis::SBoxStatistics<N,  M>>& eliteK(std::vector<analysis::SBoxStatistics<N,  M>>& stats, uint k) {

		// TODO: parallelize this

		std::priority_queue<int, std::vector<int>, std::greater<analysis::SBoxStatistics<N,M>>> pq;
		defs::SBox<N, M> min_stat = *stats.begin();

		// eventually need to mix in the worst candidate to prevent localization
		k--;
		for(auto& stat : stats) {
			if(pq.size() < k) {
				pq.push(stat);
			} else {
				if(pq.top() < stat) {
					pq.pop();
					pq.push(stat);
				}
			}
			if(stat < min_stat) {
				min_stat = stat;
			}
		}

		std::vector<analysis::SBoxStatistics<N,  M>> res;
		while (!pq.empty()) {
		  res.push_back(pq.top());
		  pq.pop();
		}
		res.push_back(min_stat);

		return res;
	}
}

template<std::size_t N, std::size_t M>
Population<N,M>::Population(std::size_t size) : size(size) {}


template<std::size_t N, std::size_t M>
Population<N,M>::Population(std::vector<std::shared_ptr<defs::SBox<N, M>>> sboxes) : sboxes(sboxes) {}


template<std::size_t N, std::size_t M>
void Population<N,M>::init_random() {
	sboxes.reserve(size);
	for(int i = 0; i < size; i++) {
		sboxes.push_back(std::make_shared<defs::SBox<N, M>>(defs::SBox<N, M>::random()));
	}
}


template<std::size_t N, std::size_t M>
std::vector<analysis::SBoxStatistics<N, M>>& Population<N,M>::population_statistics() {
	std::vector<analysis::SBoxStatistics<N, M>> stats;
	stats.reserve(size);
	for(auto& sbox : sboxes) {
		stats.push_back(analysis::sbox_analyze(sbox, score::test_score));
		if(stats.size() == 1) {
			this->best_cand = stats[0];
		} else if(*(--stats.end()) > this->best_cand) {
			this->best_cand = *(--stats.end());
		}
	}

	return stats;
}

template<std::size_t N, std::size_t M>
void Population<N,M>::evolve() {
	std::vector<std::shared_ptr<defs::SBox<N, M>>> new_sboxes;
	new_sboxes.reserve(size);


	//TODO: put k in a config or something
	std::vector<analysis::SBoxStatistics<N,  M>> elites = selection::eliteK(this->population_statistics(), 10);
	for(auto& stat : elites) {
		new_sboxes.push_back(stat.sbox);
	}

	std::uniform_int_distribution<std::size_t> distr(0,elites.size()-1);
	std::uniform_int_distribution<std::size_t> mixer(0,size/2);

	while(new_sboxes.size() < size) {
		//pick 1 from elites
		std::shared_ptr<defs::SBox<N, M>> p1 = elites[distr(rng)];
		//pick 1 from general pop
		std::shared_ptr<defs::SBox<N, M>> p2 = sboxes[mixer(rng)];
		auto child = genetic::crossOverSBox(*p1.get(), *p2.get());

		//TODO: put mutation rate in a config
		if(rand() % 10 < 0.8*10) {
			child = genetic::mutateSBox(child);
		}
		new_sboxes.push_back(child);
	}

	this->sboxes = new_sboxes;
}

template<std::size_t N, std::size_t M>
analysis::SBoxStatistics<N,M> Population<N,M>::best_sbox() {
	return this->best_cand;
}


}

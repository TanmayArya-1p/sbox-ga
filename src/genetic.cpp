#pragma once
#include <array>
#include <bits/stdc++.h>
#include <cstddef>
#include "sbox.cpp"
#include "analysis.cpp"
#include <functional>
#include <memory>
#include <queue>
#include <random>




namespace genetic {

namespace crossover_perm {
	template<std::size_t N>
	using crossover_alg = std::function<std::array<typename std::array<uint, N>::value_type, N>(const std::array<uint, N>& p1,const std::array<uint, N>& p2)>;

	template<std::size_t N>
	crossover_alg<N> pmx;
	template<std::size_t N>
	crossover_alg<N> ox1= [](const std::array<uint, N>& p0,const std::array<uint, N>& p1) {
		// sauce: https://en.wikipedia.org/wiki/Crossover_(evolutionary_algorithm)#Order_crossover_(OX1)
		std::uniform_int_distribution<uint> distr(0, 1);
		std::array<uint, N> child;
		std::priority_queue<std::pair<uint,uint>, std::vector<std::pair<uint,uint>> , std::greater<std::pair<uint,uint>>> q;

		std::pair<uint,uint> pivots = {distr(rng), distr(rng)};
		if(pivots.first > pivots.second) std::swap(pivots.first, pivots.second);

		std::array<uint, N> p1_index_table;
		for(int i = 0; i < N; i++) {
			p1_index_table[p1[i]] = i;
		}

		for(int i = 0;i < N; i++) {
			if(pivots.first <= i && i <= pivots.second) {
				child[i] = p0[i];
			} else {
				child[i] = -1;
				q.push({p1_index_table[p0[i]], p0[i]});
			}
		}

		for(int i = 0 ;i < N;i++) {
			if(child[i] == -1) {
				child[i] = q.top().second;
				q.pop();
			}
		}

		return child;
	};

}

//TODO: REVALUATE MULTITHREADING OPT

template<std::size_t N>
defs::SBox<N, N> mutateSBox(defs::SBox<N,N>& sbox) {

 	std::array<uint, N> parent_sub = sbox.getSubstitution();
	auto distr = std::uniform_int_distribution<std::size_t>(0, N-1);

	// TODO: put mutation intensity in config somewhere
	for(int i =0 ; i < 2; i++) {
		std::swap(parent_sub[distr(rng)], parent_sub[distr(rng)]);
	}
	defs::SBox<N, N> otpt = {parent_sub};
	return otpt;
}


template<std::size_t N>
defs::SBox<N, N> crossOverSBox(const defs::SBox<N, N>& p1, const defs::SBox<N, N>& p2, crossover_perm::crossover_alg<N> cross) {
	std::array<uint, N> p1_sub = p1.getSubstitution();
	std::array<uint, N> p2_sub = p2.getSubstitution();
	auto distr = std::uniform_int_distribution<std::size_t>(1, N-1);

	std::array<uint,N> child_sub = cross(p1_sub, p2_sub);
	auto otpt = defs::SBox<N, N>(child_sub);
	return otpt;
}

namespace selection {

	template<std::size_t N , std::size_t M>
	std::vector<analysis::SBoxStatistics<N,  M>> eliteK(const std::vector<analysis::SBoxStatistics<N,  M>>& stats, uint k) {

		// TODO: parallelize this

		std::priority_queue<analysis::SBoxStatistics<N,M>, std::vector<analysis::SBoxStatistics<N,M>>, std::greater<analysis::SBoxStatistics<N,M>>> pq;
		analysis::SBoxStatistics<N,  M> min_stat = *stats.begin();

		// eventually need to mix in the worst candidate to prevent localization
		k--;
		for(const auto& stat : stats) {
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
		res.reserve(k + 1);
		while (!pq.empty()) {
			res.push_back(pq.top());
			pq.pop();
		}
		res.push_back(min_stat);

		return res;
	}
}





template<std::size_t N>
class Population {
	public:
		Population(std::size_t size, crossover_perm::crossover_alg<N> crossover) : size(size) ,crossover(crossover) {}

		Population(std::vector<std::shared_ptr<defs::SBox<N, N>>> sboxes, crossover_perm::crossover_alg<N> crossover ) : sboxes(sboxes), crossover(crossover) {}

		void init_random() {
			sboxes.reserve(size);
			for(int i = 0; i < size; i++) {
				sboxes.push_back(std::make_shared<defs::SBox<N, N>>(defs::randomInvertibleSBox<N>()));
			}
		}

		std::vector<analysis::SBoxStatistics<N, N>> population_statistics() {
			std::vector<analysis::SBoxStatistics<N, N>> stats;
			stats.reserve(this->size);
			for(auto& sbox : sboxes) {
				stats.push_back(analysis::SBoxStatistics<N, N>{
					sbox,
					std::function<int(const analysis::SBoxStatistics<N,N>&)>(
			        [](const analysis::SBoxStatistics<N,N>& stats) -> int {
			            return -stats.delta; //TODO: FIND BETTER METRIC FROM A PAPER
			        })
				});

				if(stats.size() == 1) {
					this->best_cand = stats[0];
				} else if(*(--stats.end()) > this->best_cand) {
					this->best_cand = *(--stats.end());
				}
			}


			return stats;
		}


		void evolve() {

			std::vector<std::shared_ptr<defs::SBox<N, N>>> new_sboxes;
			new_sboxes.reserve(size);


			//TODO: put k in a config or something
			std::vector<analysis::SBoxStatistics<N, N>> elites = selection::eliteK(this->population_statistics(), 10);
			for(auto& stat : elites) {
				new_sboxes.push_back(stat.sbox);
			}

			std::uniform_int_distribution<std::size_t> distr(0,elites.size()-1);
			std::uniform_int_distribution<std::size_t> mixer(0,size/2);

			while(new_sboxes.size() < size) {
				//pick 1 from elites
				std::shared_ptr<defs::SBox<N, N>> p1 = elites[distr(rng)].sbox;
				//pick 1 from general pop
				std::shared_ptr<defs::SBox<N, N>> p2 = this->sboxes[mixer(rng)];
				auto child = genetic::crossOverSBox<N>(*p1.get(), *p2.get(), this->crossover);

				std::uniform_real_distribution<double> mutation_dist(0.0, 1.0);
				//TODO: put mutation rate in a config
				if(mutation_dist(rng) < 0.8) {
				    child = genetic::mutateSBox(child);
				}
				new_sboxes.push_back(std::make_shared<defs::SBox<N, N>>(child));
			}

			this->sboxes = new_sboxes;
		}


		analysis::SBoxStatistics<N,N> best_sbox() {
			return this->best_cand;
		}


	private:
		std::size_t size;
		std::vector<std::shared_ptr<defs::SBox<N, N>>> sboxes;
		analysis::SBoxStatistics<N, N> best_cand;
		crossover_perm::crossover_alg<N> crossover;
};

}

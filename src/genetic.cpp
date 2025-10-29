#pragma once
#include "config.h"
#include "analysis.cpp"
#include <bits/stdc++.h>
#include <cstddef>
#include <random>
#include <sys/types.h>




namespace genetic {

namespace crossover {
	template<std::size_t N>
	using crossover_alg = std::function<std::array<typename std::array<ulong, 1<<N>::value_type, 1<<N>(const std::array<ulong, 1<<N>& p1,const std::array<ulong, 1<<N>& p2)>;

	template<std::size_t N>
	crossover_alg<N> tpc = [](const std::array<ulong, 1<<N>& p0,const std::array<ulong, 1<<N>& p1) -> std::array<typename std::array<ulong, 1<<N>::value_type, 1<<N> {
		//two point crossover

		std::uniform_int_distribution<uint> distr(0, (1<<N)-1);
		std::array<ulong, 1<<N> child;

		std::pair<uint,uint> pivots = {distr(rng), distr(rng)};
		if(pivots.first > pivots.second) std::swap(pivots.first, pivots.second);

		for (size_t i= 0; i <= pivots.first; i++) {
			child[i] = p0[i];
		}
		for (size_t i= pivots.first; i < pivots.second; i++) {
			child[i] = p1[i];
		}
		for (size_t i= pivots.second; i < 1<<N; i++) {
			child[i] = p0[i];
		}
		return child;
	};

}


template<std::size_t N>
defs::SBox<N, N> mutateSBox(defs::SBox<N,N>& sbox) {

 	std::array<ulong, 1<<N> parent_sub = sbox.getSubstitution();
	auto distr = std::uniform_int_distribution<std::size_t>(0, (1<<N)-1);

	// TODO: put mutation intensity in config somewhere
	for(int i =0 ; i < 5; i++) {
		std::swap(parent_sub[distr(rng)], parent_sub[distr(rng)]);
	}
	defs::SBox<N, N> otpt = {parent_sub};
	return otpt;
}


template<std::size_t N>
defs::SBox<N, N> crossOverSBox(const defs::SBox<N, N>& p1, const defs::SBox<N, N>& p2, crossover::crossover_alg<N> cross) {
	// TODO: queue crosses over a thread pool

	std::array<ulong, 1<<N> p1_sub = p1.getSubstitution();
	std::array<ulong, 1<<N> p2_sub = p2.getSubstitution();

	std::array<ulong,1<<N> child_sub = cross(p1_sub, p2_sub);
	auto otpt = defs::SBox<N, N>(child_sub);
	return otpt;
}

namespace selection {

	template<std::size_t N, std::size_t M>
	using criteria = std::function<std::vector<analysis::SBoxStatistics<N,  M>>(const std::vector<analysis::SBoxStatistics<N,M>>& stats, uint k)>;


	template<std::size_t N , std::size_t M>
	static const criteria<N,M> eliteK = [](const std::vector<analysis::SBoxStatistics<N,  M>>& stats, uint k) -> std::vector<analysis::SBoxStatistics<N,  M>> {

		// TODO: parallelize this with a thread pool

		std::priority_queue<analysis::SBoxStatistics<N,M>, std::vector<analysis::SBoxStatistics<N,M>>, std::greater<analysis::SBoxStatistics<N,M>>> pq;
		analysis::SBoxStatistics<N,  M> min_stat = *stats.begin();

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
	};


	template<std::size_t N , std::size_t M>
	static const criteria<N, M> tournamentKF = [](const std::vector<analysis::SBoxStatistics<N,  M>>& stats, uint k) -> std::vector<analysis::SBoxStatistics<N,  M>> {

		std::uniform_int_distribution<size_t> distr(0, stats.size() - 1);

		// TODO: QUEUE K TOURNAMENTS OVER DIFFERENT THREADS
		std::vector<analysis::SBoxStatistics<N,  M>> selected;
		selected.reserve(k);


		size_t tournament_size = stats.size()*TOURNAMENT_SELECTION_PRESSURE;
		for (size_t i = 0 ; i< tournament_size ; i++) {
			std::unordered_set<size_t> chosen;

			size_t max_cand = distr(rng);
			chosen.insert(max_cand);

			while (chosen.size() < k) {
				size_t idx = distr(rng);
				if (chosen.find(idx) == chosen.end()) {
					chosen.insert(idx);
					if(stats[max_cand] < stats[idx]) {
						max_cand = idx;
					}
				}
			}
			selected.push_back(stats[max_cand]);
		}
		return selected;
	};

}



template<std::size_t N>
using score_statistic = std::function<int(const analysis::SBoxStatistics<N,N>&)>;

template<std::size_t N>
class Population {
	public:
		Population(std::size_t size, crossover::crossover_alg<N> crossover, score_statistic<N> score_stat, selection::criteria<N, N> selection_criteria) : size(size) ,crossover(crossover), score_stat(score_stat), selection_criteria(selection_criteria) {}

		Population(std::vector<std::shared_ptr<defs::SBox<N, N>>> sboxes, crossover::crossover_alg<N> crossover , score_statistic<N> score_stat, selection::criteria<N, N> selection_criteria) : sboxes(sboxes), crossover(crossover), score_stat(score_stat), selection_criteria(selection_criteria) {}

		void init_random() {
			sboxes.reserve(size);
			for(size_t i = 0; i < size; i++) {
				sboxes.push_back(std::make_shared<defs::SBox<N, N>>(defs::randomInvertibleSBox<N>()));
			}
		}

		std::vector<analysis::SBoxStatistics<N, N>> collect_statistics() {
			std::vector<analysis::SBoxStatistics<N, N>> stats;
			stats.reserve(this->size);
			for(auto& sbox : sboxes) {
				stats.push_back(analysis::SBoxStatistics<N, N>{
					sbox,
					this->score_stat
				});

				if(stats.size() == 1) {
					this->best_cand = stats[0];
				} else if(*(--stats.end()) > this->best_cand) {
					this->best_cand = *(--stats.end());
				}
			}


			return stats;
		}


		void evolve(uint k=10) {

			std::vector<std::shared_ptr<defs::SBox<N, N>>> new_sboxes;
			new_sboxes.reserve(size);


			std::vector<analysis::SBoxStatistics<N, N>> elites =  this->selection_criteria(this->collect_statistics(), k);
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

				if(mutation_dist(rng) < MUTATION_RATE) {
				    child = genetic::mutateSBox(child);
				}
				new_sboxes.push_back(std::make_shared<defs::SBox<N, N>>(child));
			}

			this->sboxes = new_sboxes;
		}


		analysis::SBoxStatistics<N,N> best_sbox_candidate() {
			return this->best_cand;
		}


	private:
		std::size_t size;
		std::vector<std::shared_ptr<defs::SBox<N, N>>> sboxes;
		analysis::SBoxStatistics<N, N> best_cand;
		crossover::crossover_alg<N> crossover;
		score_statistic<N> score_stat;
		selection::criteria<N, N> selection_criteria;
};

}

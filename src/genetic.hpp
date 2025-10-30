#pragma once
#include "const.h"
#include "analysis.hpp"
#include "sbox.hpp"
#include <bits/stdc++.h>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <thread>




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

		for (std::size_t i= 0; i <= pivots.first; i++) {
			child[i] = p0[i];
		}
		for (std::size_t i= pivots.first; i < pivots.second; i++) {
			child[i] = p1[i];
		}
		for (std::size_t i= pivots.second; i < 1<<N; i++) {
			child[i] = p0[i];
		}
		return child;
	};

}


template<std::size_t N>
defs::SBox<N, N> mutateSBox(defs::SBox<N,N>& sbox) {
 	std::array<ulong, 1<<N> parent_sub = sbox.getSubstitution();
	auto distr = std::uniform_int_distribution<std::size_t>(0, (1<<N)-1);

	for(std::size_t i =0 ; i < MUTATION_ROUNDS; i++) {
		std::swap(parent_sub[distr(rng)], parent_sub[distr(rng)]);
	}
	defs::SBox<N, N> otpt = {parent_sub};
	return otpt;
}


template<std::size_t N>
defs::SBox<N, N> crossOverSBox(const defs::SBox<N, N>& p1, const defs::SBox<N, N>& p2, crossover::crossover_alg<N> cross) {

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

	std::uniform_int_distribution<std::size_t> distr(0, stats.size() - 1);

	std::vector<analysis::SBoxStatistics<N,  M>> selected;
	selected.reserve(k);


	std::size_t tournament_size = stats.size()*TOURNAMENT_SELECTION_PRESSURE;
	for (std::size_t i = 0 ; i< k ; i++) {
		std::unordered_set<std::size_t> chosen;

		std::size_t max_cand = distr(rng);
		chosen.insert(max_cand);

		while (chosen.size() < tournament_size) {
			std::size_t idx = distr(rng);
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
			for(std::size_t i = 0; i < size; i++) {
				sboxes.push_back(std::make_shared<defs::SBox<N, N>>(defs::randomInvertibleSBox<N>()));
			}
		}

		std::vector<analysis::SBoxStatistics<N, N>> collect_statistics() {
			std::vector<analysis::SBoxStatistics<N, N>> stats(this->size);
			auto thread_pool = boost::asio::thread_pool(std::thread::hardware_concurrency());

			constexpr float _min_work_per_thread = 0.1;
			std::size_t _sb_per_thread = _min_work_per_thread*size;
			std::size_t _num_threads = size / _sb_per_thread + (size%_sb_per_thread? 1 : 0);

			if(_num_threads > std::thread::hardware_concurrency()) {
				_num_threads = std::thread::hardware_concurrency();
				_sb_per_thread =  size / _num_threads + (size % _num_threads? 0 : 1);
			}

			auto worker = [&stats, &sboxes=this->sboxes, score_stat=this->score_stat](std::size_t l, std::size_t r) {
				for(std::size_t i = l; i<=r ;i++) {
					stats[i] = analysis::SBoxStatistics<N, N>{
						sboxes[i],
						score_stat
					};
				}
			};


			for (size_t i = 0; i < _num_threads; i++) {
		        size_t l = i * _sb_per_thread;
		        size_t r = std::min((i + 1) * _sb_per_thread - 1, size - 1);
		        boost::asio::post(thread_pool,[&, i, l, r] {
		            worker(l, r);
		        });
		    }

			thread_pool.join();
			this->best_cand = stats[0];
			for(auto& sbox_stat : stats) {
			 	if(sbox_stat > this->best_cand) {
					this->best_cand = sbox_stat;
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

			constexpr float _min_work_per_thread = 0.2;
			std::size_t _remaining_sb = size-new_sboxes.size();
			new_sboxes.resize(size);

			std::size_t work_per_thread = _remaining_sb*_min_work_per_thread;
			std::size_t num_threads = _remaining_sb/work_per_thread + (_remaining_sb%work_per_thread? 1 : 0);
			if(num_threads > std::thread::hardware_concurrency()) {
			    num_threads = std::thread::hardware_concurrency();
			    work_per_thread = (_remaining_sb+1)/num_threads;
			}

			auto thread_pool = boost::asio::thread_pool(num_threads);

			auto worker = [&](std::size_t l , std::size_t r) {
			    for(std::size_t i = l; i < r; ++i) {
			        //pick 1 from elite
			        std::shared_ptr<defs::SBox<N, N>> p1 = elites[distr(rng)].sbox;
			        //pick 1 from general pop
			        std::shared_ptr<defs::SBox<N, N>> p2 = this->sboxes[mixer(rng)];
			        auto child = genetic::crossOverSBox<N>(*p1.get(), *p2.get(), this->crossover);

			        std::uniform_real_distribution<double> mutation_dist(0.0, 1.0);

			        if(mutation_dist(rng) < MUTATION_RATE) {
			            child = genetic::mutateSBox(child);
			        }

			        new_sboxes[i] = std::make_shared<defs::SBox<N, N>>(child);
			    }
			};

			for(std::size_t i = 0; i < num_threads; ++i) {
			    std::size_t l = k+i*work_per_thread;
			    std::size_t r = std::min(k+l+work_per_thread, size);

			    boost::asio::post(thread_pool , [&, l, r]() {
			        worker(l,r);
			    });
			}
			thread_pool.join();

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

#include <bits/stdc++.h>
#include "genetic.hpp"
#include "analysis.hpp"
#include "utils.hpp"
#include "sbox.hpp"

#define N 4
#define pop 1000
#define gens 100

int main() {
	auto start_time = std::chrono::high_resolution_clock::now();

	genetic::score_statistic<N> scorer = [](const analysis::SBoxStatistics<N,N>& stats) -> int {
		return (16-stats.delta)*stats.zero_count*(16-stats.cnt_delta);
	};

	genetic::Population<N> population(pop, genetic::crossover::tpc<N>, scorer, genetic::selection::tournamentKF<N,N>);
	population.init_random();

	for(int i=0;i<gens;i++){
		population.evolve();
		std::cout << "generation " << i+1;
		std::cout << "-best sbox score: " << population.best_sbox_candidate().score.value() << std::endl;
	}

	std::cout << "\n" << *population.best_sbox_candidate().sbox.get();
	std::cout << "-score: " << population.best_sbox_candidate().score.value() << std::endl;
	std::cout << "-delta: " << population.best_sbox_candidate().delta << std::endl;
	std::cout << "-delta count:" << population.best_sbox_candidate().cnt_delta << std::endl;
	std::cout << "-zero count:" << population.best_sbox_candidate().zero_count << std::endl;


	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	std::cout << "spent time: " << duration << " milliseconds" << std::endl;
	std::cout << "throughput: " << (double)pop*gens/duration << " sboxes/s" << std::endl;
}

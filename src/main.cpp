#include <bits/stdc++.h>
#include "genetic.hpp"
#include "analysis.hpp"
#include "sbox.hpp"

#define N 4
#define pop 1000
#define gens 1000

int main(int argc, char **argv) {

	std::size_t population_size = pop;
	std::size_t generations = gens;

	if (argc>=2) {
		 population_size= atoi(argv[1]);
	}
	if (argc>=3) {
		generations = atoi(argv[2]);
	}


	genetic::score_statistic<N> scorer = [](const analysis::SBoxStatistics<N,N>& stats) -> int {
		return (16-stats.delta)*stats.zero_count*(16-stats.cnt_delta);
	};

	genetic::Population<N> population(population_size, genetic::crossover::tpc<N>, scorer, genetic::selection::tournamentKF<N,N>);
	population.init_random();


	auto start_time = std::chrono::high_resolution_clock::now();
	for(std::size_t i=0;i<generations;i++){
		population.evolve();
		std::cout << "Generation " << i + 1;
		std::cout << " : Best sbox score: " << population.best_sbox_candidate().score.value() << std::endl;
	}


	auto best_sbox = population.best_sbox_candidate();
	std::cout << "\n" << *best_sbox.sbox.get();
	std::cout << "Score: " << best_sbox.score.value() << '\n';
	std::cout << "Delta: " << best_sbox.delta << '\n';
	std::cout << "Delta Count:" << best_sbox.cnt_delta << '\n';
	std::cout << "Zero Count:" << best_sbox.zero_count << std::endl;


	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

	std::cout << "Spent time: " << duration << " milliseconds" << '\n';
	std::cout << "Throughput: " << (double)pop*gens/duration << " sboxes/s" << std::endl;


}

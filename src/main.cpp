#include <bits/stdc++.h>
#include "genetic.cpp"

#define N 4
#define pop 1000
#define gens 10

int main() {

	genetic::score_statistic<N> scorer = [](const analysis::SBoxStatistics<N,N>& stats) -> int {
		//TODO: find a better heuristic
		return (16-stats.delta)*stats.zero_count*(16-stats.cnt_delta);
	};

	genetic::Population<N> population(pop , genetic::crossover::tpc<N>, scorer);
	population.init_random();
	for(int i=0;i<gens;i++){
		std::cout << "generation " << i << std::endl;
		population.evolve();
		std::cout << "best sbox delta: " << population.best_sbox_candidate().score.value() << std::endl;
		std::cout << std::endl;
	}

	  std::cout << "\n" << *population.best_sbox_candidate().sbox.get();
}

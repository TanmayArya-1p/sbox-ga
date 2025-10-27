#include <bits/stdc++.h>
#include "genetic.cpp"

int main() {

	genetic::score_statistic<4> scorer = [](const analysis::SBoxStatistics<4,4>& stats) -> int {
		return (16-stats.delta)*stats.zero_count*(16-stats.cnt_delta);
	};

	genetic::Population<4> population(1000 , genetic::crossover::tpc<4>, scorer);
	population.init_random();
	for(int i=0;i<1000;i++){
		std::cout << "generation " << i << std::endl;
		population.collect_statistics();
		std::cout << "best sbox score yet: " << population.best_sbox_candidate().score.value() << std::endl;
		// population.evolve();
		// std::cout << "best sbox delta: " << population.best_sbox_candidate().score.value()  << "\n" << *population.best_sbox_candidate().sbox.get() << std::endl;
		// std::cout << std::endl;
	}

	  std::cout << "\n" << *population.best_sbox_candidate().sbox.get();
}

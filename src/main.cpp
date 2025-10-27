#include <bits/stdc++.h>
#include "genetic.cpp"
#include "analysis.cpp"



int main() {
	genetic::Population<4,4> population(100 , genetic::crossover::pmx<4>);
	population.init_random();


	for(int i=0;i<10;i++){
		std::cout << "generation " << i << std::endl;
		std::cout << "best sbox delta: \n" << *population.best_sbox().sbox.get() << std::endl;
		population.evolve();
		std::cout << "best sbox delta: \n" << *population.best_sbox().sbox.get() << std::endl;
		std::cout << std::endl;
	}


	std::cout << population.best_sbox().delta << std::endl;
}

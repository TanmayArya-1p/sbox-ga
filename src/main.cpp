#include <bits/stdc++.h>
#include "genetic.cpp"
#include "analysis.cpp"



int main() {
	genetic::Population<16,16> population(100);
	population.init_random();

	for(int i=0;i<10;i++){
		population.evolve();
	}


	std::cout << population.best_sbox().delta << std::endl;
}

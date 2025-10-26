#pragma once
#include <array>
#include <bits/stdc++.h>
#include <bitset>
#include <sys/types.h>
#include <vector>


namespace defs {

	template<std::size_t N , std::size_t M>
	class SBox {
		public:

		SBox(std::array<uint, N>&& substitution);
		SBox(std::array<uint, N>& substitution);
		std::bitset<M> encrypt(std::bitset<N> input);

		std::bitset<N> decrypt(std::bitset<M> output);
		std::vector<std::vector<uint>>& generate_ddt(bool flush=false);

		~SBox();

		private:
			std::array<uint, M> sub;
			std::vector<std::vector<uint>> ddt;
			uint input_ub;
			uint output_ub;
};

template<std::size_t N , std::size_t M>
SBox<N,M>& randomSBOX();


}

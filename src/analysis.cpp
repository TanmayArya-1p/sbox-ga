#pragma once
#include <bits/stdc++.h>
#include "sbox.cpp"


template<std::size_t N , std::size_t M>
std::vector<std::vector<uint>>& defs::SBox<N,M>::generate_ddt(bool flush) {
	if(!flush && !ddt.empty())
		return this->ddt;

	this->ddt = std::vector<std::vector<uint>>(input_ub,std::vector<uint>(output_ub,0));

	//need to go through every single inp diff
	for(uint idiff = 0; idiff < input_ub ;idiff++) {
		for(uint i1 = 0 ;i1 < input_ub ; i1++) {
			uint i2 = i1^idiff;
			ddt[idiff][this->encrypt(i1) ^ this->encrypt(i2)]++;
		}
	}
	return this->ddt;
}

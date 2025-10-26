#pragma once
#include <bits/stdc++.h>
#include <memory>
#include <optional>
#include "sbox.hpp"


namespace analysis {

template<std::size_t N, std::size_t M>
struct SBoxStatistics {
	uint zero_count;
	uint delta;
	uint cnt_delta;
	std::shared_ptr<defs::SBox<N,M>> sbox;
	std::optional<int> score;

	SBoxStatistics(std::shared_ptr<defs::SBox<N, M>> sbox_shared_ptr);

	auto operator<=>(const SBoxStatistics<N,M>& other) const;
};


template<std::size_t N, std::size_t M>
struct SBoxStatistics<N,M> sbox_analyze(std::shared_ptr<defs::SBox<N,M>> sbox);
}


namespace score {

	template<std::size_t N , std::size_t M>
	int test_score(const analysis::SBoxStatistics<N,M>& stats);
}

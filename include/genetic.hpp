#pragma once
#include <bits/stdc++.h>
#include "sbox.hpp"

namespace genetic {
    template<std::size_t N, std::size_t M>
    defs::SBox<N, M>& mutateSBox(defs::SBox<N, M>&);
}

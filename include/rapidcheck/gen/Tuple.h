#pragma once

namespace rc {
namespace gen {

template<typename ...Gens> class TupleOf;
template<typename Gen1, typename Gen2> class PairOf;

//! Generates tuples containing elements generated by the provided generators.
template<typename ...Gens>
TupleOf<Gens...> tupleOf(Gens ...generators);

//! Generates pairs containing elements generated by the provided generators.
template<typename Gen1, typename Gen2>
PairOf<Gen1, Gen2> pairOf(Gen1 generator1, Gen2 generator2);

} // namespace gen
} // namespace rc

#include "Tuple.hpp"

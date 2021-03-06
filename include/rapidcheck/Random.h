#pragma once

#include <cstdint>
#include <array>
#include <limits>

namespace rc {

//! Implementation of a splittable random generator as described in:
//!   Claessen, K. och Palka, M. (2013) Splittable Pseudorandom Number
//!   Generators using Cryptographic Hashing.
class Random
{
    friend bool operator==(const Random &lhs, const Random &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Random &random);

public:
    //! Key type
    typedef std::array<uint64_t, 4> Key;

    //! Type of a generated random number.
    typedef uint64_t Number;

    //! Constructs a Random engine from a full size 256-bit key.
    Random(const Key &key);

    //! Constructs a Random engine from a 64-bit seed.
    Random(uint64_t seed);

    //! Creates a second generator from this one. Both `split` and `next` should
    //! not be called on the same state.
    Random split();

    //! Returns the next random number. Both `split` and `next` should not be
    //! called on the same state.
    Number next();

private:
    typedef std::array<uint64_t, 4> Block;

    typedef uint64_t Bits;
    static constexpr auto kBits = std::numeric_limits<Bits>::digits;

    typedef uint64_t Counter;
    static constexpr auto kCounterMax = std::numeric_limits<Counter>::max();

    void append(bool x);
    void mash(Block &output);

    Block m_key;
    Block m_block;
    Bits m_bits;
    Counter m_counter;
    uint8_t m_bitsi;
};

bool operator!=(const Random &lhs, const Random &rhs);

} // namespace rc

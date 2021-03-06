#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Operations.h"

#include "util/TypeListMacros.h"
#include "util/Meta.h"
#include "util/Util.h"
#include "util/Predictable.h"

using namespace rc;

namespace {

template<typename T, typename Predicate>
void generatesSuchThat(const Predicate &pred)
{
    auto generator = gen::noShrink(gen::resize(50, gen::arbitrary<T>()));
    while (true) {
        T x = *generator;
        if (pred(x)) {
            RC_SUCCEED("Generated value that satisfied predicate");
            return;
        }
    }
    RC_FAIL("The impossible happened...");
}

struct NumericProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "generates only zero when size is zero",
            [] {
                auto value =
                    *gen::noShrink(gen::resize(0, gen::arbitrary<T>()));
                RC_ASSERT(value == 0);
            });
    }
};

struct SignedProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "generates positive values",
            [] {
                generatesSuchThat<T>([] (T x) { return x > 0; });
            });

        templatedProp<T>(
            "generates negative values",
            [] {
                generatesSuchThat<T>([] (T x) { return x < 0; });
            });

        templatedProp<T>(
            "shrinks negative values to their positive equivalent",
            [] {
                T value = *gen::negative<T>();
                auto shrink = gen::arbitrary<T>().shrink(value).next();
                RC_ASSERT(shrink);
                RC_ASSERT(*shrink == -value);
            });
    }
};

} // namespace

TEST_CASE("gen::arbitrary<T> (signed integral)") {
    meta::forEachType<NumericProperties, RC_SIGNED_INTEGRAL_TYPES>();
    meta::forEachType<SignedProperties, RC_SIGNED_INTEGRAL_TYPES>();
}

TEST_CASE("gen::arbitrary<T> (unsigned integral)") {
    meta::forEachType<NumericProperties, RC_UNSIGNED_INTEGRAL_TYPES>();
}

namespace {

struct RealProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "shrinks to nearest integer",
            [] {
                T value = *gen::nonZero<T>();
                RC_PRE(value != std::trunc(value));
                auto seq = gen::arbitrary<T>().shrink(value);
                RC_ASSERT(seq::contains(seq, std::trunc(value)));
            });
    }
};

} // namespace

TEST_CASE("gen::arbitrary<T> (reals)") {
    meta::forEachType<NumericProperties, RC_REAL_TYPES>();
    meta::forEachType<SignedProperties, RC_REAL_TYPES>();
    meta::forEachType<RealProperties, RC_REAL_TYPES>();
}

TEST_CASE("gen::arbitrary<bool>") {
    prop("generates both true and false",
         [] {
             while (true) {
                 if (*gen::noShrink(gen::arbitrary<bool>()))
                     break;
             }

             while (true) {
                 if (!*gen::arbitrary<bool>())
                     break;
             }
         });

    SECTION("shrinks 'true' to 'false'") {
        REQUIRE(gen::arbitrary<bool>().shrink(true) == seq::just(false));
    }

    SECTION("does not shrink 'false'") {
        REQUIRE(gen::arbitrary<bool>().shrink(false) == Seq<bool>());
    }
}

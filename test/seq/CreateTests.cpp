#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Create.h"
#include "rapidcheck/seq/Transform.h"

#include "util/CopyGuard.h"
#include "util/Meta.h"
#include "util/Util.h"
#include "util/TypeListMacros.h"
#include "util/SeqUtils.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("seq::repeat") {
    prop("repeatedly returns the given value",
         [](const std::string &value) {
             auto seq = seq::repeat(value);
             for (int i = 0; i < 2000; i++) {
                 auto x = seq.next();
                 RC_ASSERT(x);
                 RC_ASSERT(*x == value);
             }
         });

    prop("copies are equal",
         [](const std::string &value) {
             assertEqualCopies(seq::take(200, seq::repeat(value)));
         });

    prop("does not copy value on construction",
         [](CopyGuard guard) {
             auto seq = seq::repeat(std::move(guard));
         });
}

TEST_CASE("seq::just") {
    SECTION("does not copy values") {
        auto seq = seq::just(CopyGuard(), CopyGuard(), CopyGuard());
        seq.next();
        seq.next();
        seq.next();
    }

    prop("returns the passed in objects",
         [](const std::string &a,
             const std::string &b,
             const std::string &c)
         {
             auto seq = seq::just(a, b, c);
             RC_ASSERT(*seq.next() == a);
             RC_ASSERT(*seq.next() == b);
             RC_ASSERT(*seq.next() == c);
             RC_ASSERT(!seq.next());
         });

    prop("copies are equal",
         [](const std::string &a,
             const std::string &b,
             const std::string &c)
         {
             assertEqualCopies(seq::just(a, b, c));
         });
}

struct FromContainerTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "the returned elements are equal to the elements"
            " returned by iterating",
            [](const T &elements) {
                auto seq = seq::fromContainer(elements);
                for (const auto &x : elements) {
                    RC_ASSERT(*seq.next() == x);
                }
                RC_ASSERT(!seq.next());
            });

        templatedProp<T>(
            "copies are equal",
            [](const T &elements) {
                assertEqualCopies(seq::fromContainer(elements));
            });
    }
};

struct FromContainerCopyTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "does not copy elements",
            [](T elements) {
                auto seq = seq::fromContainer(std::move(elements));
                while (seq.next());
            });
    }
};

TEST_CASE("seq::fromContainer") {
    meta::forEachType<FromContainerTests,
                      RC_ORDERED_CONTAINERS(std::string),
                      RC_STRING_TYPES,
                      std::array<std::string, 100>>();

    // TODO Weirdly arbitrary to run this for a category called
    // RC_SEQUENCE_CONTAINERS
    meta::forEachType<FromContainerCopyTests,
                      RC_SEQUENCE_CONTAINERS(CopyGuard),
                      std::array<CopyGuard, 100>>();
}

TEST_CASE("seq::fromIteratorRange") {
    prop("creates a sequence that returns the values in the range",
         [] (const std::vector<int> &elements) {
             const int start = *gen::ranged<int>(0, elements.size());
             const int end = *gen::ranged<int>(start, elements.size());
             const auto startIt = elements.begin() + start;
             const auto endIt = elements.begin() + end;
             std::vector<int> subElements(startIt, endIt);
             RC_ASSERT(seq::fromIteratorRange(startIt, endIt) ==
                       seq::fromContainer(subElements));
         });
}

TEST_CASE("seq::iterate") {
    prop("returns an infinite sequence of applying the given function to the"
         " value",
         [](int start, int inc) {
             const auto func = [=](int x) { return x + inc; };
             auto seq = seq::iterate(start, func);
             int x = start;
             // Arbitrary number of iterations, sequence is infinite
             for (int i = 0; i < 1000; i++) {
                 RC_ASSERT(*seq.next() == x);
                 x = func(x);
             }
         });

    prop("copies are equal",
         [](int start, int inc) {
             const auto func = [=](int x) { return x + inc; };
             assertEqualCopies(seq::take(1000, seq::iterate(start, func)));
         });
}


TEST_CASE("seq::range") {
    auto smallInt = gen::scale(0.25, gen::arbitrary<int>());

    prop("returns a sequence from start to end",
         [&]{
             int start = *smallInt;
             int end = *smallInt;

             auto seq = seq::range(start, end);
             int inc = (start < end) ? 1 : -1;
             int i = start;
             while (i != end) {
                 auto x = seq.next();
                 RC_ASSERT(x);
                 RC_ASSERT(*x == i);
                 i += inc;
             }
             RC_ASSERT(!seq.next());
         });

    prop("copies are equal",
         [&]{ assertEqualCopies(seq::range(*smallInt, *smallInt)); });
}

TEST_CASE("seq::index") {
    prop("returns an infinite sequence of increasing indexes from 0",
         []{
             auto seq = seq::index();
             for (std::size_t i = 0; i < 2000; i++) {
                 auto x = seq.next();
                 RC_ASSERT(x);
                 RC_ASSERT(*x == i);
             }
         });

    prop("copies are equal",
         []{ assertEqualCopies(seq::take(2000, seq::index())); });
}

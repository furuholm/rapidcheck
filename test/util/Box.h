#pragma once

namespace rc {
namespace test {

//! Utility for testing showValue(...) implementations to ensure that they are
//! using rc::show(...) for their sub values. Implicit conversion to its string
//! value.
struct Box
{
    Box(int x) : value(x) {}
    int value;

    std::string str() const { return "|" + std::to_string(value) + "|"; }
};

inline void showValue(const Box &value, std::ostream &os)
{
    os << value.str();
}

} // namespace test

template<>
class Arbitrary<test::Box> : public gen::Generator<test::Box>
{
public:
    test::Box generate() const override
    { return test::Box(*gen::arbitrary<int>()); }

    Seq<test::Box> shrink(const test::Box &box)
    {
        return seq::map([](int x) { return test::Box(x); },
                        gen::arbitrary<int>().shrink(box.value));
    }
};

} // namespace rc

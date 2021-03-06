#pragma once

template<typename T>
void doShow(const T &value, std::ostream &os)
{
    using namespace rc;
    show(value, os);
}

namespace rc {
namespace detail {

template<typename T>
Rose<T>::Rose(const gen::Generator<T> *generator, const TestCase &testCase)
    : m_generator(generator)
    , m_testCase(testCase)
{
    // Initialize the tree with the test case.
    currentValue();
}

template<typename T>
T Rose<T>::currentValue()
{
    ImplicitScope scope;
    ImplicitParam<param::Random> random{Random(m_testCase.seed)};
    ImplicitParam<param::Size> size(m_testCase.size);

    return m_root.currentValue(m_generator).template get<T>();
}

template<typename T>
T Rose<T>::nextShrink(bool &didShrink)
{
    ImplicitScope scope;
    ImplicitParam<param::Random> random{Random(m_testCase.seed)};
    ImplicitParam<param::Size> size(m_testCase.size);

    return m_root.nextShrink(m_generator, didShrink).template get<T>();
}

template<typename T>
void Rose<T>::acceptShrink()
{
    return m_root.acceptShrink();
}

template<typename T>
std::vector<std::pair<std::string, std::string>> Rose<T>::example()
{
    ImplicitScope scope;
    ImplicitParam<param::Random> random{Random(m_testCase.seed)};
    ImplicitParam<param::Size> size(m_testCase.size);

    return m_root.example(m_generator);
}

} // namespace detail
} // namespace rc

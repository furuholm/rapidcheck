#include "rapidcheck/detail/Results.h"

#include "rapidcheck/Show.h"

namespace rc {
namespace detail {

bool operator==(const TestCase &r1, const TestCase &r2)
{
    return
        (r1.size == r2.size) &&
        (r1.seed == r2.seed);
}

bool operator!=(const TestCase &r1, const TestCase &r2)
{ return !(r1 == r2); }

std::ostream &operator<<(std::ostream &os, const detail::TestCase &testCase)
{
    os << "seed=" << testCase.seed << ", size=" << testCase.size;
    return os;
}

CaseResult::CaseResult()
    : type(CaseResult::Type::Failure) {}

CaseResult::CaseResult(Type t, std::string desc)
    : type(t)
    , description(desc) {}

bool operator==(const CaseResult &r1, const CaseResult &r2)
{
    return
        (r1.type == r2.type) &&
        (r1.description == r2.description);
}

bool operator!=(const CaseResult &r1, const CaseResult &r2)
{ return !(r1 == r2); }

bool operator==(const SuccessResult &r1, const SuccessResult &r2)
{ return r1.numSuccess == r2.numSuccess; }

bool operator!=(const SuccessResult &r1, const SuccessResult &r2)
{ return !(r1 == r2); }

std::ostream &operator<<(std::ostream &os, const detail::SuccessResult &result)
{
    os << "numSuccess=" << result.numSuccess;
    return os;
}

bool operator==(const FailureResult &r1, const FailureResult &r2)
{
    return
        (r1.numSuccess == r2.numSuccess) &&
        (r1.failingCase == r2.failingCase) &&
        (r1.description == r2.description) &&
        (r1.numShrinks == r2.numShrinks) &&
        (r1.counterExample == r2.counterExample);
}

bool operator!=(const FailureResult &r1, const FailureResult &r2)
{ return !(r1 == r2); }

std::ostream &operator<<(std::ostream &os, const detail::FailureResult &result)
{
    os << "numSuccess=" << result.numSuccess
       << ", failingCase=<" << result.failingCase << ">"
       << ", description='" << result.description << "'"
       << ", numShrinks=" << result.numShrinks
       << ", counterExample=";
    show(result.counterExample, os);
    return os;
}

bool operator==(const GaveUpResult &r1, const GaveUpResult &r2)
{
    return
        (r1.numSuccess == r2.numSuccess) &&
        (r1.description == r2.description);
}

bool operator!=(const GaveUpResult &r1, const GaveUpResult &r2)
{ return !(r1 == r2); }

std::ostream &operator<<(std::ostream &os, const detail::GaveUpResult &result)
{
    os << "numSuccess=" << result.numSuccess
       << ", description='" << result.description << "'";
    return os;
}

std::string resultMessage(const TestResult &result)
{
    SuccessResult success;
    FailureResult failure;
    GaveUpResult gaveUp;

    if (result.match(success)) {
        return "OK, passed " + std::to_string(success.numSuccess) + " tests";
    } else if (result.match(failure)) {
        std::string msg;
        msg += "Falsifiable after " + std::to_string(failure.numSuccess + 1);
        msg += " tests";
        if (failure.numShrinks > 0) {
            msg += " and " + std::to_string(failure.numShrinks);
            msg += (failure.numShrinks == 1) ? " shrink" : " shrinks";
        }

        return msg;
    } else if (result.match(gaveUp)) {
        return "Gave up after " + std::to_string(gaveUp.numSuccess) + " tests";
    }

    return std::string();
}

std::ostream &operator<<(std::ostream &os, CaseResult::Type type)
{
    switch (type) {
    case CaseResult::Type::Success:
        os << "Success";
        break;

    case CaseResult::Type::Failure:
        os << "Failure";
        break;

    case CaseResult::Type::Discard:
        os << "Discard";
        break;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, const CaseResult &result)
{
    os << result.type << ": " << result.description;
    return os;
}

} // namespace detail
} // namespace rc

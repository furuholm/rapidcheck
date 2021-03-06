#pragma once

#include "rapidcheck/gen/Generator.h"

namespace rc {
namespace detail {

//! Global suite configuration.
struct Configuration
{
    //! The global seed. The actual seed for a particular test case will be a
    //! hash of this and some specific data which identifies that case.
    RandomEngine::Seed seed = 0;

    //! The number of test cases for each property.
    int maxSuccess = 100;

    //! The maximum size for each property.
    int maxSize = gen::kNominalSize;

    //! The maximum discard ratio.
    int maxDiscardRatio = 10;
};

std::ostream &operator<<(std::ostream &os, const Configuration &config);
bool operator==(const Configuration &c1, const Configuration &c2);
bool operator!=(const Configuration &c1, const Configuration &c2);

//! Thrown to indicate something went wrong when loading configuration.
class ConfigurationException : public std::exception
{
public:
    ConfigurationException(std::string msg);
    const char *what() const noexcept override;

private:
    std::string m_msg;
};

//! Reads a `Configuration` from the given string.
//!
//! 
Configuration configFromString(const std::string &str,
                               const Configuration &defaults = Configuration());

//! Returns a configuration string from a `Configuration`.
std::string configToString(const Configuration &config);

//! Returns a configuration string from a `Configuration` that only contains the
//! keys that differ from the default configuration.
std::string configToMinimalString(const Configuration &config);

//! Returns the default configuration.
const Configuration &defaultConfiguration();

namespace param {

//! ImplicitParam containing the current configuration.
struct CurrentConfiguration
{
    typedef Configuration ValueType;
    static Configuration defaultValue() { return defaultConfiguration(); }
};

} // namespace param

} // namespace detail
} // namespace rc

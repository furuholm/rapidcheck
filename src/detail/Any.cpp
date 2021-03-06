#include "rapidcheck/detail/Any.h"

namespace rc {
namespace detail {

Any::Any() noexcept {}

Any::Any(const Any &other)
    : m_impl(other.m_impl
             ? other.m_impl->copy()
             : nullptr) {}

Any &Any::operator=(const Any &rhs)
{
    if (rhs.m_impl)
        m_impl = rhs.m_impl->copy();
    else
        m_impl.reset();
    return *this;
}

void Any::reset() { m_impl.reset(); }

std::pair<std::string, std::string> Any::describe() const
{
    assert(m_impl);
    return m_impl
        ? m_impl->describe()
        : std::make_pair(std::string(), std::string());
}

Any::operator bool() const { return static_cast<bool>(m_impl); }

bool Any::isCopyable() const { return !m_impl || m_impl->isCopyable(); }

std::ostream &operator<<(std::ostream &os, const Any &value)
{
    auto desc = value.describe();
    os << desc.second << " (" << desc.first << ")";
    return os;
}

} // namespace detail
} // namespace rc

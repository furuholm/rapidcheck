#pragma once

#include "rapidcheck/Show.h"

namespace rc {

template<typename T>
class Seq<T>::ISeqImpl
{
public:
    virtual Maybe<T> next() = 0;
    virtual std::unique_ptr<ISeqImpl> copy() const = 0;
    virtual ~ISeqImpl() = default;
};

template<typename T>
template<typename Impl>
class Seq<T>::SeqImpl : public Seq<T>::ISeqImpl
{
public:
    template<typename ...Args>
    SeqImpl(Args &&...args) : m_impl(std::forward<Args>(args)...) {}

    Maybe<T> next() override { return m_impl(); }

    std::unique_ptr<ISeqImpl> copy() const override
    { return std::unique_ptr<ISeqImpl>(new SeqImpl(*this)); }

private:
    Impl m_impl;
};

template<typename T>
template<typename Impl, typename>
Seq<T>::Seq(Impl &&impl)
    : m_impl(new SeqImpl<Decay<Impl>>(std::forward<Impl>(impl))) {}

template<typename T>
Maybe<T> Seq<T>::next()
{ return m_impl ? m_impl->next() : Nothing; }

template<typename T>
Seq<T>::Seq(const Seq &other)
    : m_impl(other.m_impl ? other.m_impl->copy() : nullptr) {}

template<typename T>
Seq<T> &Seq<T>::operator=(const Seq &rhs)
{
    m_impl = rhs.m_impl->copy();
    return *this;
}

template<typename Impl, typename ...Args>
Seq<typename std::result_of<Impl()>::type::ValueType> makeSeq(Args &&...args)
{
    typedef Seq<typename std::result_of<Impl()>::type::ValueType> SeqT;
    typedef typename SeqT::template SeqImpl<Impl> ImplT;

    SeqT seq;
    seq.m_impl.reset(new ImplT(std::forward<Args>(args)...));
    return seq;
}

template<typename A, typename B>
bool operator==(Seq<A> lhs, Seq<B> rhs)
{
    while (true) {
        Maybe<A> a(lhs.next());
        Maybe<B> b(rhs.next());
        if (a != b)
            return false;

        if (!a && !b)
            return true;
    }
}

template<typename A, typename B>
bool operator!=(Seq<A> lhs, Seq<B> rhs)
{ return !(std::move(lhs) == std::move(rhs)); }

template<typename T>
std::ostream &operator<<(std::ostream &os, Seq<T> seq)
{
    os << "[";
    int n = 1;
    auto first = seq.next();
    if (first) {
        show(*first, os);
        while (true) {
            const auto value = seq.next();
            if (!value)
                break;

            os << ", ";
            // Don't print infinite sequences...
            if (n++ >= 1000) {
                os << "...";
                break;
            }

            show(*value, os);
        }
    }
    os << "]";
    return os;
}

} // namespace rc

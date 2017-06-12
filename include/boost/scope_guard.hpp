
//                     Copyright Yuri Kilochek 2017.
//        Distributed under the Boost Software License, Version 1.0.
//           (See accompanying file LICENSE_1_0.txt or copy at
//                 http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SCOPE_GUARD_HPP
#define BOOST_SCOPE_GUARD_HPP

#if __cplusplus < 201703L
    #error Boost.ScopeGuard requires C++17 or later.
#endif

#include <functional>
#include <type_traits>
#include <utility>
#include <tuple>
#include <exception>

#include <boost/config.hpp>

namespace boost {
///////////////////////////////////////////////////////////////////////////////

namespace detail::scope_guard {
    template <typename Fn, typename Args, std::size_t... I>
    auto apply_impl(Fn&& fn, Args&& args, std::index_sequence<I...>)
    noexcept(noexcept(std::invoke(
        std::forward<Fn>(fn), std::get<I>(std::forward<Args>(args))...)))
    -> decltype(      std::invoke(
        std::forward<Fn>(fn), std::get<I>(std::forward<Args>(args))...))
    { return          std::invoke(
        std::forward<Fn>(fn), std::get<I>(std::forward<Args>(args))...); }
     
    // Like `std::apply` but SFINAE friendly and propagates `noexcept`ness.
    template <class Fn, typename Args>
    auto apply(Fn&& fn, Args&& args)
    noexcept(noexcept(apply_impl(
        std::forward<Fn>(fn), std::forward<Args>(args),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Args>>>())))
    -> decltype(      apply_impl(
        std::forward<Fn>(fn), std::forward<Args>(args),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Args>>>()))
    { return          apply_impl(
        std::forward<Fn>(fn), std::forward<Args>(args),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Args>>>()); }

    template <typename Fn, typename... Args>
    class callback
    {
        Fn fn;
        std::tuple<Args...> args;

    public:
        template <typename Fn_, typename... Args_, std::enable_if_t<
            (std::is_constructible_v<Fn, Fn_> && ... &&
             std::is_constructible_v<Args, Args_>)>*...>
        explicit callback(Fn_&& fn, Args_&&... args)
        noexcept((std::is_nothrow_constructible_v<Fn, Fn_> && ... &&
                  std::is_nothrow_constructible_v<Args, Args_>))
        : fn(std::forward<Fn_>(fn))
        , args(std::forward<Args_>(args)...)
        {}

        template <typename Fn_ = Fn>
        auto operator()()
        noexcept(noexcept((void)scope_guard::apply(
            std::forward<Fn_>(fn), std::move(args))))
        -> decltype(      (void)scope_guard::apply(
            std::forward<Fn_>(fn), std::move(args)))
        { return          (void)scope_guard::apply(
            std::forward<Fn_>(fn), std::move(args)); }
    };

    template <typename... Params>
    class base
    {
    protected:
        using callback_type = detail::scope_guard::callback<Params...>;

        callback_type callback;

    public:
        static_assert(std::is_invocable_v<callback_type>,
            "callback not invocable");

        template <typename... Params_, std::enable_if_t<
            std::is_constructible_v<callback_type, Params...>>*...>
        explicit base(Params_&&... params)
        noexcept(std::is_nothrow_constructible_v<callback_type, Params...>)
        : callback(std::forward<Params_>(params)...)
        {}

        base(base const&) = delete;
        auto operator=(base const&)
        -> base&
        = delete;

        base(base&&) = delete;
        auto operator=(base&&)
        -> base&
        = delete;
    };

    template <typename T>
    struct unwrap
    { using type = T; };

    template <typename T>
    struct unwrap<std::reference_wrapper<T>>
    { using type = T&; };

    template <typename T>
    using unwrap_decay_t = typename unwrap<std::decay_t<T>>::type;
} // detail::scope_guard

template <typename... Params>
struct scope_guard
: detail::scope_guard::base<Params...>
{
    using base_type = detail::scope_guard::base<Params...>;
    using this_type = scope_guard;

public:
    using base_type::base_type;

    ~scope_guard()
    noexcept(noexcept(this_type::callback()) &&
             std::is_nothrow_destructible_v<base_type>)
    { this_type::callback(); }
};

template <typename... Params>
scope_guard(Params&&...)
-> scope_guard<detail::scope_guard::unwrap_decay_t<Params>...>;

template <typename... Params>
struct scope_guard_failure
: detail::scope_guard::base<Params...>
{
    using base_type = detail::scope_guard::base<Params...>;
    using this_type = scope_guard_failure;

    int in = std::uncaught_exceptions();

public:
    using detail::scope_guard::base<Params...>::base;

    ~scope_guard_failure()
    noexcept(noexcept(this_type::callback()) &&
             std::is_nothrow_destructible_v<base_type>)
    {
        int out = std::uncaught_exceptions();
        if (BOOST_UNLIKELY(out > in)) { this_type::callback(); }
    }
};

template <typename... Params>
scope_guard_failure(Params&&...)
-> scope_guard_failure<detail::scope_guard::unwrap_decay_t<Params>...>;

template <typename... Params>
struct scope_guard_success
: detail::scope_guard::base<Params...>
{
    using base_type = detail::scope_guard::base<Params...>;
    using this_type = scope_guard_success;

    int in = std::uncaught_exceptions();

public:
    using detail::scope_guard::base<Params...>::base;

    ~scope_guard_success()
    noexcept(noexcept(this_type::callback()) &&
             std::is_nothrow_destructible_v<base_type>)
    {
        int out = std::uncaught_exceptions();
        if (BOOST_LIKELY(out == in)) { this_type::callback(); }
    }
};

template <typename... Params>
scope_guard_success(Params&&...)
-> scope_guard_success<detail::scope_guard::unwrap_decay_t<Params>...>;

///////////////////////////////////////////////////////////////////////////////
} // boost

#endif


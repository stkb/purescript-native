///////////////////////////////////////////////////////////////////////////////
//
// Module      :  purescript_rc.h
// Copyright   :  (c) Andy Arvanitis 2018
// License     :  BSD
//
// Maintainer  :  Andy Arvanitis
// Stability   :  experimental
// Portability :
//
// Reference counting (std::shared_ptr) version of basic types and functions to support
// purescript-to-C++ rendering.
//
///////////////////////////////////////////////////////////////////////////////
//
#ifndef purescript_rc_H
#define purescript_rc_H

#include <memory>
#include <functional>
#include <deque>
#include <string>
#include <limits>
#include "string_literal_dict.h"


namespace purescript {

    using std::string;

    class boxed : public std::shared_ptr<void> {

    public:
        using fn_t = std::function<boxed(const boxed&)>;
        using eff_fn_t = std::function<boxed(void)>;
        using dict_t = string_literal_dict_t<boxed>;
        using array_t = std::deque<boxed>;

    public:
        using std::shared_ptr<void>::shared_ptr;

        boxed() noexcept : std::shared_ptr<void>() {}
        boxed(const std::nullptr_t) noexcept : std::shared_ptr<void>() {}
        boxed(const int n) : std::shared_ptr<void>(std::make_shared<int>(n)) {}
        boxed(const long n) : std::shared_ptr<void>(std::make_shared<int>(static_cast<int>(n))) {
#if !defined(NDEBUG) // if debug build
            if (n < std::numeric_limits<int>::min() || n > std::numeric_limits<int>::max()) {
                throw std::runtime_error("integer out of range");
            }
#endif // !defined(NDEBUG)
        }
        boxed(const unsigned long n) : std::shared_ptr<void>(std::make_shared<int>(static_cast<int>(n))) {
#if !defined(NDEBUG) // if debug build
            if (n > std::numeric_limits<int>::max()) {
                throw std::runtime_error("integer out of range");
            }
#endif // !defined(NDEBUG)
        }
        boxed(const double n) : std::shared_ptr<void>(std::make_shared<double>(n)) {}
        boxed(const bool b) : std::shared_ptr<void>(std::make_shared<bool>(b)) {}
        boxed(const char s[]) : std::shared_ptr<void>(std::make_shared<string>(s)) {}
        boxed(string&& s) : std::shared_ptr<void>(std::make_shared<string>(std::move(s))) {}
        boxed(const string& s) : std::shared_ptr<void>(std::make_shared<string>(s)) {}
        boxed(array_t&& l) : std::shared_ptr<void>(std::make_shared<array_t>(std::move(l))) {}
        boxed(const array_t& l) : std::shared_ptr<void>(std::make_shared<array_t>(l)) {}
        boxed(dict_t&& m) : std::shared_ptr<void>(std::make_shared<dict_t>(std::move(m))) {}
        boxed(const dict_t& m) : std::shared_ptr<void>(std::make_shared<dict_t>(m)) {}

        template <typename T,
                  typename = typename std::enable_if<!std::is_same<boxed,T>::value>::type>
        boxed(const T& f,
              typename std::enable_if<std::is_assignable<std::function<boxed(boxed)>,T>::value>::type* = 0)
              : std::shared_ptr<void>(std::make_shared<fn_t>(f)) {
        }

        template <typename T,
                  typename = typename std::enable_if<!std::is_same<boxed,T>::value>::type>
        boxed(const T& f,
              typename std::enable_if<std::is_assignable<std::function<boxed(void)>,T>::value>::type* = 0)
              : std::shared_ptr<void>(std::make_shared<eff_fn_t>(f)) {
        }

        auto operator()(const boxed& arg) const -> boxed {
            auto& f = *static_cast<fn_t*>(get());
            return f(arg);
        }

        auto operator()() const -> boxed {
            auto& f = *static_cast<eff_fn_t*>(get());
            return f();
        }

        auto operator[](const char key[]) const -> const boxed& {
          return (*static_cast<const dict_t*>(get()))[key];
        }

        auto operator[](const char key[]) -> boxed& {
          return (*static_cast<dict_t*>(get()))[key];
        }

#if !defined(NDEBUG) // if debug build
        auto operator[](const int index) const -> const boxed& {
            return static_cast<const array_t*>(get())->at(index);
        }

        auto operator[](const int index) -> boxed& {
            return static_cast<array_t*>(get())->at(index);
        }
#else  // not debug build
        auto operator[](const int index) const -> const boxed& {
            return (*static_cast<const array_t*>(get()))[index];
        }

        auto operator[](const int index) -> boxed& {
            return (*static_cast<array_t*>(get()))[index];
        }
#endif // !defined(NDEBUG)

    }; // class boxed

    using fn_t = boxed::fn_t;
    using eff_fn_t = boxed::eff_fn_t;
    using dict_t = boxed::dict_t;
    using array_t = boxed::array_t;

    class boxed_r : private std::shared_ptr<void> {
        public:
        boxed_r() : std::shared_ptr<void>(std::make_shared<boxed>()) {}

        operator const boxed&() const {
            return *static_cast<const boxed*>(get());
        }

        auto operator()(const boxed& arg) const -> boxed {
            auto& b = *static_cast<boxed*>(get());
            auto& f = *static_cast<fn_t*>(b.get());
            return f(arg);
        }

        auto operator()() const -> boxed {
            auto& b = *static_cast<boxed*>(get());
            auto& f = *static_cast<eff_fn_t*>(b.get());
            return f();
        }

        template <typename T>
        auto operator=(T&& right) -> boxed_r& {
            auto& b = *static_cast<boxed*>(get());
            b = std::forward<T>(right);
            return *this;
        }

    }; // class boxed_r

    template <typename T, typename... Args>
    inline auto box(Args&&... args) -> boxed {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    constexpr auto unbox(const boxed& b) -> const T& {
        return *static_cast<const T*>(b.get());
    }

    template <typename T>
    constexpr auto unbox(boxed& b) -> T& {
        return *static_cast<T*>(b.get());
    }

    template <typename T>
    constexpr auto unbox(const T value) -> T {
        return value;
    }

    template <typename T,
              typename = typename std::enable_if<std::is_same<T, int>::value>::type>
    constexpr auto unbox(const std::size_t value) -> long long {
        return value;
    }

    inline auto array_length(const boxed& a) -> boxed::array_t::size_type {
        return unbox<boxed::array_t>(a).size();
    }

    constexpr auto undefined = nullptr;

} // namespace purescript

#define DEFINE_FOREIGN_DICTIONARY_AND_ACCESSOR() \
    inline auto foreign() -> dict_t& {\
        static dict_t $dict$;\
        return $dict$;\
    }

#define FOREIGN_BEGIN(NS) namespace NS {\
    using namespace purescript;\
    DEFINE_FOREIGN_DICTIONARY_AND_ACCESSOR()\
    static const auto $foreign_exports_init$ = ([]() -> char {\
        dict_t& exports = foreign();
#define FOREIGN_END return 0; }()); }

#define INITIALIZE_GC() // do nothing

#endif // purescript_rc_H
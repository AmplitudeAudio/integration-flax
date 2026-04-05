#pragma once

// ============================================================================
//  Cpp20Compat — C++20 polyfills for building under C++17
//
//  Flax Engine's bundled fmt library is incompatible with C++20, so this
//  plugin compiles with -std=c++17. The Amplitude Audio SDK headers use a
//  few C++20 features; this header provides minimal stand-ins so they
//  compile under C++17.
//
//  IMPORTANT: include this header BEFORE any Amplitude SDK headers.
// ============================================================================

#if __cplusplus < 202002L

// ---- std::span polyfill ---------------------------------------------------
#ifndef __cpp_lib_span
#include <cstddef>
#include <type_traits>

namespace std
{
    template<typename T>
    class span
    {
    public:
        using element_type    = T;
        using value_type      = std::remove_cv_t<T>;
        using size_type       = std::size_t;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using iterator        = pointer;

        constexpr span() noexcept : m_data(nullptr), m_size(0) {}
        constexpr span(pointer data, size_type size) noexcept : m_data(data), m_size(size) {}

        template<std::size_t N>
        constexpr span(T (&arr)[N]) noexcept : m_data(arr), m_size(N) {}

        constexpr pointer   data()  const noexcept { return m_data; }
        constexpr size_type size()  const noexcept { return m_size; }
        constexpr bool      empty() const noexcept { return m_size == 0; }

        constexpr reference operator[](size_type idx) const { return m_data[idx]; }

        constexpr iterator begin() const noexcept { return m_data; }
        constexpr iterator end()   const noexcept { return m_data + m_size; }

    private:
        pointer   m_data;
        size_type m_size;
    };
} // namespace std
#endif // __cpp_lib_span

// ---- std::is_pointer_v / std::is_void_v (C++17 variable templates) --------
// These should already be available in C++17 via <type_traits>, but if the
// compiler's standard library does not provide them, polyfill here.
#if !defined(__cpp_lib_type_trait_variable_templates) || __cpp_lib_type_trait_variable_templates < 201510L
#include <type_traits>
namespace std
{
    template<typename T>
    inline constexpr bool is_pointer_v = std::is_pointer<T>::value;

    template<typename T>
    inline constexpr bool is_void_v = std::is_void<T>::value;
} // namespace std
#endif

#endif // __cplusplus < 202002L

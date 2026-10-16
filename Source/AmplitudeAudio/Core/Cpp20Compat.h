// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     `http://www.apache.org/licenses/LICENSE-2.0`
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

// ============================================================================
//  Windows header isolation
//
//  The Amplitude SDK includes <Windows.h> directly (in Config.h) without
//  the guards that Flax Engine uses.  We pre-include it here with the same
//  exclusions Flax uses (NOGDI, etc.) and then #undef the macros that
//  collide with Flax types/methods.  The pull and the #undef block share a
//  project-local guard so the macros run exactly once, regardless of whether
//  some other header already included <Windows.h> earlier.
// ============================================================================
#if PLATFORM_WIN32 || PLATFORM_WINDOWS || defined(_WIN32)
#ifndef AMPLITUDE_WINDOWS_H_PULLED
#define AMPLITUDE_WINDOWS_H_PULLED

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NODRAWTEXT
#define NODRAWTEXT
#endif
#ifndef NOCTLMGR
#define NOCTLMGR
#endif
#ifndef NOFLATSBAPIS
#define NOFLATSBAPIS
#endif

#include <Windows.h>

// Remove Windows macros that conflict with Flax Engine identifiers.
// This list runs ONCE under our own guard; we never #undef on a second
// inclusion of this header.
#undef MemoryBarrier
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef CreateDirectory
#undef GetComputerName
#undef GetUserName
#undef MessageBox
#undef GetCommandLine
#undef CreateWindow
#undef CreateProcess
#undef SetWindowText
#undef DrawText
#undef CreateFont
#undef IsMinimized
#undef IsMaximized
#undef LoadIcon
#undef InterlockedOr
#undef InterlockedAnd
#undef InterlockedExchange
#undef InterlockedCompareExchange
#undef InterlockedIncrement
#undef InterlockedDecrement
#undef InterlockedAdd
#undef GetObject
#undef GetClassName
#undef GetMessage
#undef CreateMutex
#undef DrawState
#undef LoadLibrary
#undef Yield
#undef GetEnvironmentVariable
#undef SetEnvironmentVariable
#undef small
#undef IGNORE
#undef OPTIONAL
#undef Rectangle
#undef IN
#undef OUT
#endif // AMPLITUDE_WINDOWS_H_PULLED
#endif // PLATFORM_WIN32 || PLATFORM_WINDOWS || _WIN32

#if __cplusplus < 202002L

// ---- std::span polyfill ---------------------------------------------------
// Defined in a project namespace and injected into std via a using-alias.
// [namespace.std] forbids adding new class templates to namespace std, but
// permits a using-alias. When a future standard library exposes std::span,
// it must set __cpp_lib_span and this block compiles to nothing.
#ifndef __cpp_lib_span
#include <cstddef>
#include <type_traits>

namespace AmplitudeAudio
{
namespace cpp20
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
} // namespace cpp20
} // namespace AmplitudeAudio

namespace std
{
    template<typename T> using span = AmplitudeAudio::cpp20::span<T>;
}
#endif // __cpp_lib_span

// ---- std::is_pointer_v / std::is_void_v (C++17 variable templates) --------
// These should already be available in C++17 via <type_traits>, but if the
// compiler's standard library does not provide them, polyfill via using-alias.
#if !defined(__cpp_lib_type_trait_variable_templates) || __cpp_lib_type_trait_variable_templates < 201510L
#include <type_traits>
namespace AmplitudeAudio
{
namespace cpp20
{
    template<typename T> inline constexpr bool is_pointer_v = std::is_pointer<T>::value;
    template<typename T> inline constexpr bool is_void_v    = std::is_void<T>::value;
} // namespace cpp20
} // namespace AmplitudeAudio

namespace std
{
    using AmplitudeAudio::cpp20::is_pointer_v;
    using AmplitudeAudio::cpp20::is_void_v;
}
#endif

#endif // __cplusplus < 202002L

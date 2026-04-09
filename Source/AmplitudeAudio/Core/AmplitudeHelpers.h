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
//  AmplitudeHelpers — Core helper utilities for the Amplitude Audio plugin
//
//  This file declares three independent components:
//    1. FlaxMemoryAllocator  – routes Amplitude memory through Flax Platform APIs
//    2. FlaxLogger           – bridges Amplitude log levels to Flax LOG macros
//    3. Coordinate helpers   – convert Flax spatial types to Amplitude types
// ============================================================================

// clang-formatt off
#include "Cpp20Compat.h"
// clang-format on

#include <AudioSystem/Core/AudioSystemData.h>
#include <Engine/Core/Math/Vector3.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>
#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;
    // ========================================================================
    //  FlaxMemoryAllocator
    // ========================================================================

    /// <summary>Amplitude MemoryAllocator implementation backed by Flax Platform APIs.</summary>
    /// <remarks>
    /// All Amplitude memory-pool kinds are routed to the same underlying
    /// Flax allocator; the pool parameter is ignored for routing but is
    /// available for future diagnostic use.
    /// </remarks>
    class FlaxMemoryAllocator final : public MemoryAllocator
    {
      public:
        /// <summary>Allocates <paramref name="size"/> bytes from the given Amplitude pool.</summary>
        AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size) override;

        /// <summary>Reallocates an existing allocation to the new <paramref name="size"/>.</summary>
        AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size) override;

        /// <summary>Allocates <paramref name="size"/> bytes with the requested <paramref name="alignment"/>.</summary>
        AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment) override;

        /// <summary>Reallocates an existing aligned allocation to the new <paramref name="size"/> and <paramref name="alignment"/>.</summary>
        AmVoidPtr Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) override;

        /// <summary>Frees a previously allocated block.</summary>
        void Free(eMemoryPoolKind pool, AmVoidPtr address) override;

        /// <summary>Returns the usable size of the block at <paramref name="address"/>.</summary>
        /// <remarks>
        /// Note: Flax's Platform API does not expose a size-query; this
        ///       implementation always returns 0.  Amplitude only uses
        ///       SizeOf for diagnostic/stats purposes, so returning 0 is safe.
        /// </remarks>
        AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) override;
    };

    // ========================================================================
    //  FlaxLogger
    // ========================================================================

    /// <summary>Amplitude Logger implementation that writes to Flax's log system.</summary>
    /// <remarks>
    /// Level mapping:
    ///   Debug / Info / Success -> LogType::Info
    ///   Warning                -> LogType::Warning
    ///   Error                  -> LogType::Error
    ///   Critical               -> LogType::Fatal
    /// </remarks>
    class FlaxLogger final : public Logger
    {
      protected:
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override;
    };

    // ========================================================================
    //  Coordinate conversion helpers
    // ========================================================================

    /// <summary>Converts a Flax Vector3 to an Amplitude AmVector3.</summary>
    /// <remarks>
    /// Both types use the same XYZ component layout, so this is a straight
    /// component-wise copy.
    /// </remarks>
    AmVector3 FlaxVec3ToAmVec3(const Vector3& vec);

    /// <summary>Converts an AudioSystemTransform to an Amplitude location and orientation.</summary>
    /// <param name="transform">The Flax AudioSystem spatial transform.</param>
    /// <param name="outLocation">The world-space position expressed as AmVector3.</param>
    /// <param name="outOrientation">
    /// The orientation built from the transform's
    ///             forward and up unit vectors.
    /// </param>
    void FlaxTransformToAmplitude(
        const AudioSystemTransform& transform,
        AmVector3&                  outLocation,
        Orientation&                outOrientation);

} // namespace AmplitudeAudio

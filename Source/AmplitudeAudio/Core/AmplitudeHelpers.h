#pragma once

// ============================================================================
//  AmplitudeHelpers — Core helper utilities for the Amplitude Audio plugin
//
//  This file declares three independent components:
//    1. FlaxMemoryAllocator  – routes Amplitude memory through Flax Platform APIs
//    2. FlaxLogger           – bridges Amplitude log levels to Flax LOG macros
//    3. Coordinate helpers   – convert Flax spatial types to Amplitude types
// ============================================================================

#include <Engine/Core/Math/Vector3.h>

#include <AudioSystem/Core/AudioSystemData.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>
#include <SparkyStudios/Audio/Amplitude/Math/LinearAlgebra.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

using namespace SparkyStudios::Audio::Amplitude;

namespace AmplitudeAudio
{
    // ========================================================================
    //  FlaxMemoryAllocator
    // ========================================================================

    /// \brief Amplitude MemoryAllocator implementation backed by Flax Platform APIs.
    ///
    /// All Amplitude memory-pool kinds are routed to the same underlying
    /// Flax allocator; the pool parameter is ignored for routing but is
    /// available for future diagnostic use.
    class FlaxMemoryAllocator final : public MemoryAllocator
    {
    public:
        /// Allocates \p size bytes from the given Amplitude pool.
        AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size) override;

        /// Reallocates an existing allocation to the new \p size.
        AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size) override;

        /// Allocates \p size bytes with the requested \p alignment.
        AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment) override;

        /// Reallocates an existing aligned allocation to the new \p size and \p alignment.
        AmVoidPtr Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) override;

        /// Frees a previously allocated block.
        void Free(eMemoryPoolKind pool, AmVoidPtr address) override;

        /// Returns the usable size of the block at \p address.
        ///
        /// \note Flax's Platform API does not expose a size-query; this
        ///       implementation always returns 0.  Amplitude only uses
        ///       SizeOf for diagnostic/stats purposes, so returning 0 is safe.
        AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) override;
    };

    // ========================================================================
    //  FlaxLogger
    // ========================================================================

    /// \brief Amplitude Logger implementation that writes to Flax's log system.
    ///
    /// Level mapping:
    ///   Debug / Info / Success -> LogType::Info
    ///   Warning                -> LogType::Warning
    ///   Error                  -> LogType::Error
    ///   Critical               -> LogType::Fatal
    class FlaxLogger final : public Logger
    {
    protected:
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override;
    };

    // ========================================================================
    //  Coordinate conversion helpers
    // ========================================================================

    /// \brief Converts a Flax Vector3 to an Amplitude AmVector3.
    ///
    /// Both types use the same XYZ component layout, so this is a straight
    /// component-wise copy.
    AmVector3 FlaxVec3ToAmVec3(const Vector3& vec);

    /// \brief Converts an AudioSystemTransform to an Amplitude location and orientation.
    ///
    /// \param[in]  transform   The Flax AudioSystem spatial transform.
    /// \param[out] outLocation The world-space position expressed as AmVector3.
    /// \param[out] outOrientation The orientation built from the transform's
    ///             forward and up unit vectors.
    void FlaxTransformToAmplitude(
        const AudioSystemTransform& transform,
        AmVector3& outLocation,
        Orientation& outOrientation);

} // namespace AmplitudeAudio

// ============================================================================
//  AmplitudeHelpers — Core helper utilities for the Amplitude Audio plugin
//  (implementation)
// ============================================================================

#include "AmplitudeHelpers.h"

#include <Engine/Core/Log.h>
#include <Engine/Platform/Platform.h>

#include <cstdlib>   // ::realloc, ::malloc_usable_size / ::_msize
#include <cstring>   // ::memcpy

namespace AmplitudeAudio
{
    // ========================================================================
    //  FlaxMemoryAllocator
    // ========================================================================

    AmVoidPtr FlaxMemoryAllocator::Malloc(eMemoryPoolKind /*pool*/, AmSize size)
    {
        // Use alignment of 1 to signal "no special alignment requirement".
        // Platform::Allocate still satisfies at least pointer-sized alignment.
        return Platform::Allocate(static_cast<uint64>(size), 1u);
    }

    AmVoidPtr FlaxMemoryAllocator::Realloc(eMemoryPoolKind /*pool*/, AmVoidPtr address, AmSize size)
    {
        // Flax's Platform API does not expose a realloc primitive, so fall
        // back to the C standard library realloc which is safe to mix with
        // Platform::Free on all supported platforms.
        return ::realloc(address, size);
    }

    AmVoidPtr FlaxMemoryAllocator::Malign(eMemoryPoolKind /*pool*/, AmSize size, AmUInt32 alignment)
    {
        return Platform::Allocate(static_cast<uint64>(size), static_cast<uint64>(alignment));
    }

    AmVoidPtr FlaxMemoryAllocator::Realign(
        eMemoryPoolKind /*pool*/, AmVoidPtr address, AmSize size, AmUInt32 alignment)
    {
        // Flax's Platform API provides no aligned-realloc primitive.
        // Allocate a fresh block at the required alignment, copy the old
        // contents (up to the new size), then release the original block.
        // Callers that need the previous size for the copy are responsible
        // for not passing a size larger than what was originally allocated.
        AmVoidPtr newBlock = Platform::Allocate(static_cast<uint64>(size), static_cast<uint64>(alignment));
        if (newBlock != nullptr && address != nullptr)
        {
            // Copy at most 'size' bytes – we have no way of knowing the
            // exact old size here, so the caller must ensure this is safe.
            ::memcpy(newBlock, address, size);
            Platform::Free(address);
        }
        return newBlock;
    }

    void FlaxMemoryAllocator::Free(eMemoryPoolKind /*pool*/, AmVoidPtr address)
    {
        Platform::Free(address);
    }

    AmSize FlaxMemoryAllocator::SizeOf(eMemoryPoolKind /*pool*/, AmVoidPtr /*address*/)
    {
        // Flax's Platform API does not expose a block-size query.
        // Amplitude uses SizeOf only for memory statistics; returning 0 is
        // a safe no-op that prevents stat accumulation for this allocator.
        return 0;
    }

    // ========================================================================
    //  FlaxLogger
    // ========================================================================

    void FlaxLogger::Log(eLogMessageLevel level, const char* file, int line, const AmString& message)
    {
        // Convert the narrow AmString (std::string) to a Flax StringView for
        // the LOG_STR macro. We build a wide String on the stack.
        const ::String flaxMessage(message.c_str());

        switch (level)
        {
        case eLogMessageLevel_Debug:
        case eLogMessageLevel_Info:
        case eLogMessageLevel_Success:
            LOG_STR(Info, flaxMessage);
            break;

        case eLogMessageLevel_Warning:
            LOG_STR(Warning, flaxMessage);
            break;

        case eLogMessageLevel_Error:
            LOG_STR(Error, flaxMessage);
            break;

        case eLogMessageLevel_Critical:
            LOG_STR(Fatal, flaxMessage);
            break;

        default:
            LOG_STR(Info, flaxMessage);
            break;
        }
    }

    // ========================================================================
    //  Coordinate conversion helpers
    // ========================================================================

    AmVector3 FlaxVec3ToAmVec3(const Vector3& vec)
    {
        // Both types store components as (x, y, z) floats; a direct copy
        // is sufficient.
        const AmVector3 result = { vec.X, vec.Y, vec.Z };
        return result;
    }

    void FlaxTransformToAmplitude(
        const AudioSystemTransform& transform,
        AmVector3& outLocation,
        Orientation& outOrientation)
    {
        outLocation = FlaxVec3ToAmVec3(transform.Position);

        const AmVector3 forward = FlaxVec3ToAmVec3(transform.Forward);
        const AmVector3 up      = FlaxVec3ToAmVec3(transform.Up);

        // Orientation(forward, up) stores the canonical look direction and
        // the world-up direction so the SDK can reconstruct any rotation
        // matrix it needs.  The ezEngine integration negates forward, but the
        // Amplitude SDK's Orientation constructor takes the *actual* forward
        // vector (not its negation), so we pass it unchanged here.
        outOrientation = Orientation(forward, up);
    }

} // namespace AmplitudeAudio

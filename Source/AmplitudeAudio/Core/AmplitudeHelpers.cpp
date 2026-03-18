// ============================================================================
//  AmplitudeHelpers — Core helper utilities for the Amplitude Audio plugin
//  (implementation)
// ============================================================================

#include "AmplitudeHelpers.h"

#include <Engine/Core/Log.h>
#include <Engine/Platform/Platform.h>

#include <cstring>   // ::memcpy

namespace AmplitudeAudio
{
    // ========================================================================
    //  FlaxMemoryAllocator
    //
    //  Every allocation is prefixed with a size header so that Realloc and
    //  Realign can safely copy only the original number of bytes when growing.
    //  The header is aligned to at least 16 bytes so that the user pointer
    //  retains the requested alignment.
    // ========================================================================

    namespace
    {
        struct AllocHeader
        {
            AmSize size;
        };

        // Minimum header alignment so user pointer stays well-aligned.
        constexpr AmSize kHeaderAlignment = 16;
        constexpr AmSize kHeaderSize = ((sizeof(AllocHeader) + kHeaderAlignment - 1) / kHeaderAlignment) * kHeaderAlignment;

        AmVoidPtr AllocWithHeader(AmSize size, uint64 alignment)
        {
            const AmSize totalSize = kHeaderSize + size;
            const uint64 actualAlignment = alignment > kHeaderAlignment ? alignment : kHeaderAlignment;
            auto* raw = static_cast<uint8*>(Platform::Allocate(static_cast<uint64>(totalSize), actualAlignment));
            if (raw == nullptr)
                return nullptr;
            auto* header = reinterpret_cast<AllocHeader*>(raw);
            header->size = size;
            return raw + kHeaderSize;
        }

        AllocHeader* GetHeader(AmVoidPtr address)
        {
            return reinterpret_cast<AllocHeader*>(static_cast<uint8*>(address) - kHeaderSize);
        }

        void FreeWithHeader(AmVoidPtr address)
        {
            if (address == nullptr)
                return;
            Platform::Free(static_cast<uint8*>(address) - kHeaderSize);
        }
    } // anonymous namespace

    AmVoidPtr FlaxMemoryAllocator::Malloc(eMemoryPoolKind /*pool*/, AmSize size)
    {
        return AllocWithHeader(size, 1u);
    }

    AmVoidPtr FlaxMemoryAllocator::Realloc(eMemoryPoolKind /*pool*/, AmVoidPtr address, AmSize size)
    {
        AmVoidPtr newBlock = AllocWithHeader(size, 1u);
        if (newBlock != nullptr && address != nullptr)
        {
            const AmSize oldSize = GetHeader(address)->size;
            ::memcpy(newBlock, address, oldSize < size ? oldSize : size);
            FreeWithHeader(address);
        }
        return newBlock;
    }

    AmVoidPtr FlaxMemoryAllocator::Malign(eMemoryPoolKind /*pool*/, AmSize size, AmUInt32 alignment)
    {
        return AllocWithHeader(size, static_cast<uint64>(alignment));
    }

    AmVoidPtr FlaxMemoryAllocator::Realign(
        eMemoryPoolKind /*pool*/, AmVoidPtr address, AmSize size, AmUInt32 alignment)
    {
        AmVoidPtr newBlock = AllocWithHeader(size, static_cast<uint64>(alignment));
        if (newBlock != nullptr && address != nullptr)
        {
            const AmSize oldSize = GetHeader(address)->size;
            ::memcpy(newBlock, address, oldSize < size ? oldSize : size);
            FreeWithHeader(address);
        }
        return newBlock;
    }

    void FlaxMemoryAllocator::Free(eMemoryPoolKind /*pool*/, AmVoidPtr address)
    {
        FreeWithHeader(address);
    }

    AmSize FlaxMemoryAllocator::SizeOf(eMemoryPoolKind /*pool*/, AmVoidPtr address)
    {
        if (address == nullptr)
            return 0;
        return GetHeader(address)->size;
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

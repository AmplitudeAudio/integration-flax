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

// ============================================================================
//  AmplitudeHelpers — Core helper utilities for the Amplitude Audio plugin
//  (implementation)
// ============================================================================

#include "AmplitudeHelpers.h"

#include <Engine/Core/Log.h>
#include <Engine/Platform/Platform.h>
#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>
#include <cstring> // ::memcpy

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
        constexpr AmSize kHeaderSize      = ((sizeof(AllocHeader) + kHeaderAlignment - 1) / kHeaderAlignment) * kHeaderAlignment;

        AmVoidPtr AllocWithHeader(AmSize size, uint64 alignment)
        {
            const AmSize totalSize       = kHeaderSize + size;
            const uint64 actualAlignment = alignment > kHeaderAlignment ? alignment : kHeaderAlignment;
            auto*        raw             = static_cast<uint8*>(Platform::Allocate(static_cast<uint64>(totalSize), actualAlignment));
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
        static const CartesianCoordinateSystem flax(
            CartesianCoordinateSystem::Axis::PositiveX,
            CartesianCoordinateSystem::Axis::PositiveZ,
            CartesianCoordinateSystem::Axis::PositiveY);

        const AmVector3 result = {vec.X / 100.0f, vec.Y / 100.0f, vec.Z / 100.0f};
        return CartesianCoordinateSystem::ConvertToDefault(result, flax);
    }

    void FlaxTransformToAmplitude(
        const AudioSystemTransform& transform,
        AmVector3&                  outLocation,
        Orientation&                outOrientation)
    {
        outLocation = FlaxVec3ToAmVec3(transform.Position);

        const AmVector3 forward = FlaxVec3ToAmVec3(transform.Forward);
        const AmVector3 up      = FlaxVec3ToAmVec3(transform.Up);

        outOrientation = Orientation(forward, up);
    }

} // namespace AmplitudeAudio

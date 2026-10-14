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

#include "FlaxMemoryAllocator.h"

#include <Engine/Platform/Platform.h>

namespace AmplitudeAudio
{
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
    }

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
} // namespace AmplitudeAudio
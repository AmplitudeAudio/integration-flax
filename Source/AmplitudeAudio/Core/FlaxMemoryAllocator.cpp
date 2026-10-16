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

#include <cstdint>

#include <Engine/Platform/Platform.h>

namespace AmplitudeAudio
{
    namespace
    {
        struct AllocHeader
        {
            AmSize size;
            void*  raw;
        };

        constexpr uint64 kMinAlignment = 16;

        // The header sits directly below the aligned user pointer, so the block reserves room for both the header and the alignment padding.
        AmVoidPtr AllocWithHeader(AmSize size, uint64 alignment)
        {
            const uint64 actualAlignment = alignment > kMinAlignment ? alignment : kMinAlignment;
            auto*        raw             = static_cast<uint8*>(Platform::Allocate(static_cast<uint64>(size + actualAlignment + sizeof(AllocHeader)), kMinAlignment));
            if (raw == nullptr)
                return nullptr;

            const uintptr_t base = reinterpret_cast<uintptr_t>(raw) + sizeof(AllocHeader);
            const uintptr_t user = (base + actualAlignment - 1) & ~static_cast<uintptr_t>(actualAlignment - 1);

            auto* header = reinterpret_cast<AllocHeader*>(user - sizeof(AllocHeader));
            header->size = size;
            header->raw  = raw;
            return reinterpret_cast<AmVoidPtr>(user);
        }

        AllocHeader* GetHeader(AmVoidPtr address)
        {
            return reinterpret_cast<AllocHeader*>(static_cast<uint8*>(address) - sizeof(AllocHeader));
        }

        void FreeWithHeader(AmVoidPtr address)
        {
            if (address == nullptr)
                return;

            Platform::Free(GetHeader(address)->raw);
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
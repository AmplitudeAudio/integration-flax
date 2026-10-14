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

// clang-format off
#include "Cpp20Compat.h"
// clang-format on

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>
    /// Amplitude MemoryAllocator implementation backed by Flax Platform APIs.
    /// </summary>
    /// <remarks>
    /// <para>
    /// All Amplitude memory-pool kinds are routed to the same underlying
    /// Flax allocator; the pool parameter is ignored for routing but is
    /// available for future diagnostic use.
    /// </para>
    /// <para>
    /// Every allocation is prefixed with a size header so that Realloc and
    /// Realign can safely copy only the original number of bytes when growing.
    /// The header is aligned to at least 16 bytes so that the user pointer
    /// retains the requested alignment.
    /// </para>
    /// </remarks>
    class FlaxMemoryAllocator final : public MemoryAllocator
    {
    public:
        /// <summary>
        /// Allocates <paramref name="size"/> bytes from the given Amplitude pool.
        /// </summary>
        AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size) override;

        /// <summary>
        /// Reallocates an existing allocation to the new <paramref name="size"/>.
        /// </summary>
        AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size) override;

        /// <summary>
        /// Allocates <paramref name="size"/> bytes with the requested <paramref name="alignment"/>.
        /// </summary>
        AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment) override;

        /// <summary>
        /// Reallocates an existing aligned allocation to the new <paramref name="size"/> and <paramref name="alignment"/>.
        /// </summary>
        AmVoidPtr Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) override;

        /// <summary>
        /// Frees a previously allocated block.
        /// </summary>
        void Free(eMemoryPoolKind pool, AmVoidPtr address) override;

        /// <summary>
        /// Returns the usable size of the block at <paramref name="address"/>.
        /// </summary>
        /// <remarks>
        /// Flax's Platform API does not expose a size-query, so this
        /// implementation always returns 0. Amplitude only uses SizeOf
        /// for diagnostic/stats purposes, so returning 0 is safe.
        /// </remarks>
        AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) override;
    };
}
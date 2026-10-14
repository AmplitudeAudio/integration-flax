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

#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Delegate.h>
#include <Engine/Core/Types/String.h>

namespace AmplitudeAudio
{
    /// <summary>Reference-counts sound banks by file name and tracks when their sound files finish loading.</summary>
    /// <remarks>Engine calls go through callbacks so the counting logic can be tested without the SDK.</remarks>
    class AMPLITUDEAUDIO_API BankRegistry
    {
    public:
        using BankId = uint64;

        /// <summary>Engine operations the registry drives.</summary>
        struct Callbacks
        {
            Function<bool(const String&, BankId&)> Load;
            Function<void(BankId)> Unload;
            Function<void()> StartLoadFiles;
            Function<bool()> TryFinalizeLoadFiles;
        };

        explicit BankRegistry(const Callbacks& callbacks);

        /// <summary>Loads the bank on first use, otherwise adds a reference. Returns false if the load failed.</summary>
        bool Acquire(const String& file);

        /// <summary>Drops a reference and unloads the bank when none remain.</summary>
        void Release(const String& file);

        /// <summary>Marks banks loaded once the engine has finished loading their sound files.</summary>
        void Update();

        /// <summary>Blocks until no load is pending. Returns false on timeout.</summary>
        bool WaitForPending(int32 timeoutMs);

        /// <summary>Unloads every bank regardless of references.</summary>
        void ReleaseAll();

        /// <summary>Whether the bank and its sound files are loaded.</summary>
        [[nodiscard]] bool IsLoaded(const String& file) const;

        /// <summary>Number of references held on the bank; 0 when unknown.</summary>
        [[nodiscard]] int32 RefCount(const String& file) const;

        /// <summary>Whether a bank's sound files are still loading.</summary>
        [[nodiscard]] bool HasPending() const;

    private:
        struct Entry
        {
            BankId Id = 0;
            int32 RefCount = 0;
            bool Loaded = false;
        };

        Callbacks m_callbacks;
        Dictionary<String, Entry> m_entries;
        bool m_pending = false;
    };
} // namespace AmplitudeAudio

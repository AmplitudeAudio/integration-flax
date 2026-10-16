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

#include "BankRegistry.h"

#include <Engine/Platform/Platform.h>

namespace AmplitudeAudio
{
    BankRegistry::BankRegistry(const Callbacks& callbacks)
        : m_callbacks(callbacks)
    {
    }

    bool BankRegistry::Acquire(const String& file)
    {
        if (Entry* entry = m_entries.TryGet(file))
        {
            ++entry->RefCount;
            return true;
        }

        BankId id = 0;
        if (!m_callbacks.Load(file, id))
            return false;

        Entry entry;
        entry.Id       = id;
        entry.RefCount = 1;
        m_entries[file] = entry;

        m_callbacks.StartLoadFiles();
        m_pending = true;
        return true;
    }

    void BankRegistry::Release(const String& file)
    {
        Entry* entry = m_entries.TryGet(file);
        if (entry == nullptr)
            return;

        if (--entry->RefCount > 0)
            return;

        const BankId id = entry->Id;
        m_entries.Remove(file);
        if (m_pending)
        {
            m_pending = false;
            for (const auto& other : m_entries)
            {
                if (!other.Value.Loaded)
                {
                    m_pending = true;
                    break;
                }
            }
        }

        m_callbacks.Unload(id);
    }

    void BankRegistry::Update()
    {
        if (!m_pending || !m_callbacks.TryFinalizeLoadFiles())
            return;

        // The engine finalizes all pending sound files at once, not per bank.
        for (auto& entry : m_entries)
            entry.Value.Loaded = true;

        m_pending = false;
    }

    bool BankRegistry::WaitForPending(int32 timeoutMs)
    {
        const double deadline = Platform::GetTimeSeconds() + timeoutMs / 1000.0;
        while (true)
        {
            Update();
            if (!m_pending)
                return true;

            if (Platform::GetTimeSeconds() >= deadline)
                return false;

            Platform::Sleep(1);
        }
    }

    void BankRegistry::ReleaseAll()
    {
        for (const auto& entry : m_entries)
            m_callbacks.Unload(entry.Value.Id);

        m_entries.Clear();
        m_pending = false;
    }

    bool BankRegistry::IsLoaded(const String& file) const
    {
        const Entry* entry = m_entries.TryGet(file);
        return entry != nullptr && entry->Loaded;
    }

    int32 BankRegistry::RefCount(const String& file) const
    {
        const Entry* entry = m_entries.TryGet(file);
        return entry == nullptr ? 0 : entry->RefCount;
    }

    bool BankRegistry::HasPending() const
    {
        return m_pending;
    }
} // namespace AmplitudeAudio

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
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>
    /// Amplitude FileSystem implementation backed by Flax platform APIs.
    /// </summary>
    /// <remarks>
    /// This implementation is synchronous: StartOpenFileSystem() and
    /// StartCloseFileSystem() complete immediately, and the corresponding
    /// TryFinalize* methods always return true.
    /// </remarks>
    class FlaxFileSystem final : public SparkyStudios::Audio::Amplitude::FileSystem
    {
    public:
        FlaxFileSystem() = default;

        void SetBasePath(const AmOsString& basePath) override;
        [[nodiscard]] const AmOsString& GetBasePath() const override;
        [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override;
        [[nodiscard]] bool Exists(const AmOsString& path) const override;
        [[nodiscard]] bool IsDirectory(const AmOsString& path) const override;
        [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override;
        [[nodiscard]] std::shared_ptr<SparkyStudios::Audio::Amplitude::File> OpenFile(const AmOsString& path, eFileOpenMode mode) const override;
        void StartOpenFileSystem() override;
        bool TryFinalizeOpenFileSystem() override;
        void StartCloseFileSystem() override;
        bool TryFinalizeCloseFileSystem() override;

    private:
        AmOsString m_basePath;
    };
} // namespace AmplitudeAudio
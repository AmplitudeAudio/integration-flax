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

// ============================================================================
//  AmplitudeFileSystem — Amplitude FileSystem interface backed by Flax
//
//  Implements SparkyStudios::Audio::Amplitude::FileSystem using Flax's
//  platform file-system utilities (FileSystem::FileExists, DirectoryExists,
//  etc.) and Flax's content directory as the root.
//
//  Path convention:
//    - The base path is set via SetBasePath() and is interpreted as an
//      absolute OS path to the content root (e.g. the project's Amplitude
//      assets directory).
//    - ResolvePath() concatenates the base path with relative paths using the
//      platform path separator.
// ============================================================================

#include "../Core/Cpp20Compat.h"

#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>Amplitude FileSystem implementation backed by Flax platform APIs.</summary>
    /// <remarks>
    /// This implementation is synchronous: StartOpenFileSystem() and
    /// StartCloseFileSystem() complete immediately, and the corresponding
    /// TryFinalize* methods always return true.
    /// </remarks>
    class AmplitudeFileSystem final : public SparkyStudios::Audio::Amplitude::FileSystem
    {
    public:
        AmplitudeFileSystem() = default;

        // ====================================================================
        //  FileSystem interface
        // ====================================================================

        /// <summary>Stores <paramref name="basePath"/> as the root from which relative paths are resolved.</summary>
        void SetBasePath(const AmOsString& basePath) override;

        /// <summary>Returns the previously set base path.</summary>
        [[nodiscard]] const AmOsString& GetBasePath() const override;

        /// <summary>Resolves <paramref name="path"/> relative to the base path.</summary>
        /// <remarks>If <paramref name="path"/> is already absolute it is returned unchanged.</remarks>
        [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override;

        /// <summary>Returns true if the item at <paramref name="path"/> exists (file or directory).</summary>
        [[nodiscard]] bool Exists(const AmOsString& path) const override;

        /// <summary>Returns true if <paramref name="path"/> refers to a directory.</summary>
        [[nodiscard]] bool IsDirectory(const AmOsString& path) const override;

        /// <summary>Joins <paramref name="parts"/> using the platform path separator.</summary>
        [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override;

        /// <summary>Opens the file at <paramref name="path"/> with the given <paramref name="mode"/>.</summary>
        /// <returns>A shared AmplitudeFile, or an invalid instance on error.</returns>
        [[nodiscard]] std::shared_ptr<SparkyStudios::Audio::Amplitude::File> OpenFile(const AmOsString& path, eFileOpenMode mode) const override;

        /// <summary>Marks the file system as open (no-op: Flax has no async mount step).</summary>
        void StartOpenFileSystem() override;

        /// <summary>Always returns true; opening is synchronous.</summary>
        bool TryFinalizeOpenFileSystem() override;

        /// <summary>Marks the file system as closed (no-op: Flax needs no unmount step).</summary>
        void StartCloseFileSystem() override;

        /// <summary>Always returns true; closing is synchronous.</summary>
        bool TryFinalizeCloseFileSystem() override;

    private:
        AmOsString m_basePath;
    };

} // namespace AmplitudeAudio

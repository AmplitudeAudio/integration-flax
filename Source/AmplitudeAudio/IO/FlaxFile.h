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

#include <Engine/Platform/File.h>

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>
    /// Amplitude File implementation backed by Flax platform file I/O.
    /// </summary>
    /// <remarks>
    /// The constructor opens the file immediately using Flax's File::Open().
    /// IsValid() returns false if the open failed or the file was already closed.
    /// </remarks>
    class FlaxFile final : public SparkyStudios::Audio::Amplitude::File
    {
    public:
        /// <summary>Opens the file at <paramref name="path"/> with the given Amplitude open <paramref name="mode"/>.</summary>
        /// <param name="path">Amplitude OS string path to the file on disk.</param>
        /// <param name="mode">Amplitude file open mode (read, write, append, etc.).</param>
        FlaxFile(const AmOsString& path, eFileOpenMode mode);

        /// <summary>Closes the underlying file handle on destruction.</summary>
        ~FlaxFile() override;

        [[nodiscard]] AmOsString GetPath() const override;
        [[nodiscard]] bool Eof() const override;
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) const override;
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;
        [[nodiscard]] AmSize Length() const override;
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;
        [[nodiscard]] AmSize Position() const override;
        [[nodiscard]] AmVoidPtr GetPtr() const override;
        [[nodiscard]] bool IsValid() const override;
        void Close() override;

    private:
        AmOsString  m_path;
        ::FileBase* m_handle;
    };
} // namespace AmplitudeAudio
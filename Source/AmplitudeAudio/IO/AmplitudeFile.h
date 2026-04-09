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
//  AmplitudeFile — Amplitude File interface backed by Flax platform file I/O
//
//  Wraps Flax's platform-specific ::File handle to satisfy the Amplitude
//  SDK's SparkyStudios::Audio::Amplitude::File interface.  All positioning,
//  reading, writing, and size queries are forwarded to the underlying
//  ::FileBase-derived handle obtained via ::File::Open().
// ============================================================================

#include <Engine/Platform/File.h>
#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>Amplitude File implementation backed by Flax platform file I/O.</summary>
    /// <remarks>
    /// The constructor opens the file immediately using Flax's ::File::Open().
    /// IsValid() returns false if the open failed or the file was already closed.
    /// </remarks>
    class AmplitudeFile final : public SparkyStudios::Audio::Amplitude::File
    {
      public:
        /// <summary>Opens the file at <paramref name="path"/> with the given Amplitude open <paramref name="mode"/>.</summary>
        /// <param name="path">Amplitude OS string path to the file on disk.</param>
        /// <param name="mode">Amplitude file open mode (read, write, append, etc.).</param>
        AmplitudeFile(const AmOsString& path, eFileOpenMode mode);

        /// <summary>Closes the underlying file handle on destruction.</summary>
        ~AmplitudeFile() override;

        // ====================================================================
        //  File interface
        // ====================================================================

        /// <summary>Returns the path this file was opened with.</summary>
        [[nodiscard]] AmOsString GetPath() const override;

        /// <summary>Returns true when the read/write position has reached the end of file.</summary>
        [[nodiscard]] bool Eof() const override;

        /// <summary>Reads up to <paramref name="bytes"/> from the current position into <paramref name="dst"/>.</summary>
        /// <returns>Number of bytes actually read.</returns>
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) const override;

        /// <summary>Writes <paramref name="bytes"/> from <paramref name="src"/> at the current position.</summary>
        /// <returns>Number of bytes actually written.</returns>
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /// <summary>Returns the total size of the file in bytes.</summary>
        [[nodiscard]] AmSize Length() const override;

        /// <summary>Seeks to the given <paramref name="offset"/> relative to <paramref name="origin"/>.</summary>
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;

        /// <summary>Returns the current read/write cursor position.</summary>
        [[nodiscard]] AmSize Position() const override;

        /// <summary>Returns the raw ::FileBase pointer, or nullptr if not open.</summary>
        [[nodiscard]] AmVoidPtr GetPtr() const override;

        /// <summary>Returns true if the file handle is valid and open.</summary>
        [[nodiscard]] bool IsValid() const override;

        /// <summary>Closes the file and releases the handle.</summary>
        void Close() override;

      private:
        AmOsString  m_path;
        ::FileBase* m_handle;
    };

} // namespace AmplitudeAudio

#pragma once

// ============================================================================
//  AmplitudeFile — Amplitude File interface backed by Flax platform file I/O
//
//  Wraps Flax's platform-specific ::File handle to satisfy the Amplitude
//  SDK's SparkyStudios::Audio::Amplitude::File interface.  All positioning,
//  reading, writing, and size queries are forwarded to the underlying
//  ::FileBase-derived handle obtained via ::File::Open().
// ============================================================================

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

#include <Engine/Platform/File.h>

using namespace SparkyStudios::Audio::Amplitude;

namespace AmplitudeAudio
{
    /// \brief Amplitude File implementation backed by Flax platform file I/O.
    ///
    /// The constructor opens the file immediately using Flax's ::File::Open().
    /// IsValid() returns false if the open failed or the file was already closed.
    class AmplitudeFile final : public File
    {
    public:
        /// \brief Opens the file at \p path with the given Amplitude open \p mode.
        ///
        /// \param path  Amplitude OS string path to the file on disk.
        /// \param mode  Amplitude file open mode (read, write, append, etc.).
        AmplitudeFile(const AmOsString& path, eFileOpenMode mode);

        /// \brief Closes the underlying file handle on destruction.
        ~AmplitudeFile() override;

        // ====================================================================
        //  File interface
        // ====================================================================

        /// Returns the path this file was opened with.
        [[nodiscard]] AmOsString GetPath() const override;

        /// Returns true when the read/write position has reached the end of file.
        [[nodiscard]] bool Eof() const override;

        /// Reads up to \p bytes from the current position into \p dst.
        /// \return Number of bytes actually read.
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) const override;

        /// Writes \p bytes from \p src at the current position.
        /// \return Number of bytes actually written.
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /// Returns the total size of the file in bytes.
        [[nodiscard]] AmSize Length() const override;

        /// Seeks to the given \p offset relative to \p origin.
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;

        /// Returns the current read/write cursor position.
        [[nodiscard]] AmSize Position() const override;

        /// Returns the raw ::FileBase pointer, or nullptr if not open.
        [[nodiscard]] AmVoidPtr GetPtr() const override;

        /// Returns true if the file handle is valid and open.
        [[nodiscard]] bool IsValid() const override;

        /// Closes the file and releases the handle.
        void Close() override;

    private:
        AmOsString  m_path;
        ::FileBase* m_handle;
    };

} // namespace AmplitudeAudio

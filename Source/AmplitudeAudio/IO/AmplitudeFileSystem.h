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

#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

using namespace SparkyStudios::Audio::Amplitude;

namespace AmplitudeAudio
{
    /// \brief Amplitude FileSystem implementation backed by Flax platform APIs.
    ///
    /// This implementation is synchronous: StartOpenFileSystem() and
    /// StartCloseFileSystem() complete immediately, and the corresponding
    /// TryFinalize* methods always return true.
    class AmplitudeFileSystem final : public FileSystem
    {
    public:
        AmplitudeFileSystem() = default;

        // ====================================================================
        //  FileSystem interface
        // ====================================================================

        /// Stores \p basePath as the root from which relative paths are resolved.
        void SetBasePath(const AmOsString& basePath) override;

        /// Returns the previously set base path.
        [[nodiscard]] const AmOsString& GetBasePath() const override;

        /// Resolves \p path relative to the base path.
        ///
        /// If \p path is already absolute it is returned unchanged.
        [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override;

        /// Returns true if the item at \p path exists (file or directory).
        [[nodiscard]] bool Exists(const AmOsString& path) const override;

        /// Returns true if \p path refers to a directory.
        [[nodiscard]] bool IsDirectory(const AmOsString& path) const override;

        /// Joins \p parts using the platform path separator.
        [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override;

        /// Opens the file at \p path with the given \p mode.
        /// \return A shared AmplitudeFile, or an invalid instance on error.
        [[nodiscard]] std::shared_ptr<File> OpenFile(const AmOsString& path, eFileOpenMode mode) const override;

        /// Marks the file system as open (no-op: Flax has no async mount step).
        void StartOpenFileSystem() override;

        /// Always returns true; opening is synchronous.
        bool TryFinalizeOpenFileSystem() override;

        /// Marks the file system as closed (no-op: Flax needs no unmount step).
        void StartCloseFileSystem() override;

        /// Always returns true; closing is synchronous.
        bool TryFinalizeCloseFileSystem() override;

    private:
        AmOsString m_basePath;
    };

} // namespace AmplitudeAudio

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

// ============================================================================
//  AmplitudeFileSystem — implementation
// ============================================================================

#include "AmplitudeFileSystem.h"

#include <Engine/Core/Types/String.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Platform/FileSystem.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include "AmplitudeFile.h"

namespace AmplitudeAudio
{
    // ------------------------------------------------------------------------
    //  Internal helpers — convert AmOsString to Flax String
    // ------------------------------------------------------------------------

    /// <summary>Converts an AmOsString to a Flax String for use with Flax platform APIs.</summary>
    /// <remarks>
    /// Flax's Char type can be char16_t (Unix) or wchar_t (Windows), so we
    /// cannot safely pass a wchar_t* to the Flax String constructor on all
    /// platforms.  We always route through StringAnsi: on wchar platforms the
    /// wide AmOsString (std::wstring) is narrowed first via the Amplitude SDK
    /// helper macro, then wrapped in StringAnsi for the Flax String constructor.
    /// </remarks>
    static ::String AmOsStringToFlaxString(const AmOsString& path)
    {
#if defined(AM_WCHAR_SUPPORTED)
        const std::string narrow = am_wstring_narrow(path);
        return ::String(::StringAnsi(narrow.c_str()));
#else
        return ::String(::StringAnsi(path.c_str()));
#endif
    }

    // ------------------------------------------------------------------------
    //  Helpers for path separator handling (char-type-agnostic)
    // ------------------------------------------------------------------------

    static bool IsPathSeparator(AmOsChar c)
    {
        return c == AM_OS_STRING('/') || c == AM_OS_STRING('\\');
    }

    static AmOsChar PathSeparator()
    {
        return AM_OS_STRING('/');
    }

    // ------------------------------------------------------------------------
    //  FileSystem interface implementation
    // ------------------------------------------------------------------------

    void AmplitudeFileSystem::SetBasePath(const AmOsString& basePath)
    {
        m_basePath = basePath;
    }

    const AmOsString& AmplitudeFileSystem::GetBasePath() const
    {
        return m_basePath;
    }

    AmOsString AmplitudeFileSystem::ResolvePath(const AmOsString& path) const
    {
        if (path.empty())
            return m_basePath;

        // Treat the path as absolute when it begins with a path separator or
        // contains a Windows-style drive letter (e.g. "C:").
        const AmOsChar firstChar = path.front();
        const bool     isAbsolute =
            IsPathSeparator(firstChar) ||
            (path.size() >= 2 && path[1] == AM_OS_STRING(':'));

        if (isAbsolute)
            return path;

        return Join({m_basePath, path});
    }

    bool AmplitudeFileSystem::Exists(const AmOsString& path) const
    {
        const ::String flaxPath = AmOsStringToFlaxString(ResolvePath(path));
        return ::FileSystem::FileExists(flaxPath) ||
               ::FileSystem::DirectoryExists(flaxPath);
    }

    bool AmplitudeFileSystem::IsDirectory(const AmOsString& path) const
    {
        const ::String flaxPath = AmOsStringToFlaxString(ResolvePath(path));
        return ::FileSystem::DirectoryExists(flaxPath);
    }

    AmOsString AmplitudeFileSystem::Join(const std::vector<AmOsString>& parts) const
    {
        if (parts.empty())
            return AmOsString{};

        const AmOsChar sep    = PathSeparator();
        AmOsString     result = parts[0];

        for (std::size_t i = 1; i < parts.size(); ++i)
        {
            const AmOsString& part = parts[i];
            if (part.empty())
                continue;

            const bool resultEnds = !result.empty() && IsPathSeparator(result.back());
            const bool partStarts = IsPathSeparator(part.front());

            if (!resultEnds && !partStarts)
            {
                // Neither segment has a boundary separator — add one.
                result += sep;
            }
            else if (resultEnds && partStarts)
            {
                // Both sides have a separator — remove the trailing one from
                // result to prevent a double separator.
                result.pop_back();
            }

            result += part;
        }

        return result;
    }

    std::shared_ptr<SparkyStudios::Audio::Amplitude::File> AmplitudeFileSystem::OpenFile(const AmOsString& path, eFileOpenMode mode) const
    {
        return ampoolshared(eMemoryPoolKind_IO, AmplitudeFile, ResolvePath(path), mode);
    }

    void AmplitudeFileSystem::StartOpenFileSystem()
    {
        // Flax's file system is always available; no initialisation required.
    }

    bool AmplitudeFileSystem::TryFinalizeOpenFileSystem()
    {
        return true;
    }

    void AmplitudeFileSystem::StartCloseFileSystem()
    {
        // Flax's file system requires no explicit teardown.
    }

    bool AmplitudeFileSystem::TryFinalizeCloseFileSystem()
    {
        return true;
    }
} // namespace AmplitudeAudio

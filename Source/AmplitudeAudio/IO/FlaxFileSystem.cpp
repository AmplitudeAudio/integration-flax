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

#include <AmplitudeAudio/IO/FlaxFileSystem.h>

#include <Engine/Core/Types/String.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Platform/FileSystem.h>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/IO/FlaxFile.h>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

namespace AmplitudeAudio
{
    static bool IsPathSeparator(AmOsChar c)
    {
        return c == AM_OS_STRING('/') || c == AM_OS_STRING('\\');
    }

    static AmOsChar PathSeparator()
    {
        return AM_OS_STRING('/');
    }

    void FlaxFileSystem::SetBasePath(const AmOsString& basePath)
    {
        m_basePath = basePath;
    }

    const AmOsString& FlaxFileSystem::GetBasePath() const
    {
        return m_basePath;
    }

    AmOsString FlaxFileSystem::ResolvePath(const AmOsString& path) const
    {
        if (path.empty())
            return m_basePath;

        // Treat the path as absolute when it begins with a path separator or
        // contains a Windows-style drive letter (e.g. "C:").
        const AmOsChar firstChar  = path.front();
        const bool     isAbsolute =
            IsPathSeparator(firstChar) ||
            (path.size() >= 2 && path[1] == AM_OS_STRING(':'));

        if (isAbsolute)
            return path;

        return Join({m_basePath, path});
    }

    bool FlaxFileSystem::Exists(const AmOsString& path) const
    {
        const ::String flaxPath = AmOsStringToFlaxString(ResolvePath(path));
        return ::FileSystem::FileExists(flaxPath) ||
               ::FileSystem::DirectoryExists(flaxPath);
    }

    bool FlaxFileSystem::IsDirectory(const AmOsString& path) const
    {
        const ::String flaxPath = AmOsStringToFlaxString(ResolvePath(path));
        return ::FileSystem::DirectoryExists(flaxPath);
    }

    AmOsString FlaxFileSystem::Join(const std::vector<AmOsString>& parts) const
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

    std::shared_ptr<SparkyStudios::Audio::Amplitude::File> FlaxFileSystem::OpenFile(const AmOsString& path, eFileOpenMode mode) const
    {
        return ampoolshared(eMemoryPoolKind_IO, FlaxFile, ResolvePath(path), mode);
    }

    void FlaxFileSystem::StartOpenFileSystem()
    {
        // Flax's file system is always available; no initialization required.
    }

    bool FlaxFileSystem::TryFinalizeOpenFileSystem()
    {
        return true;
    }

    void FlaxFileSystem::StartCloseFileSystem()
    {
        // Flax's file system requires no explicit teardown.
    }

    bool FlaxFileSystem::TryFinalizeCloseFileSystem()
    {
        return true;
    }
} // namespace AmplitudeAudio
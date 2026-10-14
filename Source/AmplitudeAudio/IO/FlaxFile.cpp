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

#include <AmplitudeAudio/IO/FlaxFile.h>

#include <AmplitudeAudio/Core/Common.h>

#include <Engine/Core/Types/String.h>
#include <Engine/Core/Types/StringView.h>

namespace AmplitudeAudio
{
    /// <summary>
    /// Converts an AmOsString to a Flax String suitable for platform file APIs.
    // </summary>
    /// <remarks>
    /// Flax's Char type may be char16_t (Unix) or wchar_t (Windows), so we
    /// cannot pass a wchar_t* directly on all platforms.  We always route
    /// through StringAnsi to avoid the mismatch: on wchar platforms we first
    /// narrow the wide string using the SDK helper, then use StringAnsi.
    /// </remarks>
    static ::String AmOsStringToFlaxString(const AmOsString& path)
    {
#ifdef AM_WCHAR_SUPPORTED
        // AmOsString is std::wstring.
        const std::string narrow = am_wstring_narrow(path);
        return ::String(::StringAnsi(narrow.c_str()));
#else
        // AmOsString is a narrow std::string.
        return ::String(::StringAnsi(path.c_str()));
#endif
    }

    /// <summary>
    /// Maps a high-level file open mode to the underlying platform-specific file mode and access settings.
    /// </summary>
    /// <param name="mode">The file open mode specified as an enum of type eFileOpenMode.</param>
    /// <param name="outFileMode">A reference to the resulting file mode of type FileMode.</param>
    /// <param name="outAccess">A reference to the resulting file access of type FileAccess.</param>
    static void MapOpenMode(eFileOpenMode mode, FileMode& outFileMode, FileAccess& outAccess)
    {
        switch (mode)
        {
            case eFileOpenMode_Write:
                outFileMode = FileMode::CreateAlways;
                outAccess = FileAccess::Write;
                break;

            case eFileOpenMode_Append:
                outFileMode = FileMode::OpenAlways;
                outAccess = FileAccess::Write;
                break;

            case eFileOpenMode_ReadWrite:
                outFileMode = FileMode::CreateAlways;
                outAccess = FileAccess::ReadWrite;
                break;

            case eFileOpenMode_ReadAppend:
                outFileMode = FileMode::OpenAlways;
                outAccess = FileAccess::ReadWrite;
                break;

            case eFileOpenMode_Read:
            default:
                outFileMode = FileMode::OpenExisting;
                outAccess = FileAccess::Read;
                break;
        }
    }

    FlaxFile::FlaxFile(const AmOsString& path, eFileOpenMode mode)
        : m_path(path), m_handle(nullptr)
    {
        FileMode   flaxMode   = FileMode::OpenExisting;
        FileAccess flaxAccess = FileAccess::Read;
        MapOpenMode(mode, flaxMode, flaxAccess);

        const ::String flaxPath = AmOsStringToFlaxString(path);
        m_handle                = ::File::Open(flaxPath, flaxMode, flaxAccess);
        // m_handle is nullptr when the open fails; IsValid() propagates this.
    }

    FlaxFile::~FlaxFile()
    {
        Close();
    }

    AmOsString FlaxFile::GetPath() const
    {
        return m_path;
    }

    bool FlaxFile::IsValid() const
    {
        return m_handle != nullptr && m_handle->IsOpened();
    }

    bool FlaxFile::Eof() const
    {
        if (m_handle == nullptr)
            return true;

        return m_handle->GetPosition() >= m_handle->GetSize();
    }

    AmSize FlaxFile::Read(AmUInt8Buffer dst, AmSize bytes) const
    {
        if (m_handle == nullptr || dst == nullptr || bytes == 0)
            return 0;

        uint32     bytesRead = 0;
        const bool error     = m_handle->Read(dst, static_cast<uint32>(bytes), &bytesRead);
        return error ? 0 : static_cast<AmSize>(bytesRead);
    }

    AmSize FlaxFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        if (m_handle == nullptr || src == nullptr || bytes == 0)
            return 0;

        uint32     bytesWritten = 0;
        // FileBase::Write takes a non-const void* for the buffer argument.
        const bool error        = m_handle->Write(src, static_cast<uint32>(bytes), &bytesWritten);
        return error ? 0 : static_cast<AmSize>(bytesWritten);
    }

    AmSize FlaxFile::Length() const
    {
        if (m_handle == nullptr)
            return 0;

        return static_cast<AmSize>(m_handle->GetSize());
    }

    void FlaxFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        if (m_handle == nullptr)
            return;

        uint32 targetPosition = 0;

        switch (origin)
        {
            case eFileSeekOrigin_Start:
                // Clamp negative offsets to zero; uint32 position cannot go negative.
                targetPosition = (offset >= 0) ? static_cast<uint32>(offset) : 0u;
                break;

            case eFileSeekOrigin_Current:
            {
                const int64 current = static_cast<int64>(m_handle->GetPosition());
                const int64 next    = current + offset;
                targetPosition      = (next >= 0) ? static_cast<uint32>(next) : 0u;
                break;
            }

            case eFileSeekOrigin_End:
            {
                const int64 end  = static_cast<int64>(m_handle->GetSize());
                const int64 next = end + offset;
                targetPosition   = (next >= 0) ? static_cast<uint32>(next) : 0u;
                break;
            }

            default:
                return;
        }

        m_handle->SetPosition(targetPosition);
    }

    AmSize FlaxFile::Position() const
    {
        if (m_handle == nullptr)
            return 0;

        return static_cast<AmSize>(m_handle->GetPosition());
    }

    AmVoidPtr FlaxFile::GetPtr() const
    {
        return static_cast<AmVoidPtr>(m_handle);
    }

    void FlaxFile::Close()
    {
        if (m_handle != nullptr)
        {
            m_handle->Close();
            Delete(m_handle);
            m_handle = nullptr;
        }
    }
} // namespace AmplitudeAudio
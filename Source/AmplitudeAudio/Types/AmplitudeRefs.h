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

#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/BaseTypes.h>
#include <Engine/Scripting/ScriptingType.h>

/// <summary>References a sound by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeSoundRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSoundRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References a collection by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeCollectionRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeCollectionRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References a switch container by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeSwitchContainerRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSwitchContainerRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References any sound object (sound, collection, or switch container) by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeSoundObjectRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSoundObjectRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References an event by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeEventRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeEventRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References an RTPC by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeRtpcRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeRtpcRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References a switch by its Amplitude ID.</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeSwitchRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSwitchRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

/// <summary>References a switch state by its Amplitude ID (unique within its switch).</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeSwitchStateRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSwitchStateRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Amplitude ID; 0 means no reference.</summary>
    API_FIELD()
    uint64 ID = 0;

    /// <summary>Whether an ID is set.</summary>
    bool IsValid() const
    {
        return ID != 0;
    }
};

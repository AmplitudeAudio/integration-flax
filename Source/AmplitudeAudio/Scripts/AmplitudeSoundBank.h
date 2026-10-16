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

#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/Script.h>

#include <AmplitudeAudio/Types/AmplitudeSoundBankRef.h>

/// <summary>Keeps a sound bank loaded while this script is enabled.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "FlaxEngine.Category(\"Amplitude\")")
class AMPLITUDEAUDIO_API AmplitudeSoundBank : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AmplitudeSoundBank);

public:
    /// <summary>The sound bank to keep loaded while this script is enabled.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    AmplitudeSoundBankRef Bank;

    /// <summary>Load the bank's sound files in the background instead of blocking the frame that enables this script.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    bool LoadAsync = false;

    /// <summary>Whether the bank and its sound files are loaded.</summary>
    API_PROPERTY()
    bool IsLoaded() const;

    void OnEnable() override;
    void OnDisable() override;

private:
    String _acquiredFile;
};

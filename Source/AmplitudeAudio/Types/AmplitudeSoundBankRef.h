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
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingType.h>

/// <summary>Reference to an Amplitude sound bank, by file name (e.g. <c>init.ambank</c>).</summary>
API_STRUCT(Namespace = "AmplitudeAudio")
struct AMPLITUDEAUDIO_API AmplitudeSoundBankRef : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AmplitudeSoundBankRef);
    API_AUTO_SERIALIZATION();

    /// <summary>Bank file name, as the engine resolves it from the project build.</summary>
    API_FIELD()
    String File;
};

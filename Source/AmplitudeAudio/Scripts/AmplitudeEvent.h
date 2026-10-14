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

#include <AmplitudeAudio/Core/Cpp20Compat.h>

#include <Engine/Core/Collections/Array.h>
#include <Engine/Scripting/Script.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>
#include <AmplitudeAudio/Types/AmplitudeEventHandle.h>

class AmplitudeEntity;

/// <summary>Triggers an Amplitude event on an entity. Attach to an actor with an AmplitudeEntity ancestor.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "FlaxEngine.Category(\"Amplitude\")")
class AMPLITUDEAUDIO_API AmplitudeEvent : public Script
{
    DECLARE_SCRIPTING_TYPE(AmplitudeEvent);

public:
    /// <summary>The event to trigger.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    AmplitudeEventRef Event;

    /// <summary>Optional explicit entity; if null, finds the nearest AmplitudeEntity ancestor.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    ScriptingObjectReference<AmplitudeEntity> EntityOverride;

    /// <summary>Trigger the event when the script starts.</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    bool TriggerOnStart = false;

    /// <summary>Trigger the event when the script is enabled.</summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    bool TriggerOnEnable = false;

    /// <summary>Cancel all triggered events when the script is disabled.</summary>
    API_FIELD(Attributes = "EditorOrder(4)")
    bool CancelOnDisable = true;

    /// <summary>Triggers the event and returns the handle.</summary>
    API_FUNCTION()
    AmplitudeEventHandle* Trigger();

    /// <summary>Cancels all events triggered by this script.</summary>
    API_FUNCTION()
    void CancelAll();

    // Script
    void OnStart() override;
    void OnEnable() override;
    void OnDisable() override;
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    Array<ScriptingObjectReference<AmplitudeEventHandle>> _handles;
};

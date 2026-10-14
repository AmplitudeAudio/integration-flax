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

#include <Engine/Level/Actor.h>

#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>

/// <summary>An Amplitude listener. Usually a child of the camera.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "ActorContextMenu(\"New/Amplitude/Listener\"), ActorToolbox(\"Other\")")
class AMPLITUDEAUDIO_API AmplitudeListener : public Actor, public AmplitudeAudio::IAmplitudeServiceClient
{
    DECLARE_SCENE_OBJECT(AmplitudeListener);

public:
    /// <summary>Whether this listener is the engine's default listener. The last enabled default listener wins.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    bool IsDefault = true;

    /// <summary>Directivity, from 0 (omnidirectional) to 1 (cardioid).</summary>
    API_FIELD(Attributes = "EditorOrder(1), Limit(0, 1, 0.01f)")
    float Directivity = 1.0f;

    /// <summary>Sharpness of the directivity pattern.</summary>
    API_FIELD(Attributes = "EditorOrder(2), Limit(0, 10, 0.01f)")
    float Sharpness = 1.0f;

    /// <summary>Whether the SDK listener exists (the engine is running and this actor is enabled).</summary>
    API_PROPERTY()
    bool IsValid() const;

    /// <summary>The SDK listener ID, or 0.</summary>
    uint64 GetListenerId() const;

    void OnEngineStarted() override;
    void OnEngineStopping() override;

    // Actor
    void OnEnable() override;
    void OnDisable() override;
    void OnTransformChanged() override;
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    void PushState() const;

    SparkyStudios::Audio::Amplitude::Listener _listener;
    uint64 _id = 0;
    bool _registered = false;
};

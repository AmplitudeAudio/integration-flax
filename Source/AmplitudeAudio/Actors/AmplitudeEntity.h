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

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>

#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>
#include <AmplitudeAudio/Runtime/Occlusion/IOcclusionSource.h>

class IAmplitudeEmitter;

/// <summary>An Amplitude entity. Usually attached to a character or object that emits sound.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "ActorContextMenu(\"New/Amplitude/Entity\"), ActorToolbox(\"Other\")")
class AMPLITUDEAUDIO_API AmplitudeEntity : public Actor, public AmplitudeAudio::IAmplitudeServiceClient, public AmplitudeAudio::IOcclusionSource
{
    DECLARE_SCENE_OBJECT(AmplitudeEntity);

public:
    /// <summary>Directivity, from 0 (omnidirectional) to 1 (cardioid).</summary>
    API_FIELD(Attributes = "EditorOrder(0), Limit(0, 1, 0.01f)")
    float Directivity = 1.0f;

    /// <summary>Sharpness of the directivity pattern.</summary>
    API_FIELD(Attributes = "EditorOrder(1), Limit(0, 10, 0.01f)")
    float Sharpness = 1.0f;

    /// <summary>How physics obstruction is evaluated for sounds on this entity.</summary>
    API_FIELD(Attributes = "EditorOrder(10), EditorDisplay(\"Obstruction\")")
    AmplitudeObstructionMode ObstructionMode = AmplitudeObstructionMode::None;

    /// <summary>Physics layers checked for obstruction rays.</summary>
    API_FIELD(Attributes = "EditorOrder(11), EditorDisplay(\"Obstruction\")")
    LayersMask ObstructionLayers;

    /// <summary>Whether the SDK entity exists (the engine is running and this actor is enabled).</summary>
    API_PROPERTY()
    bool IsValid() const;

    /// <summary>Finds the nearest AmplitudeEntity on <paramref name="from"/> or its ancestors.</summary>
    API_FUNCTION()
    static AmplitudeEntity* FindNearest(Actor* from);

    /// <summary>Returns the SDK entity handle.</summary>
    const SparkyStudios::Audio::Amplitude::Entity& GetHandle() const;

    /// <summary>Finds an emitter by name on this entity or its descendants.</summary>
    IAmplitudeEmitter* GetEmitter(const ::StringView& name);

    /// <summary>Returns all emitters on this entity and its descendants.</summary>
    Array<IAmplitudeEmitter*> GetEmitters();

    void OnEngineStarted() override;
    void OnEngineStopping() override;

    // IOcclusionSource
    AmplitudeObstructionMode GetObstructionMode() const override;
    bool IsOcclusionActive() const override;
    Vector3 GetOcclusionLocation() const override;
    void ApplyOcclusion(float obstruction, float occlusion) override;

    // Actor
    void OnBeginPlay() override;
    void OnEndPlay() override;
    void OnEnable() override;
    void OnDisable() override;
    void OnTransformChanged() override;
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    void Register();
    void Unregister();
    void PushState() const;

    SparkyStudios::Audio::Amplitude::Entity _entity;
    bool _registered = false;
};

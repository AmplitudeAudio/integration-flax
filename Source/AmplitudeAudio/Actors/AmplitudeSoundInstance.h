// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <AmplitudeAudio/Core/Cpp20Compat.h>

#include <Engine/Level/Actor.h>

#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelInstance.h>

#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>
#include <AmplitudeAudio/Runtime/Occlusion/IOcclusionSource.h>
#include <AmplitudeAudio/Runtime/Spatial/ISpatialTarget.h>

/// <summary>A sound instance actor used for instanced sound emitters.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "ActorContextMenu(\"New/Amplitude/Sound Instance\"), ActorToolbox(\"Other\")")
class AMPLITUDEAUDIO_API AmplitudeSoundInstance : public Actor, public AmplitudeAudio::IAmplitudeServiceClient, public AmplitudeAudio::IOcclusionSource, public AmplitudeAudio::ISpatialTarget
{
    DECLARE_SCENE_OBJECT(AmplitudeSoundInstance);

public:
    /// <summary>Weight of this instance (0 to 1).</summary>
    API_FIELD(Attributes = "EditorOrder(0), Limit(0, 1, 0.01f)")
    float Weight = 1.0f;

    /// <summary>Whether this instance tracks actor orientation.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    bool TrackOrientation = false;

    /// <summary>Obstruction mode for this instance.</summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    AmplitudeObstructionMode ObstructionMode = AmplitudeObstructionMode::None;

    /// <summary>Layers checked for obstruction for this instance.</summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    LayersMask ObstructionLayers = LayersMask(~0);

    /// <summary>Whether this sound instance is currently bound to an active channel instance.</summary>
    API_PROPERTY()
    bool IsBound() const;

    /// <summary>Returns the bound channel instance ID, or 0 if not bound.</summary>
    API_PROPERTY()
    uint64 GetBoundId() const;

    /// <summary>Binds an SDK channel instance handle to this actor.</summary>
    void Bind(const SparkyStudios::Audio::Amplitude::ChannelInstance& handle);

    /// <summary>Unbinds the channel instance handle.</summary>
    void Unbind();

    const SparkyStudios::Audio::Amplitude::ChannelInstance& GetHandle() const { return _handle; }

    void OnEngineStarted() override;
    void OnEngineStopping() override;

    // IOcclusionSource
    AmplitudeObstructionMode GetObstructionMode() const override;
    bool IsOcclusionActive() const override;
    Vector3 GetOcclusionLocation() const override;
    const Actor* GetOcclusionActor() const override;
    uint32 GetOcclusionLayers() const override;
    void ApplyOcclusion(float obstruction, float occlusion) override;

    // ISpatialTarget
    Vector3 GetSpatialLocation() const override;
    void ApplyRoom(uint64 roomId) override;
    void ApplyEnvironmentFactor(uint64 environmentId, float factor) override;

    // Actor
    void OnEndPlay() override;
    void OnDeleteObject() override;
    void OnTransformChanged() override;
#if USE_EDITOR
    void OnDebugDrawSelected() override;
#endif
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    void PushState();

    SparkyStudios::Audio::Amplitude::ChannelInstance _handle;
    bool _registered = false;
};

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

#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>

#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>
#include <AmplitudeAudio/Runtime/Spatial/AmplitudeSpatialProcessor.h>
#include <AmplitudeAudio/Types/AmplitudeRefs.h>

/// <summary>Environment zone shape kind.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeEnvironmentShape
{
    Box = 0,
    Sphere = 1,
    Capsule = 2,
    Cone = 3,
};

/// <summary>An Amplitude environment actor defining a regional zone effect and blend distance.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "ActorContextMenu(\"New/Amplitude/Environment\"), ActorToolbox(\"Other\")")
class AMPLITUDEAUDIO_API AmplitudeEnvironment : public Actor, public AmplitudeAudio::IAmplitudeServiceClient
{
    DECLARE_SCENE_OBJECT(AmplitudeEnvironment);

public:
    /// <summary>Zone shape kind.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    AmplitudeEnvironmentShape Shape = AmplitudeEnvironmentShape::Box;

    /// <summary>Dimensions of the environment zone box in world units.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    Vector3 Size = Vector3(400.0f, 300.0f, 400.0f);

    /// <summary>Distance outside the zone over which the effect fades linearly to zero.</summary>
    API_FIELD(Attributes = "EditorOrder(2), Limit(0)")
    float BlendDistance = 100.0f;

    /// <summary>Environment effect applied within this zone.</summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    AmplitudeEnvironmentEffectRef Effect;

    /// <summary>Whether the SDK environment handle is valid.</summary>
    API_PROPERTY()
    bool IsValid() const;

    /// <summary>Returns the environment ID allocated for this environment.</summary>
    API_PROPERTY()
    uint64 GetEnvironmentId() const { return _id; }

    /// <summary>Returns the SDK environment handle.</summary>
    const SparkyStudios::Audio::Amplitude::Environment& GetHandle() const { return _environment; }

    API_PROPERTY(Attributes = "HideInEditor")
    bool IsBox() const { return Shape == AmplitudeEnvironmentShape::Box; }

    API_PROPERTY(Attributes = "HideInEditor")
    bool IsSphere() const { return Shape == AmplitudeEnvironmentShape::Sphere; }

    API_PROPERTY(Attributes = "HideInEditor")
    bool IsCapsule() const { return Shape == AmplitudeEnvironmentShape::Capsule; }

    API_PROPERTY(Attributes = "HideInEditor")
    bool IsCone() const { return Shape == AmplitudeEnvironmentShape::Cone; }

    API_PROPERTY(Attributes = "HideInEditor")
    bool UsesRadius() const { return Shape != AmplitudeEnvironmentShape::Box; }

    AmplitudeAudio::Spatial::ZoneShape MakeZone() const;

    void OnEngineStarted() override;
    void OnEngineStopping() override;

    // Actor
    void OnBeginPlay() override;
    void OnEndPlay() override;
    void OnEnable() override;
    void OnDisable() override;
    void OnTransformChanged() override;
#if USE_EDITOR
    void OnDebugDrawSelected() override;
#endif
    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

private:
    void Register();
    void Unregister();
    void PushState() const;
    AmplitudeAudio::EnvironmentCandidate MakeCandidate() const;

    SparkyStudios::Audio::Amplitude::Environment _environment;
    uint64 _id = 0;
    bool _registered = false;
};

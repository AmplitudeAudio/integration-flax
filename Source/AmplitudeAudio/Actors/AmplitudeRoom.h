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

#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>
#include <AmplitudeAudio/Runtime/Spatial/SpatialMath.h>

/// <summary>Material type of a room wall.</summary>
API_ENUM(Namespace = "AmplitudeAudio")
enum class AmplitudeWallMaterial
{
    Transparent = 0,
    AcousticTile = 1,
    CarpetOnConcrete = 2,
    HeavyDrapes = 3,
    GypsumBoard = 4,
    ConcreteUnpainted = 5,
    Wood = 6,
    BrickPainted = 7,
    FoamPanel = 8,
    Glass = 9,
    PlasterSmooth = 10,
    Metal = 11,
    Marble = 12,
    WaterSurface = 13,
    IceSurface = 14,
};

/// <summary>An Amplitude room actor defining acoustic properties and wall materials of an enclosed space.</summary>
API_CLASS(Namespace = "AmplitudeAudio", Attributes = "ActorContextMenu(\"New/Amplitude/Room\"), ActorToolbox(\"Other\")")
class AMPLITUDEAUDIO_API AmplitudeRoom : public Actor, public AmplitudeAudio::IAmplitudeServiceClient
{
    DECLARE_SCENE_OBJECT(AmplitudeRoom);

public:
    /// <summary>Dimensions of the room box in world units.</summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    Vector3 Size = Vector3(400.0f, 300.0f, 400.0f);

    /// <summary>Gain factor applied to reverb/early reflections in the room.</summary>
    API_FIELD(Attributes = "EditorOrder(1), Limit(0, 1, 0.01f)")
    float Gain = 1.0f;

    /// <summary>Material of the left wall.</summary>
    API_FIELD(Attributes = "EditorOrder(10), EditorDisplay(\"Wall Materials\")")
    AmplitudeWallMaterial LeftWall = AmplitudeWallMaterial::GypsumBoard;

    /// <summary>Material of the right wall.</summary>
    API_FIELD(Attributes = "EditorOrder(11), EditorDisplay(\"Wall Materials\")")
    AmplitudeWallMaterial RightWall = AmplitudeWallMaterial::GypsumBoard;

    /// <summary>Material of the floor.</summary>
    API_FIELD(Attributes = "EditorOrder(12), EditorDisplay(\"Wall Materials\")")
    AmplitudeWallMaterial Floor = AmplitudeWallMaterial::GypsumBoard;

    /// <summary>Material of the ceiling.</summary>
    API_FIELD(Attributes = "EditorOrder(13), EditorDisplay(\"Wall Materials\")")
    AmplitudeWallMaterial Ceiling = AmplitudeWallMaterial::GypsumBoard;

    /// <summary>Material of the front wall.</summary>
    API_FIELD(Attributes = "EditorOrder(14), EditorDisplay(\"Wall Materials\")")
    AmplitudeWallMaterial FrontWall = AmplitudeWallMaterial::GypsumBoard;

    /// <summary>Material of the back wall.</summary>
    API_FIELD(Attributes = "EditorOrder(15), EditorDisplay(\"Wall Materials\")")
    AmplitudeWallMaterial BackWall = AmplitudeWallMaterial::GypsumBoard;

    /// <summary>Whether the SDK room handle is valid.</summary>
    API_PROPERTY()
    bool IsValid() const;

    /// <summary>Returns the room ID allocated for this room.</summary>
    API_PROPERTY()
    uint64 GetRoomId() const { return _id; }

    /// <summary>Returns the SDK room handle.</summary>
    const SparkyStudios::Audio::Amplitude::Room& GetHandle() const { return _room; }

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
    AmplitudeAudio::Spatial::RoomCandidate MakeCandidate() const;

    SparkyStudios::Audio::Amplitude::Room _room;
    uint64 _id = 0;
    bool _registered = false;
};

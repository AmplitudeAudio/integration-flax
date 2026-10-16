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

#include "TestFixture.h"

#include <Engine/Scripting/ScriptingObject.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Actors/AmplitudeSoundInstance.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Core/Helpers.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;
using SparkyStudios::Audio::Amplitude::AmVector3;

namespace
{
    struct InstancedChannel
    {
        ServiceGuard Guard;
        AmplitudeEntity* Entity = nullptr;
        AmplitudeChannel* Channel = nullptr;

        InstancedChannel()
        {
            String root, buildPath;
            REQUIRE_TEST_PROJECT(root, buildPath);
            REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), root, buildPath));
            REQUIRE(AmplitudeService::Get().LoadSoundBank(TEXT("sample_01.ambank"), true));

            Entity = ScriptingObject::NewObject<AmplitudeEntity>();
            Entity->OnBeginPlay();
            AmplitudeSoundObjectRef grass;
            grass.ID = 201;
            Channel = Amplitude::Play(grass, Entity);
            REQUIRE(Channel->IsValid());
            Channel->GetHandle().EnableInstancing(SparkyStudios::Audio::Amplitude::eChannelInstanceMode_Blended);
        }

        ~InstancedChannel()
        {
            Amplitude::StopAll();
            Channel->DeleteObjectNow();
            Entity->OnEndPlay();
            Entity->DeleteObjectNow();
        }
    };
} // namespace

TEST_CASE("a bound instance follows its actor", "[sound-instance]")
{
    InstancedChannel fixture;
    auto* instance = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    instance->SetPosition(Vector3(100, 0, 0));
    instance->OnBeginPlay();

    const auto handle = fixture.Channel->GetHandle().AddInstance(AmplitudeAudio::FlaxVec3ToAmVec3(instance->GetPosition()));
    REQUIRE(handle.Valid());
    instance->Bind(handle);
    CHECK(instance->IsBound());

    instance->SetPosition(Vector3(0, 0, 300));
    const AmVector3 expected = AmplitudeAudio::FlaxVec3ToAmVec3(Vector3(0, 0, 300));
    CHECK(handle.GetLocation()[0] == Approx(expected[0]));
    CHECK(handle.GetLocation()[1] == Approx(expected[1]));
    CHECK(handle.GetLocation()[2] == Approx(expected[2]));

    instance->Unbind();
    CHECK_FALSE(instance->IsBound());
    instance->OnEndPlay();
    instance->DeleteObjectNow();
}

TEST_CASE("weight reaches the channel instance", "[sound-instance]")
{
    InstancedChannel fixture;
    auto* instance = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    instance->Weight = 0.25f;
    instance->OnBeginPlay();
    const auto handle = fixture.Channel->GetHandle().AddInstance(AmplitudeAudio::FlaxVec3ToAmVec3(Vector3::Zero));
    instance->Bind(handle);

    CHECK(handle.GetWeight() == Approx(0.25f));

    instance->Unbind();
    instance->OnEndPlay();
    instance->DeleteObjectNow();
}

TEST_CASE("an unbound instance does not touch the engine when it moves", "[sound-instance]")
{
    auto* instance = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    instance->SetPosition(Vector3(1, 2, 3));
    CHECK_FALSE(instance->IsBound());
    instance->DeleteObjectNow();
}

TEST_CASE("an instance registers with the processors only while bound", "[sound-instance]")
{
    InstancedChannel fixture;
    auto* instance = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    instance->OnBeginPlay();

    const int occlusion = AmplitudeService::Get().GetOcclusion().GetSourceCount();
    const int spatial = AmplitudeService::Get().GetSpatial().GetTargetCount();
    const auto handle = fixture.Channel->GetHandle().AddInstance(AmplitudeAudio::FlaxVec3ToAmVec3(Vector3::Zero));
    instance->Bind(handle);
    CHECK(AmplitudeService::Get().GetOcclusion().GetSourceCount() == occlusion + 1);
    CHECK(AmplitudeService::Get().GetSpatial().GetTargetCount() == spatial + 1);

    instance->Unbind();
    CHECK(AmplitudeService::Get().GetOcclusion().GetSourceCount() == occlusion);
    CHECK(AmplitudeService::Get().GetSpatial().GetTargetCount() == spatial);

    instance->OnEndPlay();
    instance->DeleteObjectNow();
}

TEST_CASE("a bound instance is released when the engine stops", "[sound-instance]")
{
    auto* instance = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    {
        InstancedChannel fixture;
        instance->OnBeginPlay();
        const auto handle = fixture.Channel->GetHandle().AddInstance(AmplitudeAudio::FlaxVec3ToAmVec3(Vector3::Zero));
        instance->Bind(handle);
        REQUIRE(instance->IsBound());

        AmplitudeService::Get().Shutdown();
        CHECK_FALSE(instance->IsBound());
    }
    instance->OnEndPlay();
    instance->DeleteObjectNow();
}

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

#include "TestFixture.h"

#include <Engine/Scripting/ScriptingObject.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Types/AmplitudeEventHandle.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

// The handle owns an SDK event instance. At Stop the engine shuts down before deferred deletes are flushed,
// so the wrapper must let go of it while the engine is still alive.
TEST_CASE("an event handle outliving the engine is safe to delete", "[event-handle]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), root, buildPath));
    REQUIRE(AmplitudeService::Get().LoadSoundBank(TEXT("sample_01.ambank"), true));

    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->OnEnable();

    AmplitudeEventRef play;
    play.ID = AmplitudeService::Get().GetEngine()->GetEventHandle(SparkyStudios::Audio::Amplitude::AmString("play_throw"))->GetId();
    REQUIRE(play.IsValid());

    AmplitudeEventHandle* handle = Amplitude::TriggerEvent(play, entity);
    REQUIRE(handle->IsValid());

    entity->OnDisable();
    AmplitudeService::Get().Shutdown();

    CHECK_FALSE(handle->IsValid());
    handle->DeleteObjectNow();
    entity->DeleteObjectNow();
}

// Channel::Valid() only checks for a non-null state pointer, so a wrapper that outlives the engine would
// otherwise keep reporting valid and touch freed channel state on Stop/IsPlaying.
TEST_CASE("a channel wrapper invalidates when the engine stops", "[channel-lifetime]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), root, buildPath));
    REQUIRE(AmplitudeService::Get().LoadSoundBank(TEXT("sample_01.ambank"), true));

    AmplitudeSoundObjectRef grass;
    grass.ID = 201;
    AmplitudeChannel* channel = Amplitude::Play(grass);
    REQUIRE(channel->IsValid());

    AmplitudeService::Get().Shutdown();

    CHECK_FALSE(channel->IsValid());
    CHECK_FALSE(channel->IsPlaying());
    channel->Stop(0.0f);
    channel->DeleteObjectNow();
}

TEST_CASE("an entity never keeps a handle past engine shutdown", "[channel-lifetime]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->OnBeginPlay();
    REQUIRE(entity->IsValid());

    AmplitudeService::Get().Shutdown();
    CHECK_FALSE(entity->IsValid());

    entity->OnEndPlay();
    entity->DeleteObjectNow();
}

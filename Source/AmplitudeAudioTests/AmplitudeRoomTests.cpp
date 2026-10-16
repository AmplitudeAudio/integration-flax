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
#include <AmplitudeAudio/Actors/AmplitudeRoom.h>
#include <AmplitudeAudio/Core/Helpers.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;
using SparkyStudios::Audio::Amplitude::AmVector3;

TEST_CASE("a room handle follows the engine lifetime", "[room]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    auto* room = ScriptingObject::NewObject<AmplitudeRoom>();

    room->OnBeginPlay();
    CHECK_FALSE(room->IsValid());
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    CHECK(room->IsValid());

    AmplitudeService::Get().Shutdown();
    CHECK_FALSE(room->IsValid());

    room->OnEndPlay();
    room->DeleteObjectNow();
}

TEST_CASE("a room is findable by id while registered", "[room]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    auto* room = ScriptingObject::NewObject<AmplitudeRoom>();
    room->OnBeginPlay();

    CHECK(AmplitudeService::Get().FindRoom(room->GetRoomId()).Valid());
    room->OnEndPlay();
    CHECK_FALSE(AmplitudeService::Get().FindRoom(room->GetRoomId()).Valid());
    room->DeleteObjectNow();
}

TEST_CASE("an unregistered room id is not found", "[room]")
{
    CHECK_FALSE(AmplitudeService::Get().FindRoom(123456).Valid());
}

TEST_CASE("an entity inside a room is assigned to it", "[room]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    auto* room = ScriptingObject::NewObject<AmplitudeRoom>();
    room->SetPosition(Vector3(0, 0, 0));
    room->Size = Vector3(400, 300, 400);
    room->OnBeginPlay();
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->SetPosition(Vector3(10, 10, 10));
    entity->OnBeginPlay();

    // pc.config has track_rooms = false, so the processor is told to skip rooms: nothing is assigned.
    AmplitudeService::Get().Tick();
    CHECK_FALSE(entity->GetHandle().GetRoom().Valid());

    entity->OnEndPlay();
    room->OnEndPlay();
    entity->DeleteObjectNow();
    room->DeleteObjectNow();
}

TEST_CASE("room dimensions use the same axis mapping as positions", "[room]")
{
    const AmVector3 mapped = AmplitudeAudio::FlaxVec3ToAmVec3(Vector3(1, 2, 3));
    const AmVector3 dims = AmplitudeAudio::ToAmDimensions(Vector3(1, 2, 3));
    CHECK(dims[0] == Approx(Math::Abs(mapped[0])));
    CHECK(dims[1] == Approx(Math::Abs(mapped[1])));
    CHECK(dims[2] == Approx(Math::Abs(mapped[2])));
}

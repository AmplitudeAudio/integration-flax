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
#include <AmplitudeAudio/Core/Helpers.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

TEST_CASE("entity handle follows the engine lifetime", "[entity]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();

    entity->OnEnable();
    CHECK_FALSE(entity->IsValid());
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    CHECK(entity->IsValid());
    AmplitudeService::Get().Shutdown();
    CHECK_FALSE(entity->IsValid());

    entity->OnDisable();
    entity->DeleteObjectNow();
}

TEST_CASE("entity location follows the actor", "[entity]")
{
    using Catch::Detail::Approx;
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->OnEnable();

    entity->SetPosition(Vector3(1, 2, 3));
    const auto expected = AmplitudeAudio::FlaxVec3ToAmVec3(Vector3(1, 2, 3));
    CHECK(entity->GetHandle().GetLocation()[0] == Approx(expected[0]));
    CHECK(entity->GetHandle().GetLocation()[1] == Approx(expected[1]));
    CHECK(entity->GetHandle().GetLocation()[2] == Approx(expected[2]));

    entity->OnDisable();
    entity->DeleteObjectNow();
}

TEST_CASE("FindNearest returns the actor itself, then ancestors", "[entity]")
{
    auto* parent = ScriptingObject::NewObject<AmplitudeEntity>();
    auto* child = ScriptingObject::NewObject<Actor>();
    child->SetParent(parent, false);

    CHECK(AmplitudeEntity::FindNearest(parent) == parent);
    CHECK(AmplitudeEntity::FindNearest(child) == parent);
    CHECK(AmplitudeEntity::FindNearest(nullptr) == nullptr);

    parent->DeleteObjectNow();
}

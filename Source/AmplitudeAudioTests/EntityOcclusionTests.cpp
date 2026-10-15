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
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

TEST_CASE("entity registers with occlusion on begin play and unregisters on end play", "[entity-occlusion]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    AmplitudeService& audioService = AmplitudeService::Get();
    CHECK(audioService.GetOcclusion().GetSourceCount() == 0);

    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->ObstructionMode = AmplitudeObstructionMode::SingleRay;
    entity->OnBeginPlay();

    CHECK(entity->IsValid());
    CHECK(audioService.GetOcclusion().GetSourceCount() == 1);

    entity->OnEndPlay();
    CHECK_FALSE(entity->IsValid());
    CHECK(audioService.GetOcclusion().GetSourceCount() == 0);

    entity->DeleteObjectNow();
}

TEST_CASE("service shutdown clears the processor's sources", "[entity-occlusion]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    AmplitudeService& audioService = AmplitudeService::Get();

    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->ObstructionMode = AmplitudeObstructionMode::SingleRay;
    entity->OnBeginPlay();
    CHECK(audioService.GetOcclusion().GetSourceCount() == 1);

    audioService.Shutdown();
    CHECK(audioService.GetOcclusion().GetSourceCount() == 0);
    CHECK_FALSE(entity->IsValid());

    entity->OnEndPlay();
    entity->DeleteObjectNow();
}

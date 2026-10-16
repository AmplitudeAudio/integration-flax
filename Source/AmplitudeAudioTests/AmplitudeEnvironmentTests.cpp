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
#include <AmplitudeAudio/Actors/AmplitudeEnvironment.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

TEST_CASE("an environment handle follows the engine lifetime", "[environment]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();

    environment->OnBeginPlay();
    CHECK_FALSE(environment->IsValid());
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    CHECK(environment->IsValid());

    AmplitudeService::Get().Shutdown();
    CHECK_FALSE(environment->IsValid());

    environment->OnEndPlay();
    environment->DeleteObjectNow();
}

TEST_CASE("an entity inside an environment receives its factor", "[environment]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    // pc.config has track_environments = true.

    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->Size = Vector3(400, 300, 400);
    environment->BlendDistance = 200.0f;
    environment->OnBeginPlay();
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->SetPosition(Vector3(0, 0, 0));
    entity->OnBeginPlay();

    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(environment->GetEnvironmentId())) == Approx(1.0f));

    entity->SetPosition(Vector3(300, 0, 0)); // 100 outside the 200 half extent: halfway through the blend
    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(environment->GetEnvironmentId())) == Approx(0.5f));

    entity->OnEndPlay();
    environment->OnEndPlay();
    entity->DeleteObjectNow();
    environment->DeleteObjectNow();
}

TEST_CASE("a destroyed environment zeroes the factor", "[environment]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->OnBeginPlay();
    const auto id = static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(environment->GetEnvironmentId());
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->OnBeginPlay();
    AmplitudeService::Get().Tick();
    REQUIRE(entity->GetHandle().GetEnvironmentFactor(id) == Approx(1.0f));

    environment->OnEndPlay();
    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(0.0f));

    entity->OnEndPlay();
    entity->DeleteObjectNow();
    environment->DeleteObjectNow();
}

TEST_CASE("an environment is a box by default and describes its zone", "[environment]")
{
    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->SetPosition(Vector3(10, 20, 30));
    environment->Size = Vector3(400, 300, 400);

    CHECK(environment->Shape == AmplitudeEnvironmentShape::Box);
    const auto zone = environment->MakeZone();
    CHECK(zone.Kind == AmplitudeAudio::Spatial::ZoneKind::Box);
    CHECK(zone.Center == Vector3(10, 20, 30));
    CHECK(zone.Extents == Vector3(200, 150, 200));
    environment->DeleteObjectNow();
}

TEST_CASE("a sphere environment describes its zone", "[environment][zone-sphere]")
{
    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->SetPosition(Vector3(1, 2, 3));
    environment->Shape = AmplitudeEnvironmentShape::Sphere;
    environment->Radius = 120.0f;

    const auto zone = environment->MakeZone();
    CHECK(zone.Kind == AmplitudeAudio::Spatial::ZoneKind::Sphere);
    CHECK(zone.Center == Vector3(1, 2, 3));
    CHECK(zone.Extents.X == Approx(120.0f));
    environment->DeleteObjectNow();
}

TEST_CASE("an entity inside a sphere environment receives its factor", "[environment][zone-sphere]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->Shape = AmplitudeEnvironmentShape::Sphere;
    environment->Radius = 100.0f;
    environment->BlendDistance = 100.0f;
    environment->OnBeginPlay();
    const auto id = static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(environment->GetEnvironmentId());
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->OnBeginPlay();

    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(1.0f));

    entity->SetPosition(Vector3(150, 0, 0)); // 50 outside a radius of 100, halfway through the blend
    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(0.5f));

    entity->OnEndPlay();
    environment->OnEndPlay();
    entity->DeleteObjectNow();
    environment->DeleteObjectNow();
}

TEST_CASE("a capsule environment describes its zone", "[environment][zone-capsule]")
{
    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->Shape = AmplitudeEnvironmentShape::Capsule;
    environment->Radius = 50.0f;
    environment->HalfHeight = 150.0f;

    const auto zone = environment->MakeZone();
    CHECK(zone.Kind == AmplitudeAudio::Spatial::ZoneKind::Capsule);
    CHECK(zone.Extents.X == Approx(50.0f));
    CHECK(zone.Extents.Y == Approx(150.0f));
    environment->DeleteObjectNow();
}

TEST_CASE("an entity beside a capsule environment receives its factor", "[environment][zone-capsule]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->Shape = AmplitudeEnvironmentShape::Capsule;
    environment->Radius = 50.0f;
    environment->HalfHeight = 150.0f;
    environment->BlendDistance = 100.0f;
    environment->OnBeginPlay();
    const auto id = static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(environment->GetEnvironmentId());
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->SetPosition(Vector3(0, 100, 0));
    entity->OnBeginPlay();

    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(1.0f));

    entity->SetPosition(Vector3(100, 0, 0)); // 50 outside the radius: halfway through the blend
    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(0.5f));

    entity->OnEndPlay();
    environment->OnEndPlay();
    entity->DeleteObjectNow();
    environment->DeleteObjectNow();
}

TEST_CASE("a cone environment describes its zone", "[environment][zone-cone]")
{
    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->SetPosition(Vector3(5, 6, 7));
    environment->Shape = AmplitudeEnvironmentShape::Cone;
    environment->Radius = 100.0f;
    environment->Height = 300.0f;

    const auto zone = environment->MakeZone();
    CHECK(zone.Kind == AmplitudeAudio::Spatial::ZoneKind::Cone);
    CHECK(zone.Center == Vector3(5, 6, 7));
    CHECK(zone.Extents.X == Approx(100.0f));
    CHECK(zone.Extents.Y == Approx(300.0f));
    environment->DeleteObjectNow();
}

TEST_CASE("an entity inside a cone environment receives its factor", "[environment][zone-cone]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    auto* environment = ScriptingObject::NewObject<AmplitudeEnvironment>();
    environment->Shape = AmplitudeEnvironmentShape::Cone;
    environment->Radius = 100.0f;
    environment->Height = 200.0f;
    environment->BlendDistance = 100.0f;
    environment->OnBeginPlay();
    const auto id = static_cast<SparkyStudios::Audio::Amplitude::AmEnvironmentID>(environment->GetEnvironmentId());
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->SetPosition(Vector3(0, 0, 100)); // on the axis, forward of the apex
    entity->OnBeginPlay();

    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(1.0f));

    entity->SetPosition(Vector3(0, 0, -50)); // behind the apex, halfway through the blend
    AmplitudeService::Get().Tick();
    CHECK(entity->GetHandle().GetEnvironmentFactor(id) == Approx(0.5f));

    entity->OnEndPlay();
    environment->OnEndPlay();
    entity->DeleteObjectNow();
    environment->DeleteObjectNow();
}

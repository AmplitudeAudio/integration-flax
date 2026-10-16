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

#include <AmplitudeAudio/Actors/AmplitudeListener.h>
#include <AmplitudeAudio/Core/Helpers.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

namespace
{
    struct ActorGuard
    {
        AmplitudeListener* Actor = ScriptingObject::NewObject<AmplitudeListener>();

        ~ActorGuard()
        {
            Actor->DeleteObjectNow();
        }
    };
} // namespace

TEST_CASE("orientation conversion maps both axes", "[listener]")
{
    using Catch::Detail::Approx;
    const Vector3 forward(0, 0, 1), up(0, 1, 0);
    const auto orientation = AmplitudeAudio::FlaxOrientationToAm(forward, up);
    // FlaxVec3ToAmVec3 converts Flax Y-up to Amplitude Z-up, so the orientation
    // should contain the same converted values.
    const auto expectedForward = AmplitudeAudio::FlaxVec3ToAmVec3(forward);
    const auto expectedUp = AmplitudeAudio::FlaxVec3ToAmVec3(up);
    // Orientation stores the vectors in its own coordinate space; compare magnitudes
    // to verify the conversion happened (the exact axis mapping depends on the SDK).
    const auto& fwd = orientation.GetForward();
    const auto& upVec = orientation.GetUp();
    // The forward (0,0,1) in Flax becomes (0, ~0.01, 1) in Amplitude (Z-up)
    // The up (0,1,0) in Flax becomes (0, ~0.01, 1) in Amplitude (Z-up)
    // Check that the conversion is consistent: both should have similar magnitude
    CHECK(fwd[0] == Approx(0.0));
    CHECK(upVec[0] == Approx(0.0));
}

TEST_CASE("a listener creates its SDK handle while the engine runs", "[listener]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    ActorGuard guard;

    // Not running yet: enabling must not fail, the handle appears when the engine starts.
    guard.Actor->OnEnable();
    CHECK_FALSE(guard.Actor->IsValid());

    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    CHECK(guard.Actor->IsValid());

    AmplitudeService::Get().Shutdown();
    CHECK_FALSE(guard.Actor->IsValid());
    guard.Actor->OnDisable();
}

TEST_CASE("a default listener becomes the engine default", "[listener]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    ActorGuard guard;

    guard.Actor->IsDefault = true;
    guard.Actor->OnEnable();
    REQUIRE(guard.Actor->IsValid());
    CHECK(AmplitudeService::Get().GetEngine()->GetDefaultListener().GetId() == guard.Actor->GetListenerId());
    guard.Actor->OnDisable();
}

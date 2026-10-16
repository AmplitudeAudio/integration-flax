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

#include <ThirdParty/catch2/catch.hpp>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

TEST_CASE("no warning when the game tracks both", "[tracking]")
{
    CHECK(AmplitudeService::BuildTrackingWarnings(true, true).IsEmpty());
}

TEST_CASE("each disabled flag warns once and names its config key", "[tracking]")
{
    const auto rooms = AmplitudeService::BuildTrackingWarnings(false, true);
    REQUIRE(rooms.Count() == 1);
    CHECK(rooms[0].Contains(TEXT("track_rooms")));

    const auto environments = AmplitudeService::BuildTrackingWarnings(true, false);
    REQUIRE(environments.Count() == 1);
    CHECK(environments[0].Contains(TEXT("track_environments")));

    CHECK(AmplitudeService::BuildTrackingWarnings(false, false).Count() == 2);
}

TEST_CASE("startup records the warnings of the loaded config", "[tracking]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    // pc.config ships with track_rooms = false.
    bool mentionsRooms = false;
    for (const String& warning : AmplitudeService::Get().GetStartupWarnings())
        mentionsRooms |= warning.Contains(TEXT("track_rooms"));
    CHECK(mentionsRooms);
}

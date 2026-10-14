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

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/IAmplitudeServiceClient.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

namespace
{
    struct Recorder : AmplitudeAudio::IAmplitudeServiceClient
    {
        int Started = 0;
        int Stopping = 0;

        void OnEngineStarted() override { Started++; }
        void OnEngineStopping() override { Stopping++; }
    };
} // namespace

TEST_CASE("clients are told when the engine starts and stops", "[service-client]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    Recorder client;
    AmplitudeService::Get().AddClient(&client);

    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    CHECK(client.Started == 1);

    AmplitudeService::Get().Shutdown();
    CHECK(client.Stopping == 1);
    AmplitudeService::Get().RemoveClient(&client);
}

TEST_CASE("a client added while running is started at once", "[service-client]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    Recorder client;
    AmplitudeService::Get().AddClient(&client);
    CHECK(client.Started == 1);
    AmplitudeService::Get().RemoveClient(&client);
    CHECK(client.Stopping == 0);
}

TEST_CASE("posted work runs on the next tick", "[service-client]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    int ran = 0;
    AmplitudeService::Get().Post([&ran] { ran++; });
    CHECK(ran == 0);
    AmplitudeService::Get().Tick();
    CHECK(ran == 1);
}

TEST_CASE("ids are unique and non-zero", "[service-client]")
{
    const auto a = AmplitudeService::Get().AllocateId();
    const auto b = AmplitudeService::Get().AllocateId();
    CHECK(a != 0);
    CHECK(a != b);
}

TEST_CASE("seconds convert to engine milliseconds", "[service-client]")
{
    using Catch::Detail::Approx;
    CHECK(AmplitudeAudio::SecondsToAmTime(0.25f) == Approx(250.0));
}

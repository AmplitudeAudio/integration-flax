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

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

TEST_CASE("service starts with a valid profile and stops cleanly", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    AmplitudeService& service = AmplitudeService::Get();

    REQUIRE(service.Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), root, buildPath));
    CHECK(service.IsRunning());
    CHECK_FALSE(service.IsUsingPackage());
    CHECK(service.IsSoundBankLoaded(TEXT("init.ambank")));

    service.Shutdown();
    CHECK_FALSE(service.IsRunning());
    CHECK_FALSE(service.IsSoundBankLoaded(TEXT("init.ambank")));
}

TEST_CASE("service restarts after a shutdown", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    AmplitudeService& service = AmplitudeService::Get();

    REQUIRE(service.Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    service.Shutdown();
    CHECK(service.Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
}

TEST_CASE("startup fails without an engine config file", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;

    CHECK_FALSE(AmplitudeService::Get().Startup(MakeProfile(TEXT("")), root, buildPath));
    CHECK_FALSE(AmplitudeService::Get().IsRunning());
}

TEST_CASE("startup fails with a missing engine config and leaves a clean state", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    AmplitudeService& service = AmplitudeService::Get();

    CHECK_FALSE(service.Startup(MakeProfile(TEXT("missing.amconfig")), root, buildPath));
    CHECK_FALSE(service.IsRunning());

    CHECK(service.Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
}

TEST_CASE("startup fails with a missing init bank", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;

    CHECK_FALSE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("missing.ambank")}), root, buildPath));
    CHECK_FALSE(AmplitudeService::Get().IsRunning());
}

TEST_CASE("sound banks are reference counted", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    AmplitudeService& service = AmplitudeService::Get();
    REQUIRE(service.Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    REQUIRE(service.LoadSoundBank(TEXT("sample_01.ambank"), true));
    REQUIRE(service.LoadSoundBank(TEXT("sample_01.ambank"), true));
    CHECK(service.IsSoundBankLoaded(TEXT("sample_01.ambank")));

    service.UnloadSoundBank(TEXT("sample_01.ambank"));
    CHECK(service.IsSoundBankLoaded(TEXT("sample_01.ambank")));

    service.UnloadSoundBank(TEXT("sample_01.ambank"));
    CHECK_FALSE(service.IsSoundBankLoaded(TEXT("sample_01.ambank")));
}

TEST_CASE("loading a missing sound bank fails", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    CHECK_FALSE(AmplitudeService::Get().LoadSoundBank(TEXT("missing.ambank"), true));
}

TEST_CASE("async sound bank load completes on later ticks", "[service]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard guard;
    AmplitudeService& service = AmplitudeService::Get();
    REQUIRE(service.Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));

    REQUIRE(service.LoadSoundBank(TEXT("sample_01.ambank"), false));
    for (int32 frame = 0; frame < 10000 && !service.IsSoundBankLoaded(TEXT("sample_01.ambank")); ++frame)
    {
        service.Tick();
        Platform::Sleep(1);
    }

    CHECK(service.IsSoundBankLoaded(TEXT("sample_01.ambank")));
}

TEST_CASE("bank calls are ignored while the engine is stopped", "[service]")
{
    AmplitudeService& service = AmplitudeService::Get();
    REQUIRE_FALSE(service.IsRunning());

    CHECK_FALSE(service.LoadSoundBank(TEXT("init.ambank"), true));
    service.UnloadSoundBank(TEXT("init.ambank"));
    CHECK_FALSE(service.IsSoundBankLoaded(TEXT("init.ambank")));
}

TEST_CASE("service reads from a package next to the build path", "[service][package]")
{
    String packageRoot;
    if (!AmplitudeAudioTests::GetTestPackageRoot(packageRoot))
        FAIL("AMPLITUDE_TEST_PACKAGE_ROOT is not set; run the tests through Tests/run-native-tests.ps1.");

    ServiceGuard guard;
    AmplitudeService& service = AmplitudeService::Get();

    REQUIRE(service.Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), packageRoot, TEXT("am_build")));
    CHECK(service.IsUsingPackage());
    CHECK(service.IsSoundBankLoaded(TEXT("init.ambank")));
}

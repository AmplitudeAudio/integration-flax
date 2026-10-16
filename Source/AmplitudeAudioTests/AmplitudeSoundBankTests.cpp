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

#include <AmplitudeAudio/Scripts/AmplitudeSoundBank.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

namespace
{
    struct ScriptGuard
    {
        AmplitudeSoundBank* Script = ScriptingObject::NewObject<AmplitudeSoundBank>();

        ~ScriptGuard()
        {
            Script->DeleteObjectNow();
        }
    };
} // namespace

TEST_CASE("sound bank script holds its bank while enabled", "[sound-bank-script]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    ScriptGuard guard;

    guard.Script->Bank.File = TEXT("sample_01.ambank");
    guard.Script->OnEnable();
    CHECK(guard.Script->IsLoaded());

    guard.Script->OnDisable();
    CHECK_FALSE(guard.Script->IsLoaded());
    CHECK_FALSE(AmplitudeService::Get().IsSoundBankLoaded(TEXT("sample_01.ambank")));
}

TEST_CASE("sound bank script releases the bank it loaded even if Bank was edited", "[sound-bank-script]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), root, buildPath));
    ScriptGuard guard;

    guard.Script->Bank.File = TEXT("sample_01.ambank");
    guard.Script->OnEnable();
    guard.Script->Bank.File = TEXT("init.ambank");
    guard.Script->OnDisable();

    CHECK_FALSE(AmplitudeService::Get().IsSoundBankLoaded(TEXT("sample_01.ambank")));
    CHECK(AmplitudeService::Get().IsSoundBankLoaded(TEXT("init.ambank")));
}

TEST_CASE("sound bank script without a bank does nothing", "[sound-bank-script]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    ScriptGuard guard;

    guard.Script->OnEnable();
    CHECK_FALSE(guard.Script->IsLoaded());
    guard.Script->OnDisable();
}

TEST_CASE("async sound bank script loads on later ticks", "[sound-bank-script]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    ScriptGuard guard;

    guard.Script->Bank.File = TEXT("sample_01.ambank");
    guard.Script->LoadAsync = true;
    guard.Script->OnEnable();

    for (int32 frame = 0; frame < 10000 && !guard.Script->IsLoaded(); ++frame)
    {
        AmplitudeService::Get().Tick();
        Platform::Sleep(1);
    }

    CHECK(guard.Script->IsLoaded());
    guard.Script->OnDisable();
}

TEST_CASE("enabling twice holds a single reference", "[sound-bank-script]")
{
    String root, buildPath;
    REQUIRE_TEST_PROJECT(root, buildPath);
    ServiceGuard service;
    REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig")), root, buildPath));
    ScriptGuard guard;

    guard.Script->Bank.File = TEXT("sample_01.ambank");
    guard.Script->OnEnable();
    guard.Script->OnEnable();
    guard.Script->OnDisable();

    CHECK_FALSE(AmplitudeService::Get().IsSoundBankLoaded(TEXT("sample_01.ambank")));
}

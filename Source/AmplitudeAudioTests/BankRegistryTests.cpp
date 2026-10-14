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

#include <climits>

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Types/String.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Core/BankRegistry.h>

using AmplitudeAudio::BankRegistry;

namespace
{
    struct FakeEngine
    {
        Dictionary<String, BankRegistry::BankId> Loadable;
        Array<String> Loads;
        Array<BankRegistry::BankId> Unloads;
        int32 StartCalls = 0;
        int32 FinalizeCalls = 0;
        int32 FinalizeAfter = 0;

        FakeEngine()
        {
            Loadable[TEXT("init.ambank")] = 1;
            Loadable[TEXT("sample_01.ambank")] = 2;
        }

        BankRegistry::Callbacks Callbacks()
        {
            BankRegistry::Callbacks callbacks;
            callbacks.Load = [this](const String& file, BankRegistry::BankId& outId)
            {
                Loads.Add(file);
                return Loadable.TryGet(file, outId);
            };
            callbacks.Unload = [this](BankRegistry::BankId id)
            {
                Unloads.Add(id);
            };
            callbacks.StartLoadFiles = [this]
            {
                ++StartCalls;
            };
            callbacks.TryFinalizeLoadFiles = [this]
            {
                return ++FinalizeCalls > FinalizeAfter;
            };
            return callbacks;
        }
    };
} // namespace

TEST_CASE("first acquire loads the bank and starts loading its files", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());

    CHECK(registry.Acquire(TEXT("init.ambank")));
    REQUIRE(engine.Loads.Count() == 1);
    CHECK(engine.Loads[0] == TEXT("init.ambank"));
    CHECK(engine.StartCalls == 1);
    CHECK(registry.RefCount(TEXT("init.ambank")) == 1);
    CHECK(registry.HasPending());
    CHECK_FALSE(registry.IsLoaded(TEXT("init.ambank")));
}

TEST_CASE("second acquire only adds a reference", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());

    registry.Acquire(TEXT("init.ambank"));
    CHECK(registry.Acquire(TEXT("init.ambank")));
    CHECK(engine.Loads.Count() == 1);
    CHECK(engine.StartCalls == 1);
    CHECK(registry.RefCount(TEXT("init.ambank")) == 2);
}

TEST_CASE("failed load records nothing", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());

    CHECK_FALSE(registry.Acquire(TEXT("missing.ambank")));
    CHECK(registry.RefCount(TEXT("missing.ambank")) == 0);
    CHECK(engine.StartCalls == 0);
    CHECK_FALSE(registry.HasPending());
}

TEST_CASE("bank unloads when its last reference is released", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());

    registry.Acquire(TEXT("init.ambank"));
    registry.Acquire(TEXT("init.ambank"));

    registry.Release(TEXT("init.ambank"));
    CHECK(engine.Unloads.Count() == 0);
    CHECK(registry.RefCount(TEXT("init.ambank")) == 1);

    registry.Release(TEXT("init.ambank"));
    REQUIRE(engine.Unloads.Count() == 1);
    CHECK(engine.Unloads[0] == 1);
    CHECK(registry.RefCount(TEXT("init.ambank")) == 0);
}

TEST_CASE("releasing an unknown bank does nothing", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());

    registry.Release(TEXT("init.ambank"));
    CHECK(engine.Unloads.Count() == 0);
}

TEST_CASE("bank is loaded only once its files finalize", "[bank-registry]")
{
    FakeEngine engine;
    engine.FinalizeAfter = 2;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));

    registry.Update();
    registry.Update();
    CHECK_FALSE(registry.IsLoaded(TEXT("init.ambank")));

    registry.Update();
    CHECK(registry.IsLoaded(TEXT("init.ambank")));
    CHECK_FALSE(registry.HasPending());
}

TEST_CASE("update does not poll when nothing is pending", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());

    registry.Update();
    CHECK(engine.FinalizeCalls == 0);
}

TEST_CASE("finalizing marks every pending bank loaded", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));
    registry.Acquire(TEXT("sample_01.ambank"));

    registry.Update();
    CHECK(registry.IsLoaded(TEXT("init.ambank")));
    CHECK(registry.IsLoaded(TEXT("sample_01.ambank")));
}

TEST_CASE("wait for pending succeeds once files finalize", "[bank-registry]")
{
    FakeEngine engine;
    engine.FinalizeAfter = 3;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));

    CHECK(registry.WaitForPending(1000));
    CHECK(registry.IsLoaded(TEXT("init.ambank")));
}

TEST_CASE("wait for pending times out", "[bank-registry]")
{
    FakeEngine engine;
    engine.FinalizeAfter = INT_MAX;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));

    CHECK_FALSE(registry.WaitForPending(5));
    CHECK_FALSE(registry.IsLoaded(TEXT("init.ambank")));
}

TEST_CASE("release all unloads every bank", "[bank-registry]")
{
    FakeEngine engine;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));
    registry.Acquire(TEXT("init.ambank"));
    registry.Acquire(TEXT("sample_01.ambank"));

    registry.ReleaseAll();
    CHECK(engine.Unloads.Count() == 2);
    CHECK(registry.RefCount(TEXT("init.ambank")) == 0);
    CHECK(registry.RefCount(TEXT("sample_01.ambank")) == 0);
    CHECK_FALSE(registry.HasPending());
}

TEST_CASE("releasing the only pending bank clears pending", "[bank-registry]")
{
    FakeEngine engine;
    engine.FinalizeAfter = INT_MAX;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));

    registry.Release(TEXT("init.ambank"));
    CHECK_FALSE(registry.HasPending());
    CHECK(registry.WaitForPending(5));
}

TEST_CASE("releasing a pending bank keeps pending false when only loaded banks remain", "[bank-registry]")
{
    FakeEngine engine;
    engine.FinalizeAfter = 0;
    BankRegistry registry(engine.Callbacks());
    registry.Acquire(TEXT("init.ambank"));
    registry.Update();

    engine.FinalizeAfter = INT_MAX;
    engine.FinalizeCalls = 0;
    registry.Acquire(TEXT("sample_01.ambank"));
    CHECK(registry.HasPending());

    registry.Release(TEXT("sample_01.ambank"));
    CHECK_FALSE(registry.HasPending());
    CHECK(registry.IsLoaded(TEXT("init.ambank")));
    CHECK(registry.WaitForPending(5));
}

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

#include <AmplitudeAudio/AmplitudeSettings.h>

TEST_CASE("obstruction settings have usable defaults", "[obstruction-settings]")
{
    AmplitudeSettings settings;
    const AmplitudeObstructionSettings& o = settings.Obstruction;

    CHECK(o.DefaultAmount == Approx(1.0f));
    CHECK(o.MaxDistance > o.FullObstructionDistance);
    CHECK(o.RayBudgetPerFrame > 0);
    CHECK(o.IndirectRayCount > 0);
    CHECK(o.AttackSmoothing >= 0.0f);
    CHECK(o.ReleaseSmoothing >= 0.0f);
    CHECK(o.Layers.Mask == MAX_uint32);
    CHECK_FALSE(o.DebugDraw);
    CHECK(o.MaterialAmounts.IsEmpty());
    CHECK(o.TagAmounts.IsEmpty());
}

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

#include <AmplitudeAudio/Runtime/Occlusion/ObstructionMath.h>

using namespace AmplitudeAudio::Obstruction;

TEST_CASE("transmission only ever increases with more surfaces", "[obstruction-math]")
{
    const float strong[] = {0.9f};
    const float strongThenWeak[] = {0.9f, 0.1f};
    CHECK(CombineTransmission(nullptr, 0) == Approx(0.0f));
    CHECK(CombineTransmission(strong, 1) == Approx(0.9f));
    CHECK(CombineTransmission(strongThenWeak, 2) == Approx(0.91f));
    CHECK(CombineTransmission(strongThenWeak, 2) > CombineTransmission(strong, 1));
}

TEST_CASE("full surfaces saturate at one", "[obstruction-math]")
{
    const float walls[] = {1.0f, 1.0f, 0.5f};
    CHECK(CombineTransmission(walls, 3) == Approx(1.0f));
}

TEST_CASE("distance scale is flat then linear", "[obstruction-math]")
{
    CHECK(DistanceScale(0.0f, 1000.0f, 5000.0f) == Approx(1.0f));
    CHECK(DistanceScale(1000.0f, 1000.0f, 5000.0f) == Approx(1.0f));
    CHECK(DistanceScale(3000.0f, 1000.0f, 5000.0f) == Approx(0.5f));
    CHECK(DistanceScale(5000.0f, 1000.0f, 5000.0f) == Approx(0.0f));
    CHECK(DistanceScale(9000.0f, 1000.0f, 5000.0f) == Approx(0.0f));
}

TEST_CASE("distance scale tolerates a degenerate range", "[obstruction-math]")
{
    CHECK(DistanceScale(10.0f, 1000.0f, 1000.0f) == Approx(1.0f));
    CHECK(DistanceScale(2000.0f, 1000.0f, 1000.0f) == Approx(0.0f));
}

TEST_CASE("smoothing uses attack when rising and release when falling", "[obstruction-math]")
{
    // One time constant covers ~63% of the gap.
    CHECK(SmoothTo(0.0f, 1.0f, 0.1f, 0.1f, 1.0f) == Approx(0.632f).margin(0.01f));
    CHECK(SmoothTo(1.0f, 0.0f, 0.1f, 1.0f, 0.1f) == Approx(0.368f).margin(0.01f));
    CHECK(SmoothTo(0.0f, 1.0f, 0.1f, 1.0f, 0.01f) < 0.2f);
}

TEST_CASE("zero time constants snap", "[obstruction-math]")
{
    CHECK(SmoothTo(0.0f, 1.0f, 0.016f, 0.0f, 0.5f) == Approx(1.0f));
    CHECK(SmoothTo(1.0f, 0.25f, 0.016f, 0.5f, 0.0f) == Approx(0.25f));
}

TEST_CASE("smoothing is frame rate independent", "[obstruction-math]")
{
    float a = 0.0f;
    for (int i = 0; i < 10; i++)
        a = SmoothTo(a, 1.0f, 0.01f, 0.2f, 0.2f);
    const float b = SmoothTo(0.0f, 1.0f, 0.1f, 0.2f, 0.2f);
    CHECK(a == Approx(b).margin(0.001f));
}

TEST_CASE("open direct path gives neither obstruction nor occlusion", "[obstruction-math]")
{
    const Classification c = Classify(0.0f, false, true);
    CHECK(c.Obstruction == Approx(0.0f));
    CHECK(c.Occlusion == Approx(0.0f));
}

TEST_CASE("blocked direct with an open indirect path is obstruction", "[obstruction-math]")
{
    const Classification c = Classify(0.8f, true, true);
    CHECK(c.Obstruction == Approx(0.8f));
    CHECK(c.Occlusion == Approx(0.0f));
}

TEST_CASE("everything blocked is occlusion and resets obstruction", "[obstruction-math]")
{
    const Classification c = Classify(0.8f, false, true);
    CHECK(c.Obstruction == Approx(0.0f));
    CHECK(c.Occlusion == Approx(0.8f));
}

TEST_CASE("single ray mode never occludes", "[obstruction-math]")
{
    const Classification c = Classify(0.8f, false, false);
    CHECK(c.Obstruction == Approx(0.8f));
    CHECK(c.Occlusion == Approx(0.0f));
}

TEST_CASE("disk offsets are deterministic and inside the unit disk", "[obstruction-math]")
{
    const Array<Float2> a = MakeDiskOffsets(16, 1337);
    const Array<Float2> b = MakeDiskOffsets(16, 1337);
    const Array<Float2> c = MakeDiskOffsets(16, 7);

    REQUIRE(a.Count() == 16);
    bool differs = false;
    for (int i = 0; i < 16; i++)
    {
        CHECK(a[i] == b[i]);
        CHECK(a[i].Length() <= 1.0001f);
        differs |= a[i] != c[i];
    }
    CHECK(differs);
    CHECK(MakeDiskOffsets(0, 1).Count() == 1);
}

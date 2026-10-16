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

#include <AmplitudeAudio/Runtime/Spatial/SpatialMath.h>

using namespace AmplitudeAudio::Spatial;

namespace
{
    OrientedBox Box(Vector3 center, Vector3 half, Quaternion rotation = Quaternion::Identity)
    {
        OrientedBox box;
        box.Center = center;
        box.HalfExtents = half;
        box.Rotation = rotation;
        return box;
    }
} // namespace

TEST_CASE("an axis aligned box contains its interior and surface", "[spatial-math]")
{
    const OrientedBox box = Box(Vector3(100, 0, 0), Vector3(50, 50, 50));
    CHECK(Contains(box, Vector3(100, 0, 0)));
    CHECK(Contains(box, Vector3(149, 49, -49)));
    CHECK(Contains(box, Vector3(150, 0, 0)));
    CHECK_FALSE(Contains(box, Vector3(151, 0, 0)));
    CHECK_FALSE(Contains(box, Vector3(100, 0, 60)));
}

TEST_CASE("a rotated box is tested in its own frame", "[spatial-math]")
{
    // 90 degrees around Y turns the long X axis into Z.
    const OrientedBox box = Box(Vector3::Zero, Vector3(100, 10, 10), Quaternion::Euler(0, 90, 0));
    CHECK(Contains(box, Vector3(0, 0, 90)));
    CHECK_FALSE(Contains(box, Vector3(90, 0, 0)));
}

TEST_CASE("distance is zero inside and euclidean outside", "[spatial-math]")
{
    const OrientedBox box = Box(Vector3::Zero, Vector3(50, 50, 50));
    CHECK(DistanceTo(box, Vector3(10, 10, 10)) == Approx(0.0f));
    CHECK(DistanceTo(box, Vector3(80, 0, 0)) == Approx(30.0f));
    CHECK(DistanceTo(box, Vector3(80, 90, 0)) == Approx(Vector3(30, 40, 0).Length()));
}

TEST_CASE("environment factor blends linearly outside the box", "[spatial-math]")
{
    const OrientedBox box = Box(Vector3::Zero, Vector3(50, 50, 50));
    CHECK(EnvironmentFactor(box, 100.0f, Vector3(0, 0, 0)) == Approx(1.0f));
    CHECK(EnvironmentFactor(box, 100.0f, Vector3(100, 0, 0)) == Approx(0.5f));
    CHECK(EnvironmentFactor(box, 100.0f, Vector3(150, 0, 0)) == Approx(0.0f));
    CHECK(EnvironmentFactor(box, 100.0f, Vector3(400, 0, 0)) == Approx(0.0f));
}

TEST_CASE("a zero blend distance is a hard edge", "[spatial-math]")
{
    const OrientedBox box = Box(Vector3::Zero, Vector3(50, 50, 50));
    CHECK(EnvironmentFactor(box, 0.0f, Vector3(50, 0, 0)) == Approx(1.0f));
    CHECK(EnvironmentFactor(box, 0.0f, Vector3(51, 0, 0)) == Approx(0.0f));
}

TEST_CASE("the smallest containing room wins", "[spatial-math]")
{
    const RoomCandidate rooms[] = {
        {1, Box(Vector3::Zero, Vector3(500, 500, 500))},
        {2, Box(Vector3::Zero, Vector3(100, 100, 100))},
        {3, Box(Vector3(2000, 0, 0), Vector3(100, 100, 100))},
    };
    CHECK(PickRoom(rooms, 3, Vector3(0, 0, 0)) == 2);
    CHECK(PickRoom(rooms, 3, Vector3(300, 0, 0)) == 1);
    CHECK(PickRoom(rooms, 3, Vector3(2000, 0, 0)) == 3);
    CHECK(PickRoom(rooms, 3, Vector3(9000, 0, 0)) == 0);
}

TEST_CASE("equal volumes keep the earlier room", "[spatial-math]")
{
    const RoomCandidate rooms[] = {
        {7, Box(Vector3::Zero, Vector3(100, 100, 100))},
        {8, Box(Vector3::Zero, Vector3(100, 100, 100))},
    };
    CHECK(PickRoom(rooms, 2, Vector3::Zero) == 7);
    CHECK(PickRoom(nullptr, 0, Vector3::Zero) == 0);
}

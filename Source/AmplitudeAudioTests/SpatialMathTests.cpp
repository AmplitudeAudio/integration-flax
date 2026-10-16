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

TEST_CASE("a box zone matches the oriented box math", "[spatial-math]")
{
    const OrientedBox box = Box(Vector3(100, 0, 0), Vector3(50, 50, 50));
    const ZoneShape zone = ZoneShape::MakeBox(box.Center, box.Rotation, box.HalfExtents);

    for (const Vector3& p : {Vector3(100, 0, 0), Vector3(180, 0, 0), Vector3(100, 90, 0), Vector3(400, 0, 0)})
    {
        CHECK(DistanceTo(zone, p) == Approx(DistanceTo(box, p)));
        CHECK(EnvironmentFactor(zone, 100.0f, p) == Approx(EnvironmentFactor(box, 100.0f, p)));
    }
}

TEST_CASE("the zone kind decides which extents matter", "[spatial-math]")
{
    const ZoneShape box = ZoneShape::MakeBox(Vector3::Zero, Quaternion::Identity, Vector3(10, 20, 30));
    CHECK(box.Kind == ZoneKind::Box);
    CHECK(box.Extents == Vector3(10, 20, 30));
}

TEST_CASE("a sphere zone is distance to the surface", "[spatial-math][zone-sphere]")
{
    const ZoneShape zone = ZoneShape::MakeSphere(Vector3(100, 0, 0), 50.0f);
    CHECK(zone.Kind == ZoneKind::Sphere);

    CHECK(DistanceTo(zone, Vector3(100, 0, 0)) == Approx(0.0f));
    CHECK(DistanceTo(zone, Vector3(150, 0, 0)) == Approx(0.0f)); // on the surface
    CHECK(DistanceTo(zone, Vector3(200, 0, 0)) == Approx(50.0f));
    CHECK(DistanceTo(zone, Vector3(100, 0, 90)) == Approx(40.0f));
}

TEST_CASE("a sphere zone blends linearly outside", "[spatial-math][zone-sphere]")
{
    const ZoneShape zone = ZoneShape::MakeSphere(Vector3::Zero, 50.0f);
    CHECK(EnvironmentFactor(zone, 100.0f, Vector3(0, 20, 0)) == Approx(1.0f));
    CHECK(EnvironmentFactor(zone, 100.0f, Vector3(100, 0, 0)) == Approx(0.5f));
    CHECK(EnvironmentFactor(zone, 100.0f, Vector3(150, 0, 0)) == Approx(0.0f));
    CHECK(EnvironmentFactor(zone, 0.0f, Vector3(51, 0, 0)) == Approx(0.0f));
}

TEST_CASE("a sphere zone ignores rotation", "[spatial-math][zone-sphere]")
{
    ZoneShape zone = ZoneShape::MakeSphere(Vector3::Zero, 50.0f);
    const float before = DistanceTo(zone, Vector3(120, 30, -40));
    zone.Rotation = Quaternion::Euler(33, 77, 12);
    CHECK(DistanceTo(zone, Vector3(120, 30, -40)) == Approx(before));
}

TEST_CASE("a zero radius sphere is a point", "[spatial-math][zone-sphere]")
{
    const ZoneShape zone = ZoneShape::MakeSphere(Vector3::Zero, 0.0f);
    CHECK(DistanceTo(zone, Vector3::Zero) == Approx(0.0f));
    CHECK(DistanceTo(zone, Vector3(10, 0, 0)) == Approx(10.0f));
}

TEST_CASE("an upright capsule measures to its axis segment", "[spatial-math][zone-capsule]")
{
    // radius 50, half height 150 -> the segment runs from y = -100 to y = +100.
    const ZoneShape zone = ZoneShape::MakeCapsule(Vector3::Zero, Quaternion::Identity, 50.0f, 150.0f);
    CHECK(zone.Kind == ZoneKind::Capsule);

    CHECK(DistanceTo(zone, Vector3(0, 0, 0)) == Approx(0.0f));
    CHECK(DistanceTo(zone, Vector3(40, 60, 0)) == Approx(0.0f));    // inside the cylinder
    CHECK(DistanceTo(zone, Vector3(0, 150, 0)) == Approx(0.0f));    // tip of the cap
    CHECK(DistanceTo(zone, Vector3(0, 200, 0)) == Approx(50.0f));   // above the cap
    CHECK(DistanceTo(zone, Vector3(80, 0, 0)) == Approx(30.0f));    // beside the cylinder
    CHECK(DistanceTo(zone, Vector3(0, -250, 0)) == Approx(100.0f)); // below the other cap
}

TEST_CASE("a capsule corner is measured to the cap, not the box", "[spatial-math][zone-capsule]")
{
    const ZoneShape zone = ZoneShape::MakeCapsule(Vector3::Zero, Quaternion::Identity, 50.0f, 150.0f);
    // Cap center at (0,100,0): the point (30,140,0) is sqrt(30^2+40^2) = 50 away from it -> on the surface.
    CHECK(DistanceTo(zone, Vector3(30, 140, 0)) == Approx(0.0f).margin(0.001f));
    CHECK(DistanceTo(zone, Vector3(60, 160, 0)) == Approx(Vector3(60, 60, 0).Length() - 50.0f));
}

TEST_CASE("a capsule follows the actor's local Y axis", "[spatial-math][zone-capsule]")
{
    const Quaternion rotation = Quaternion::Euler(0, 0, 90); // local Y -> world +/-X
    const ZoneShape zone = ZoneShape::MakeCapsule(Vector3::Zero, rotation, 50.0f, 150.0f);
    const Vector3 axis = rotation * Vector3::Up;

    CHECK(DistanceTo(zone, axis * 200.0f) == Approx(50.0f));
    CHECK(DistanceTo(zone, Vector3::Up * 200.0f) == Approx(150.0f));
}

TEST_CASE("a capsule shorter than its radius is a sphere", "[spatial-math][zone-capsule]")
{
    const ZoneShape zone = ZoneShape::MakeCapsule(Vector3::Zero, Quaternion::Identity, 50.0f, 10.0f);
    CHECK(DistanceTo(zone, Vector3(0, 80, 0)) == Approx(30.0f));
    CHECK(DistanceTo(zone, Vector3(80, 0, 0)) == Approx(30.0f));
}

TEST_CASE("a capsule zone blends linearly outside", "[spatial-math][zone-capsule]")
{
    const ZoneShape zone = ZoneShape::MakeCapsule(Vector3::Zero, Quaternion::Identity, 50.0f, 150.0f);
    CHECK(EnvironmentFactor(zone, 100.0f, Vector3(0, 0, 0)) == Approx(1.0f));
    CHECK(EnvironmentFactor(zone, 100.0f, Vector3(100, 0, 0)) == Approx(0.5f));
    CHECK(EnvironmentFactor(zone, 100.0f, Vector3(0, 300, 0)) == Approx(0.0f));
}

TEST_CASE("the capsule draw rotation points the drawn axis along the actor's up", "[spatial-math][zone-capsule]")
{
    for (const Quaternion& actor : {Quaternion::Identity, Quaternion::Euler(0, 90, 0), Quaternion::Euler(30, 45, 60)})
    {
        const Vector3 drawn = CapsuleDrawRotation(actor) * Vector3::Forward; // DrawWireCapsule's axis is local Z
        const Vector3 up = actor * Vector3::Up;
        CHECK(Vector3::Distance(drawn, up) == Approx(0.0f).margin(0.001f));
    }
}

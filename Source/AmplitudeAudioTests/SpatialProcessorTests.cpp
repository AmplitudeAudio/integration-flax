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

#include <AmplitudeAudio/Runtime/Spatial/AmplitudeSpatialProcessor.h>

using AmplitudeAudio::AmplitudeSpatialProcessor;
using AmplitudeAudio::EnvironmentCandidate;
using AmplitudeAudio::ISpatialTarget;
using AmplitudeAudio::Spatial::OrientedBox;
using AmplitudeAudio::Spatial::RoomCandidate;
using AmplitudeAudio::Spatial::ZoneShape;

namespace
{
    struct FakeTarget : ISpatialTarget
    {
        Vector3 Location = Vector3::Zero;
        uint64 Room = 0;
        Dictionary<uint64, float> Factors;
        int RoomWrites = 0;
        int FactorWrites = 0;

        Vector3 GetSpatialLocation() const override { return Location; }
        void ApplyRoom(uint64 id) override { Room = id; RoomWrites++; }
        void ApplyEnvironmentFactor(uint64 id, float factor) override { Factors[id] = factor; FactorWrites++; }
    };

    OrientedBox Box(Vector3 center, float half)
    {
        OrientedBox box;
        box.Center = center;
        box.HalfExtents = Vector3(half);
        return box;
    }

    ZoneShape Zone(Vector3 center, float half)
    {
        return ZoneShape::MakeBox(center, Quaternion::Identity, Vector3(half));
    }

    Array<RoomCandidate> Rooms(std::initializer_list<RoomCandidate> list)
    {
        Array<RoomCandidate> rooms;
        for (const auto& r : list)
            rooms.Add(r);
        return rooms;
    }

    Array<EnvironmentCandidate> Envs(std::initializer_list<EnvironmentCandidate> list)
    {
        Array<EnvironmentCandidate> envs;
        for (const auto& e : list)
            envs.Add(e);
        return envs;
    }
} // namespace

TEST_CASE("a target gets the room it stands in", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.SetWorld(Rooms({{5, Box(Vector3::Zero, 100)}}), Envs({}));

    processor.Update(true, true);
    CHECK(target.Room == 5);
}

TEST_CASE("leaving a room clears it", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.SetWorld(Rooms({{5, Box(Vector3::Zero, 100)}}), Envs({}));
    processor.Update(true, true);

    target.Location = Vector3(1000, 0, 0);
    processor.Update(true, true);
    CHECK(target.Room == 0);
}

TEST_CASE("environment factors follow the target", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.SetWorld(Rooms({}), Envs({{9, Zone(Vector3::Zero, 100), 100.0f}}));

    processor.Update(true, true);
    CHECK(target.Factors[9] == Approx(1.0f));

    target.Location = Vector3(150, 0, 0);
    processor.Update(true, true);
    CHECK(target.Factors[9] == Approx(0.5f));
}

TEST_CASE("an idle target costs nothing", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.SetWorld(Rooms({{5, Box(Vector3::Zero, 100)}}), Envs({{9, Zone(Vector3::Zero, 100), 0.0f}}));
    processor.Update(true, true);

    const int roomWrites = target.RoomWrites;
    const int factorWrites = target.FactorWrites;
    processor.Update(true, true);

    CHECK(processor.GetLastUpdatedCount() == 0);
    CHECK(target.RoomWrites == roomWrites);
    CHECK(target.FactorWrites == factorWrites);
}

TEST_CASE("a world change recomputes everyone once", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget a, b;
    processor.AddTarget(&a);
    processor.AddTarget(&b);
    processor.SetWorld(Rooms({}), Envs({}));
    processor.Update(true, true);

    processor.SetWorld(Rooms({{3, Box(Vector3::Zero, 100)}}), Envs({}));
    processor.Update(true, true);
    CHECK(processor.GetLastUpdatedCount() == 2);
    CHECK(a.Room == 3);
    CHECK(b.Room == 3);

    processor.Update(true, true);
    CHECK(processor.GetLastUpdatedCount() == 0);
}

TEST_CASE("a removed environment is zeroed once", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.SetWorld(Rooms({}), Envs({{9, Zone(Vector3::Zero, 100), 0.0f}}));
    processor.Update(true, true);
    REQUIRE(target.Factors[9] == Approx(1.0f));

    processor.SetWorld(Rooms({}), Envs({}));
    processor.Update(true, true);
    CHECK(target.Factors[9] == Approx(0.0f));

    const int writes = target.FactorWrites;
    processor.SetWorld(Rooms({}), Envs({}));
    processor.Update(true, true);
    CHECK(target.FactorWrites == writes);
}

TEST_CASE("a disabled part applies nothing and applies later when enabled", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.SetWorld(Rooms({{5, Box(Vector3::Zero, 100)}}), Envs({{9, Zone(Vector3::Zero, 100), 0.0f}}));

    processor.Update(false, false);
    CHECK(target.RoomWrites == 0);
    CHECK(target.FactorWrites == 0);

    processor.Update(true, false);
    CHECK(target.Room == 5);
    CHECK(target.FactorWrites == 0);

    processor.Update(true, true);
    CHECK(target.Factors[9] == Approx(1.0f));
}

TEST_CASE("a removed target is never touched again", "[spatial-processor]")
{
    AmplitudeSpatialProcessor processor;
    FakeTarget target;
    processor.AddTarget(&target);
    processor.RemoveTarget(&target);
    processor.SetWorld(Rooms({{5, Box(Vector3::Zero, 100)}}), Envs({}));

    processor.Update(true, true);
    CHECK(target.RoomWrites == 0);
    CHECK(processor.GetTargetCount() == 0);
}

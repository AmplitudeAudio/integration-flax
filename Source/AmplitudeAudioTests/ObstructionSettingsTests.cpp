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

#include <Engine/Serialization/JsonWriters.h>
#include <Engine/Serialization/Serialization.h>

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

namespace
{
    AmplitudeObstructionSettings Parse(const char* json)
    {
        rapidjson_flax::Document document;
        document.Parse(json);
        REQUIRE_FALSE(document.HasParseError());

        AmplitudeObstructionSettings settings;
        settings.Deserialize(document, nullptr);
        return settings;
    }
} // namespace

// The editor saves this asset from C#, which writes LayersMask as an object. The native reader used to assert on it
// and crash the editor on every start.
TEST_CASE("obstruction settings read the C# object form of the layers mask", "[obstruction-settings]")
{
    const AmplitudeObstructionSettings settings = Parse(R"({"DefaultAmount":0.5,"Layers":{"Mask":6}})");
    CHECK(settings.Layers.Mask == 6u);
    CHECK(settings.DefaultAmount == Approx(0.5f));
}

TEST_CASE("obstruction settings still read a bare number layers mask", "[obstruction-settings]")
{
    const AmplitudeObstructionSettings settings = Parse(R"({"Layers":12})");
    CHECK(settings.Layers.Mask == 12u);
}

TEST_CASE("missing or malformed layers keep the default mask", "[obstruction-settings]")
{
    CHECK(Parse(R"({})").Layers.Mask == MAX_uint32);
    CHECK(Parse(R"({"Layers":"all"})").Layers.Mask == MAX_uint32);
    CHECK(Parse(R"({"Layers":{}})").Layers.Mask == MAX_uint32);
}

TEST_CASE("the asset the editor wrote when it crashed loads", "[obstruction-settings]")
{
    const AmplitudeObstructionSettings settings = Parse(R"({
        "DefaultAmount": 0.0, "MaterialAmounts": null, "TagAmounts": null, "MaxDistance": 0.0,
        "FullObstructionDistance": 0.0, "RayBudgetPerFrame": 0, "IndirectRayCount": 0, "IndirectRadius": 0.0,
        "RandomSeed": 0, "AttackSmoothing": 0.0, "ReleaseSmoothing": 0.0, "Layers": { "Mask": 0 }, "DebugDraw": false })");
    CHECK(settings.Layers.Mask == 0u);
    CHECK(settings.RayBudgetPerFrame == 0);
}

TEST_CASE("obstruction settings round trip through the native writer", "[obstruction-settings]")
{
    AmplitudeObstructionSettings original;
    original.DefaultAmount = 0.25f;
    original.RayBudgetPerFrame = 7;
    original.Layers.Mask = 5;

    rapidjson_flax::StringBuffer buffer;
    CompactJsonWriter writer(buffer);
    writer.StartObject();
    AmplitudeObstructionSettings defaults;
    original.Serialize(writer, &defaults);
    writer.EndObject();

    const AmplitudeObstructionSettings loaded = Parse(buffer.GetString());
    CHECK(loaded.DefaultAmount == Approx(0.25f));
    CHECK(loaded.RayBudgetPerFrame == 7);
    CHECK(loaded.Layers.Mask == 5u);
}

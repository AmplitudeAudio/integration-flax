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

#include <Engine/Level/Tags.h>
#include <Engine/Physics/PhysicalMaterial.h>
#include <Engine/Scripting/ScriptingObject.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Runtime/Occlusion/ObstructionSurfaces.h>
#include <AmplitudeAudio/Scripts/AmplitudeObstruction.h>

using AmplitudeAudio::ObstructionSurfaces;

namespace
{
    AmplitudeObstructionSettings SettingsWithTags()
    {
        AmplitudeObstructionSettings settings;
        settings.DefaultAmount = 0.9f;

        AmplitudeObstructionTagAmount surface;
        surface.SurfaceTag = Tags::Get(TEXT("Surface"));
        surface.Amount = 0.5f;
        settings.TagAmounts.Add(surface);

        AmplitudeObstructionTagAmount wood;
        wood.SurfaceTag = Tags::Get(TEXT("Surface.Wood"));
        wood.Amount = 0.3f;
        settings.TagAmounts.Add(wood);
        return settings;
    }

    struct MaterialGuard
    {
        PhysicalMaterial* Material = ScriptingObject::NewObject<PhysicalMaterial>();
        ~MaterialGuard() { Material->DeleteObjectNow(); }
    };
} // namespace

TEST_CASE("an unknown surface gets the default amount", "[obstruction-surfaces]")
{
    ObstructionSurfaces surfaces;
    surfaces.Build(SettingsWithTags());

    CHECK(surfaces.Resolve(nullptr, nullptr) == Approx(0.9f));

    MaterialGuard guard;
    CHECK(surfaces.Resolve(nullptr, guard.Material) == Approx(0.9f));
}

TEST_CASE("a tag is matched exactly, then up its hierarchy", "[obstruction-surfaces]")
{
    ObstructionSurfaces surfaces;
    surfaces.Build(SettingsWithTags());
    MaterialGuard guard;

    guard.Material->Tag = Tags::Get(TEXT("Surface.Wood"));
    CHECK(surfaces.Resolve(nullptr, guard.Material) == Approx(0.3f));

    guard.Material->Tag = Tags::Get(TEXT("Surface.Wood.Thin"));
    CHECK(surfaces.Resolve(nullptr, guard.Material) == Approx(0.3f));

    guard.Material->Tag = Tags::Get(TEXT("Surface.Metal.Heavy"));
    CHECK(surfaces.Resolve(nullptr, guard.Material) == Approx(0.5f));

    guard.Material->Tag = Tags::Get(TEXT("Unrelated"));
    CHECK(surfaces.Resolve(nullptr, guard.Material) == Approx(0.9f));
}

TEST_CASE("a script override beats everything", "[obstruction-surfaces]")
{
    ObstructionSurfaces surfaces;
    surfaces.Build(SettingsWithTags());
    MaterialGuard guard;
    guard.Material->Tag = Tags::Get(TEXT("Surface.Wood"));

    auto* script = ScriptingObject::NewObject<AmplitudeObstruction>();
    script->Amount = 0.05f;
    CHECK(surfaces.Resolve(script, guard.Material) == Approx(0.05f));
    script->DeleteObjectNow();
}

TEST_CASE("amounts are clamped to the unit range", "[obstruction-surfaces]")
{
    AmplitudeObstructionSettings settings;
    settings.DefaultAmount = 4.0f;

    ObstructionSurfaces surfaces;
    surfaces.Build(settings);
    CHECK(surfaces.Resolve(nullptr, nullptr) == Approx(1.0f));
}

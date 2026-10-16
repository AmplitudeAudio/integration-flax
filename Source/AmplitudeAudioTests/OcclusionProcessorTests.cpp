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

#include <Engine/Scripting/ScriptingObject.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Runtime/Occlusion/AmplitudeOcclusionProcessor.h>

using AmplitudeAudio::AmplitudeOcclusionProcessor;
using AmplitudeAudio::IOcclusionSource;

namespace
{
    struct FakeSource : IOcclusionSource
    {
        AmplitudeObstructionMode Mode = AmplitudeObstructionMode::SingleRay;
        bool Active = true;
        Vector3 Location = Vector3(100, 0, 0);
        float Obstruction = 0.0f;
        float Occlusion = 0.0f;
        int Applied = 0;

        AmplitudeObstructionMode GetObstructionMode() const override { return Mode; }
        bool IsOcclusionActive() const override { return Active; }
        Vector3 GetOcclusionLocation() const override { return Location; }
        const Actor* GetOcclusionActor() const override { return nullptr; }
        uint32 GetOcclusionLayers() const override { return MAX_uint32; }
        void ApplyOcclusion(float obstruction, float occlusion) override
        {
            Obstruction = obstruction;
            Occlusion = occlusion;
            Applied++;
        }
    };

    AmplitudeObstructionSettings Instant()
    {
        AmplitudeObstructionSettings settings;
        settings.AttackSmoothing = 0.0f;
        settings.ReleaseSmoothing = 0.0f;
        settings.RayBudgetPerFrame = 100;
        settings.IndirectRayCount = 4;
        settings.MaxDistance = 5000.0f;
        return settings;
    }

    // Everything between the source and the origin is solid; paths to anywhere else are open.
    float DirectBlockedOnly(IOcclusionSource&, const Vector3&, const Vector3& to)
    {
        return to.IsZero() ? 0.8f : 0.0f;
    }

    float AlwaysBlocked(IOcclusionSource&, const Vector3&, const Vector3&)
    {
        return 0.8f;
    }
} // namespace

TEST_CASE("a blocked direct path with an open detour obstructs", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    source.Mode = AmplitudeObstructionMode::MultipleRays;
    processor.AddSource(&source);

    for (int i = 0; i < 8; i++)
        processor.Update(Vector3::Zero, 0.016f, Instant(), DirectBlockedOnly);

    CHECK(source.Obstruction == Approx(0.8f));
    CHECK(source.Occlusion == Approx(0.0f));
}

TEST_CASE("everything blocked occludes", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    source.Mode = AmplitudeObstructionMode::MultipleRays;
    processor.AddSource(&source);

    for (int i = 0; i < 8; i++)
        processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);

    CHECK(source.Obstruction == Approx(0.0f));
    CHECK(source.Occlusion == Approx(0.8f));
}

TEST_CASE("single ray mode costs one ray and never occludes", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    processor.AddSource(&source);

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);

    CHECK(processor.GetLastRayCount() == 1);
    CHECK(source.Obstruction == Approx(0.8f));
    CHECK(source.Occlusion == Approx(0.0f));
}

TEST_CASE("multiple rays mode costs two rays per update", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    source.Mode = AmplitudeObstructionMode::MultipleRays;
    processor.AddSource(&source);

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    CHECK(processor.GetLastRayCount() == 2);
}

TEST_CASE("inactive, disabled and distant sources cost nothing", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource inactiveSource, noneSource, farSource;
    inactiveSource.Active = false;
    noneSource.Mode = AmplitudeObstructionMode::None;
    farSource.Location = Vector3(99999, 0, 0);
    processor.AddSource(&inactiveSource);
    processor.AddSource(&noneSource);
    processor.AddSource(&farSource);

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    CHECK(processor.GetLastRayCount() == 0);
}

TEST_CASE("the ray budget is respected and nobody starves", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource a, b, c;
    a.Location = Vector3(100, 0, 0);
    b.Location = Vector3(200, 0, 0);
    c.Location = Vector3(300, 0, 0);
    processor.AddSource(&a);
    processor.AddSource(&b);
    processor.AddSource(&c);

    AmplitudeObstructionSettings settings = Instant();
    settings.RayBudgetPerFrame = 2;

    processor.Update(Vector3::Zero, 0.016f, settings, AlwaysBlocked);
    CHECK(processor.GetLastRayCount() == 2);

    processor.Update(Vector3::Zero, 0.016f, settings, AlwaysBlocked);
    CHECK(c.Applied > 0); // the one skipped in frame one is served in frame two
}

TEST_CASE("smoothing uses attack and release between casts", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    processor.AddSource(&source);

    AmplitudeObstructionSettings settings = Instant();
    settings.AttackSmoothing = 0.5f;
    settings.ReleaseSmoothing = 0.5f;

    processor.Update(Vector3::Zero, 0.1f, settings, AlwaysBlocked);
    CHECK(source.Obstruction > 0.0f);
    CHECK(source.Obstruction < 0.8f);
}

TEST_CASE("a source that goes inactive is released to zero", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    processor.AddSource(&source);

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    REQUIRE(source.Obstruction > 0.0f);

    source.Active = false;
    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    CHECK(source.Obstruction == Approx(0.0f));
}

TEST_CASE("unchanged values are not rewritten", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    processor.AddSource(&source);

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    const int applied = source.Applied;
    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    CHECK(source.Applied == applied);
}

TEST_CASE("a removed source is never touched again", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    processor.AddSource(&source);
    processor.RemoveSource(&source);

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    CHECK(source.Applied == 0);
    CHECK(processor.GetSourceCount() == 0);
}

TEST_CASE("the ray observer sees every cast", "[occlusion-processor]")
{
    AmplitudeOcclusionProcessor processor;
    FakeSource source;
    source.Mode = AmplitudeObstructionMode::MultipleRays;
    processor.AddSource(&source);

    int direct = 0, indirect = 0;
    processor.SetRayObserver([&](const Vector3&, const Vector3&, float, bool isDirect) { (isDirect ? direct : indirect)++; });

    processor.Update(Vector3::Zero, 0.016f, Instant(), AlwaysBlocked);
    CHECK(direct == 1);
    CHECK(indirect == 1);
}

TEST_CASE("an entity exposes its own hierarchy and layers to the caster", "[occlusion-processor]")
{
    auto* entity = ScriptingObject::NewObject<AmplitudeEntity>();
    entity->ObstructionLayers.Mask = 6;
    const IOcclusionSource* source = entity;
    CHECK(source->GetOcclusionActor() == entity);
    CHECK(source->GetOcclusionLayers() == 6u);
    entity->DeleteObjectNow();
}

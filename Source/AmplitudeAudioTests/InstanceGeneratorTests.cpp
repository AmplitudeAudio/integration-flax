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

#include <Engine/Level/Actors/Spline.h>
#include <Engine/Scripting/ScriptingObject.h>

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Runtime/Spatial/PolylineSampling.h>
#include <AmplitudeAudio/Scripts/AmplitudeInstancedEmitter.h>
#include <AmplitudeAudio/Scripts/AmplitudePointInstanceGenerator.h>
#include <AmplitudeAudio/Scripts/AmplitudeSplineInstanceGenerator.h>

using AmplitudeAudio::Spatial::ResampleByArcLength;

TEST_CASE("resampling places points at equal distances", "[generators]")
{
    Array<Vector3> line;
    line.Add(Vector3(0, 0, 0));
    line.Add(Vector3(1000, 0, 0));

    const Array<Vector3> points = ResampleByArcLength(line, 250.0f, false);
    REQUIRE(points.Count() == 5);
    for (int i = 0; i < 5; i++)
        CHECK(points[i].X == Approx(i * 250.0f));
}

TEST_CASE("resampling follows corners by arc length", "[generators]")
{
    Array<Vector3> line;
    line.Add(Vector3(0, 0, 0));
    line.Add(Vector3(100, 0, 0));
    line.Add(Vector3(100, 0, 100));

    const Array<Vector3> points = ResampleByArcLength(line, 100.0f, false);
    REQUIRE(points.Count() == 3);
    CHECK(points[1].X == Approx(100.0f));
    CHECK(points[1].Z == Approx(0.0f));
    CHECK(points[2].Z == Approx(100.0f));
}

TEST_CASE("a closed polyline wraps without duplicating the start", "[generators]")
{
    Array<Vector3> square;
    square.Add(Vector3(0, 0, 0));
    square.Add(Vector3(100, 0, 0));
    square.Add(Vector3(100, 0, 100));
    square.Add(Vector3(0, 0, 100));

    const Array<Vector3> points = ResampleByArcLength(square, 100.0f, true);
    CHECK(points.Count() == 4);
}

TEST_CASE("degenerate input is handled", "[generators]")
{
    Array<Vector3> one;
    one.Add(Vector3(5, 5, 5));
    CHECK(ResampleByArcLength(one, 100.0f, false).Count() == 1);
    CHECK(ResampleByArcLength(Array<Vector3>(), 100.0f, false).IsEmpty());
    CHECK(ResampleByArcLength(one, 0.0f, false).Count() == 1);
}

TEST_CASE("the point generator creates, tracks and clears its instances", "[generators]")
{
    auto* actor = ScriptingObject::NewObject<Actor>();
    auto* emitterActor = ScriptingObject::NewObject<Actor>();
    auto* emitter = ScriptingObject::NewObject<AmplitudeInstancedEmitter>();
    emitter->SetParent(emitterActor);

    auto* manual = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    emitter->Instances.Add(manual);

    auto* generator = ScriptingObject::NewObject<AmplitudePointInstanceGenerator>();
    generator->SetParent(actor);
    generator->Emitter = emitter;
    generator->Points.Add(Vector3(0, 0, 0));
    generator->Points.Add(Vector3(100, 0, 0));
    generator->Points.Add(Vector3(200, 0, 0));

    generator->Generate();
    CHECK(emitter->Instances.Count() == 4);
    CHECK(actor->GetChildren<AmplitudeSoundInstance>().Count() == 3);

    generator->Generate(); // regenerating replaces, it does not accumulate
    CHECK(emitter->Instances.Count() == 4);
    CHECK(actor->GetChildren<AmplitudeSoundInstance>().Count() == 3);

    generator->Clear();
    CHECK(emitter->Instances.Count() == 1);
    CHECK(emitter->Instances[0].Get() == manual);

    manual->DeleteObjectNow();
    emitterActor->DeleteObjectNow();
    actor->DeleteObjectNow();
}

TEST_CASE("the point generator places instances in the actor's local space", "[generators]")
{
    auto* actor = ScriptingObject::NewObject<Actor>();
    actor->SetPosition(Vector3(1000, 0, 0));
    auto* emitterActor = ScriptingObject::NewObject<Actor>();
    auto* emitter = ScriptingObject::NewObject<AmplitudeInstancedEmitter>();
    emitter->SetParent(emitterActor);
    auto* generator = ScriptingObject::NewObject<AmplitudePointInstanceGenerator>();
    generator->SetParent(actor);
    generator->Emitter = emitter;
    generator->Points.Add(Vector3(50, 0, 0));

    generator->Generate();
    REQUIRE(emitter->Instances.Count() == 1);
    CHECK(emitter->Instances[0]->GetPosition().X == Approx(1050.0f));

    generator->Clear();
    emitterActor->DeleteObjectNow();
    actor->DeleteObjectNow();
}

TEST_CASE("a generator without an emitter does nothing", "[generators]")
{
    auto* actor = ScriptingObject::NewObject<Actor>();
    auto* generator = ScriptingObject::NewObject<AmplitudePointInstanceGenerator>();
    generator->SetParent(actor);
    generator->Points.Add(Vector3::Zero);

    generator->Generate();
    CHECK(actor->GetChildren<AmplitudeSoundInstance>().IsEmpty());
    actor->DeleteObjectNow();
}

TEST_CASE("the spline generator spaces instances along the spline", "[generators]")
{
    auto* spline = ScriptingObject::NewObject<Spline>();
    spline->AddSplinePoint(Vector3(0, 0, 0), false);
    spline->AddSplinePoint(Vector3(1000, 0, 0), false);
    spline->UpdateSpline();

    auto* emitterActor = ScriptingObject::NewObject<Actor>();
    auto* emitter = ScriptingObject::NewObject<AmplitudeInstancedEmitter>();
    emitter->SetParent(emitterActor);

    auto* generator = ScriptingObject::NewObject<AmplitudeSplineInstanceGenerator>();
    generator->SetParent(spline);
    generator->Emitter = emitter;
    generator->Spacing = 250.0f;

    generator->Generate();
    CHECK(emitter->Instances.Count() == 5);

    generator->Clear();
    CHECK(emitter->Instances.IsEmpty());
    emitterActor->DeleteObjectNow();
    spline->DeleteObjectNow();
}

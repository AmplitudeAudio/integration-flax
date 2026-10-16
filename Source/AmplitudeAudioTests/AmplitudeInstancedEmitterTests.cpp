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
#include <AmplitudeAudio/Actors/AmplitudeSoundInstance.h>
#include <AmplitudeAudio/Amplitude.h>
#include <AmplitudeAudio/Scripts/AmplitudeInstancedEmitter.h>

using AmplitudeAudio::AmplitudeService;
using AmplitudeAudioTests::MakeProfile;
using AmplitudeAudioTests::ServiceGuard;

namespace
{
    struct TestScene
    {
        ServiceGuard Guard;
        AmplitudeEntity* Entity = nullptr;
        AmplitudeInstancedEmitter* Emitter = nullptr;
        Array<AmplitudeSoundInstance*> Instances;

        explicit TestScene(int instanceCount)
        {
            String root, buildPath;
            REQUIRE_TEST_PROJECT(root, buildPath);
            REQUIRE(AmplitudeService::Get().Startup(MakeProfile(TEXT("pc.config.amconfig"), {TEXT("init.ambank")}), root, buildPath));
            REQUIRE(AmplitudeService::Get().LoadSoundBank(TEXT("sample_01.ambank"), true));

            Entity = ScriptingObject::NewObject<AmplitudeEntity>();
            Entity->OnBeginPlay();
            Emitter = ScriptingObject::NewObject<AmplitudeInstancedEmitter>();
            Emitter->SetParent(Entity);
            Emitter->SoundObject.ID = 201;

            for (int i = 0; i < instanceCount; i++)
            {
                auto* instance = ScriptingObject::NewObject<AmplitudeSoundInstance>();
                instance->SetPosition(Vector3(i * 100.0f, 0, 0));
                instance->OnBeginPlay();
                Instances.Add(instance);
                Emitter->Instances.Add(instance);
            }
        }

        ~TestScene()
        {
            Amplitude::StopAll();
            for (auto* instance : Instances)
            {
                instance->OnEndPlay();
                instance->DeleteObjectNow();
            }
            Entity->OnEndPlay();
            Entity->DeleteObjectNow();
        }
    };
} // namespace

TEST_CASE("play binds every enabled instance", "[instanced-emitter]")
{
    TestScene scene(3);
    scene.Emitter->Play();

    CHECK(scene.Emitter->GetIsPlaying());
    CHECK(scene.Emitter->GetBoundInstanceCount() == 3);
    for (auto* instance : scene.Instances)
        CHECK(instance->IsBound());
}

TEST_CASE("instances added after play are picked up on the next update", "[instanced-emitter]")
{
    TestScene scene(1);
    scene.Emitter->Play();
    REQUIRE(scene.Emitter->GetBoundInstanceCount() == 1);

    auto* extra = ScriptingObject::NewObject<AmplitudeSoundInstance>();
    extra->OnBeginPlay();
    scene.Emitter->Instances.Add(extra);
    scene.Emitter->OnUpdate();
    CHECK(scene.Emitter->GetBoundInstanceCount() == 2);
    CHECK(extra->IsBound());

    scene.Emitter->Instances.Remove(extra);
    scene.Emitter->OnDisable();
    extra->OnEndPlay();
    extra->DeleteObjectNow();
}

TEST_CASE("removing an instance from the array unbinds it", "[instanced-emitter]")
{
    TestScene scene(2);
    scene.Emitter->Play();
    AmplitudeSoundInstance* removed = scene.Instances[1];

    scene.Emitter->Instances.RemoveAt(1);
    scene.Emitter->OnUpdate();

    CHECK(scene.Emitter->GetBoundInstanceCount() == 1);
    CHECK_FALSE(removed->IsBound());
}

TEST_CASE("a disabled instance is unbound and a re-enabled one is bound again", "[instanced-emitter]")
{
    TestScene scene(2);
    scene.Emitter->Play();
    AmplitudeSoundInstance* instance = scene.Instances[0];

    instance->SetIsActive(false);
    scene.Emitter->OnUpdate();
    CHECK_FALSE(instance->IsBound());
    CHECK(scene.Emitter->GetBoundInstanceCount() == 1);

    instance->SetIsActive(true);
    scene.Emitter->OnUpdate();
    CHECK(instance->IsBound());
    CHECK(scene.Emitter->GetBoundInstanceCount() == 2);
}

TEST_CASE("stopping releases every instance", "[instanced-emitter]")
{
    TestScene scene(2);
    scene.Emitter->Play();
    scene.Emitter->Stop();
    scene.Emitter->OnUpdate();

    CHECK(scene.Emitter->GetBoundInstanceCount() == 0);
    for (auto* instance : scene.Instances)
        CHECK_FALSE(instance->IsBound());
}

TEST_CASE("a paused emitter keeps its channel and instances", "[instanced-emitter]")
{
    TestScene scene(2);
    scene.Emitter->Play();
    scene.Emitter->Pause();
    scene.Emitter->OnUpdate();

    CHECK(scene.Emitter->GetChannel() != nullptr);
    CHECK(scene.Emitter->GetBoundInstanceCount() == 2);

    scene.Emitter->Resume();
    CHECK(scene.Emitter->GetIsPlaying());
}

TEST_CASE("an instance is never bound to two emitters", "[instanced-emitter]")
{
    TestScene scene(1);
    auto* other = ScriptingObject::NewObject<AmplitudeInstancedEmitter>();
    other->SetParent(scene.Entity);
    other->SoundObject.ID = 201;
    other->Instances.Add(scene.Instances[0]);

    scene.Emitter->Play();
    other->Play();

    CHECK(scene.Emitter->GetBoundInstanceCount() == 1);
    CHECK(other->GetBoundInstanceCount() == 0);

    other->OnDisable();
}

TEST_CASE("an instanced emitter without an entity does not play", "[instanced-emitter]")
{
    ServiceGuard guard;
    auto* actor = ScriptingObject::NewObject<Actor>();
    auto* emitter = ScriptingObject::NewObject<AmplitudeInstancedEmitter>();
    emitter->SetParent(actor);
    emitter->SoundObject.ID = 201;

    emitter->Play();
    CHECK_FALSE(emitter->GetIsPlaying());
    actor->DeleteObjectNow();
}

TEST_CASE("the entity lists an instanced emitter among its emitters", "[instanced-emitter]")
{
    TestScene scene(0);
    scene.Emitter->Name = TEXT("river");
    CHECK(scene.Entity->GetEmitter(TEXT("river")) == static_cast<IAmplitudeEmitter*>(scene.Emitter));
}

TEST_CASE("changing the mode while playing restarts the sound", "[instanced-emitter]")
{
    TestScene scene(2);
    scene.Emitter->Play();
    scene.Emitter->SetMode(AmplitudeInstanceMode::Separate);

    CHECK(scene.Emitter->GetIsPlaying());
    CHECK(scene.Emitter->GetBoundInstanceCount() == 2);
}

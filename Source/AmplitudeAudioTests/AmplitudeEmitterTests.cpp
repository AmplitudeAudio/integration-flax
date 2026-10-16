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

#include <AmplitudeAudio/Scripts/AmplitudeEmitter.h>

#if USE_EDITOR
TEST_CASE("an emitter keeps an editor attenuation shape until cleared", "[emitter]")
{
    auto* emitter = ScriptingObject::NewObject<AmplitudeEmitter>();
    CHECK_FALSE(emitter->HasEditorAttenuation());
    Array<float> inner; inner.Add(964.0f);
    Array<float> outer; outer.Add(1024.0f);
    emitter->SetEditorAttenuation(0, inner, outer);
    CHECK(emitter->HasEditorAttenuation());
    emitter->ClearEditorAttenuation();
    CHECK_FALSE(emitter->HasEditorAttenuation());
    emitter->DeleteObjectNow();
}
#endif

// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     `http://www.apache.org/licenses/LICENSE-2.0`
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "TestFixture.h"

#include <ThirdParty/catch2/catch.hpp>

#include <AmplitudeAudio/Types/AmplitudeRefs.h>

TEST_CASE("references are invalid until an ID is set", "[refs]")
{
    AmplitudeSoundObjectRef ref;
    CHECK_FALSE(ref.IsValid());
    ref.ID = 201;
    CHECK(ref.IsValid());
}

TEST_CASE("references of different kinds are distinct types", "[refs]")
{
    CHECK_FALSE(std::is_same<AmplitudeSoundRef, AmplitudeCollectionRef>::value);
    CHECK_FALSE(std::is_same<AmplitudeEventRef, AmplitudeRtpcRef>::value);
}

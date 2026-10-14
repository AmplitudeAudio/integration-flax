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

#include "AmplitudeSettings.h"

#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Config/GameSettings.h>
#include <Engine/Serialization/Serialization.h>

IMPLEMENT_GAME_SETTINGS_GETTER(AmplitudeSettings, "Amplitude Settings");

AmplitudePlatformProfile AmplitudeSettings::GetProfile(PlatformType platform) const
{
    for (const AmplitudePlatformProfile& profile : Profiles)
    {
        if (profile.Platform == platform)
            return profile;
    }

    return DefaultProfile;
}

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
#include <Engine/Core/Log.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Scripting/Internal/InternalCalls.h>
#include <Engine/Serialization/Serialization.h>

IMPLEMENT_GAME_SETTINGS_GETTER(AmplitudeSettings, "Amplitude Configuration");

AmplitudeSettings::AmplitudeSettings()
{
}

void AmplitudeSettings::Apply()
{
    LOG(Info, "[AmplitudeAudio] Configuration settings applied.");
}

DEFINE_INTERNAL_CALL(void)
AmplitudeSettingsInternal_Apply()
{
    auto settings = AmplitudeSettings::Get();
    if (!settings)
        return;

    settings->Apply();
}

namespace AmplitudeAudio
{
    void AmplitudeAssetProfiles::Add(const String& platformName, const AmplitudeConfiguration& config)
    {
        m_profiles[platformName] = config;
    }

    const AmplitudeConfiguration* AmplitudeAssetProfiles::Get(const String& platformName) const
    {
        return m_profiles.TryGet(platformName);
    }

    const String& AmplitudeAssetProfiles::GetProjectBuildPath() const
    {
        return m_projectBuildPath;
    }

    const String& AmplitudeAssetProfiles::GetProjectSourcesPath() const
    {
        return m_projectSourcesPath;
    }

    String AmplitudeAssetProfiles::GetCurrentPlatform()
    {
#if PLATFORM_ANDROID || PLATFORM_IOS
        return TEXT("Mobile");
#elif PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC
        return TEXT("Desktop");
#elif PLATFORM_WEB
        return TEXT("Web");
#else
    return TEXT("Desktop");
#endif
    }

    void AmplitudeAssetProfiles::LoadFromSettings(
        const Array<AmplitudePlatformProfile>& profiles,
        const String&                          projectBuildPath,
        const String&                          projectSourcesPath)
    {
        m_profiles.Clear();
        m_projectBuildPath   = projectBuildPath;
        m_projectSourcesPath = projectSourcesPath;

        for (const auto& profile : profiles)
        {
            m_profiles[profile.PlatformName] = profile.Config;
        }
    }
} // namespace AmplitudeAudio
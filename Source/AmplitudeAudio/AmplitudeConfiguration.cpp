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

// ============================================================================
//  AmplitudeConfiguration — implementation
// ============================================================================

#include "AmplitudeConfiguration.h"

#include <Engine/Platform/Platform.h>
#include <Engine/Serialization/Serialization.h>

// ============================================================================
//  AmplitudeConfigurationAsset — ISerializable
// ============================================================================

void AmplitudeConfigurationAsset::Serialize(SerializeStream& stream, const void* otherObj)
{
    const auto* other = static_cast<const AmplitudeConfigurationAsset*>(otherObj);
    SERIALIZE_MEMBER(Profiles, Profiles);
    SERIALIZE_MEMBER(ProjectBuildPath, ProjectBuildPath);
    SERIALIZE_MEMBER(ProjectSourcesPath, ProjectSourcesPath);
}

void AmplitudeConfigurationAsset::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    DESERIALIZE_MEMBER(Profiles, Profiles);
    DESERIALIZE_MEMBER(ProjectBuildPath, ProjectBuildPath);
    DESERIALIZE_MEMBER(ProjectSourcesPath, ProjectSourcesPath);
}

// ============================================================================
//  AmplitudeAssetProfiles
// ============================================================================

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
#else
        return TEXT("Desktop");
#endif
    }

    void AmplitudeAssetProfiles::LoadFromAsset(
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

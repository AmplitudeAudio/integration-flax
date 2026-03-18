// ============================================================================
//  AmplitudeConfiguration — Per-platform configuration profiles
//  (implementation)
// ============================================================================

#include "AmplitudeConfiguration.h"

#include <Engine/Platform/Platform.h>

namespace AmplitudeAudio
{
    // ========================================================================
    //  AmplitudeAssetProfiles — profile management
    // ========================================================================

    void AmplitudeAssetProfiles::Add(const String& platformName, const AmplitudeConfiguration& config)
    {
        m_profiles[platformName] = config;
    }

    const AmplitudeConfiguration* AmplitudeAssetProfiles::Get(const String& platformName) const
    {
        const AmplitudeConfiguration* result = m_profiles.TryGet(platformName);
        return result;
    }

    // ========================================================================
    //  AmplitudeAssetProfiles — platform detection
    // ========================================================================

    String AmplitudeAssetProfiles::GetCurrentPlatform()
    {
#if PLATFORM_ANDROID || PLATFORM_IOS
        return TEXT("Mobile");
#elif PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC
        return TEXT("Desktop");
#else
        // Fallback to Desktop for any other/unknown platform.
        return TEXT("Desktop");
#endif
    }

    // ========================================================================
    //  AmplitudeAssetProfiles — JSON loading
    // ========================================================================

    void AmplitudeAssetProfiles::LoadFromJson(const JsonTools::Value& node)
    {
        // Clear existing profiles before populating from JSON.
        m_profiles.Clear();

        // Look for the top-level "Profiles" object.
        const auto profilesMember = node.FindMember("Profiles");
        if (profilesMember == node.MemberEnd() || !profilesMember->value.IsObject())
            return;

        const JsonTools::Value& profilesNode = profilesMember->value;

        for (auto it = profilesNode.MemberBegin(); it != profilesNode.MemberEnd(); ++it)
        {
            if (!it->value.IsObject())
                continue;

            // Platform name key (narrow string from JSON, converted to Flax String).
            const String platformName(it->name.GetString());

            AmplitudeConfiguration config;
            config.InitSoundBank   = JsonTools::GetString(it->value, "InitSoundBank");
            config.EngineConfigFile = JsonTools::GetString(it->value, "EngineConfigFile");

            m_profiles[platformName] = config;
        }
    }

} // namespace AmplitudeAudio

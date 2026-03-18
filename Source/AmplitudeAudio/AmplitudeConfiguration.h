#pragma once

// ============================================================================
//  AmplitudeConfiguration — Per-platform configuration profiles
//
//  Defines the AmplitudeConfiguration struct (per-platform settings) and the
//  AmplitudeAssetProfiles class (a named map of those settings).
//
//  Profiles can be loaded from a JSON asset using the LoadFromJson() method.
//  The JSON format expected is:
//
//      {
//          "Profiles": {
//              "Desktop": {
//                  "InitSoundBank": "init.ambank",
//                  "EngineConfigFile": "pc.amconfig"
//              },
//              "Mobile": {
//                  "InitSoundBank": "init.ambank",
//                  "EngineConfigFile": "mobile.amconfig"
//              }
//          }
//      }
// ============================================================================

#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Serialization/JsonTools.h>

namespace AmplitudeAudio
{
    // ========================================================================
    //  AmplitudeConfiguration
    // ========================================================================

    /// \brief Per-platform Amplitude configuration.
    ///
    /// Groups the two filenames that the plugin needs at startup: the initial
    /// sound bank and the Amplitude engine config file.
    struct AmplitudeConfiguration
    {
        /// Filename of the initial sound bank to load at startup.
        /// Example: "init.ambank"
        String InitSoundBank;

        /// Filename of the Amplitude engine config file.
        /// Example: "pc.amconfig"
        String EngineConfigFile;
    };

    // ========================================================================
    //  AmplitudeAssetProfiles
    // ========================================================================

    /// \brief A named map of per-platform AmplitudeConfiguration values.
    ///
    /// Typical usage:
    /// \code
    ///   AmplitudeAssetProfiles profiles;
    ///   profiles.LoadFromJson(jsonValue);
    ///   const AmplitudeConfiguration* cfg = profiles.Get(profiles.GetCurrentPlatform());
    /// \endcode
    class AmplitudeAssetProfiles
    {
    public:
        AmplitudeAssetProfiles() = default;
        ~AmplitudeAssetProfiles() = default;

        // ====================================================================
        //  Profile management
        // ====================================================================

        /// \brief Adds or replaces the configuration for the given platform name.
        ///
        /// \param platformName  The platform key (e.g. "Desktop", "Mobile").
        /// \param config        The configuration to store.
        void Add(const String& platformName, const AmplitudeConfiguration& config);

        /// \brief Returns a pointer to the configuration for the given platform name.
        ///
        /// \param platformName  The platform key to look up.
        /// \return Pointer to the stored configuration, or nullptr if not found.
        [[nodiscard]] const AmplitudeConfiguration* Get(const String& platformName) const;

        // ====================================================================
        //  Platform detection
        // ====================================================================

        /// \brief Returns the canonical platform name for the current build target.
        ///
        /// Mapping:
        ///   - Windows, Linux, macOS -> "Desktop"
        ///   - Android, iOS          -> "Mobile"
        ///   - Other                 -> "Desktop" (fallback)
        [[nodiscard]] static String GetCurrentPlatform();

        // ====================================================================
        //  JSON loading
        // ====================================================================

        /// \brief Populates this object from a rapidjson Value node.
        ///
        /// The node must be the root JSON object containing a "Profiles" member
        /// whose value is an object whose keys are platform names and whose
        /// values have "InitSoundBank" and "EngineConfigFile" string members.
        ///
        /// Any existing profiles are replaced by those found in the JSON.
        /// Unknown fields are silently ignored.
        ///
        /// \param node  The root JSON value to read from.
        void LoadFromJson(const JsonTools::Value& node);

    private:
        Dictionary<String, AmplitudeConfiguration> m_profiles;
    };

} // namespace AmplitudeAudio

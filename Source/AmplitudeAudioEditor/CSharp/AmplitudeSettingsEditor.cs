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

using System;
using AmplitudeAudio;
using FlaxEditor;
using FlaxEditor.Content.Settings;
using FlaxEngine;

namespace AmplitudeAudioEditor
{
    /// <summary>
    /// Editor-side helpers for the native <see cref="AmplitudeSettings"/> game settings.
    /// </summary>
    public static class AmplitudeSettingsEditor
    {
        /// <summary>
        /// Key of the settings in the game settings' custom settings.
        /// </summary>
        public const string SettingsName = "Amplitude Settings";

        /// <summary>
        /// Gets the instance of the settings asset (default value if missing).
        /// </summary>
        public static AmplitudeSettings Get()
        {
            return GameSettings.Load<AmplitudeSettings>();
        }

        /// <summary>
        /// Returns the profile for <paramref name="platform"/>, or the default profile when none matches. Mirrors the C++ AmplitudeSettings::GetProfile.
        /// </summary>
        public static AmplitudePlatformProfile GetProfile(AmplitudeSettings settings, PlatformType platform)
        {
            foreach (var profile in settings.Profiles ?? Array.Empty<AmplitudePlatformProfile>())
            {
                if (profile.Platform == platform)
                    return profile;
            }

            return settings.DefaultProfile;
        }

        /// <summary>
        /// Replaces an all-zero obstruction block with the plugin defaults. Assets created from the editor's
        /// "New" menu (or by older plugin versions) come out of a zeroed C# struct, which would disable obstruction.
        /// </summary>
        public static void RepairObstruction()
        {
            var settings = Get();
            if (settings == null)
                return;

            var o = settings.Obstruction;
            var zeroed = o.DefaultAmount == 0.0f && o.MaxDistance == 0.0f && o.FullObstructionDistance == 0.0f &&
                         o.RayBudgetPerFrame == 0 && o.IndirectRayCount == 0 && o.IndirectRadius == 0.0f &&
                         o.AttackSmoothing == 0.0f && o.ReleaseSmoothing == 0.0f;
            if (!zeroed)
                return;

            var defaults = Amplitude.GetDefaultObstructionSettings();
            // Keep what the user may have already set.
            defaults.MaterialAmounts = o.MaterialAmounts;
            defaults.TagAmounts = o.TagAmounts;
            defaults.DebugDraw = o.DebugDraw;
            settings.Obstruction = defaults;
            Save(settings);
            Debug.Log("[AmplitudeAudio] Obstruction settings were uninitialized; restored the defaults.");
        }

        /// <summary>
        /// Saves the settings to the disk.
        /// </summary>
        public static void Save(AmplitudeSettings settings)
        {
            var path = StringUtils.CombinePaths(Globals.ProjectContentFolder, "Settings", "AmplitudeSettings.json");
            bool saveError = Editor.SaveJsonAsset(path, settings);
            if (saveError)
            {
                Debug.LogError("[AmplitudeAudio] Failed to save AmplitudeSettings to disk.");
                return;
            }
            var asset = FlaxEngine.Content.LoadAsync<JsonAsset>(path);
            GameSettings.SetCustomSettings(SettingsName, asset);
        }
    }
}

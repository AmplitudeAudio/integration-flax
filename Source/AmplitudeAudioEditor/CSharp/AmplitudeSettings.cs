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

using System.Runtime.InteropServices;
using FlaxEditor;
using FlaxEngine;

namespace FlaxEditor.Content.Settings
{
    /// <summary>
    /// Native Flax game settings that store per-platform Amplitude configuration.
    /// Provides per-platform profiles and Amplitude project paths used by the
    /// Amplitude Audio middleware at runtime and by the controls generator in the editor.
    /// </summary>
    public sealed partial class AmplitudeSettings : SettingsBase
    {
        /// <summary>
        /// Gets the instance of the settings asset (default value if missing).
        /// </summary>
        public static AmplitudeSettings Get()
        {
            return GameSettings.Load<AmplitudeSettings>();
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
            GameSettings.SetCustomSettings("Amplitude Audio", asset);
        }

        [LibraryImport("AmplitudeAudio", EntryPoint = "AmplitudeSettingsInternal_Apply")]
        public static partial void Apply();
    }
}

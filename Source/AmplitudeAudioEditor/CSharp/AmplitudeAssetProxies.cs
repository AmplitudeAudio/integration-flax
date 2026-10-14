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

using FlaxEditor;
using FlaxEditor.Content;
using FlaxEngine;
using FlaxEditor.Content.Settings;

namespace AmplitudeAudioEditor
{
    /// <summary>
    /// Handles registration of Amplitude Audio asset proxies with the Flax Editor content database.
    /// </summary>
    public class AmplitudeAssetProxies
    {
        private CustomSettingsProxy _settingsProxy;

        /// <summary>
        /// Adds Amplitude Audio asset proxies to the editor content database.
        /// </summary>
        public void Register()
        {
            _settingsProxy = new CustomSettingsProxy(typeof(AmplitudeSettings), "Amplitude Configuration", Editor.Instance.Icons.AudioSettings128);
            Editor.Instance.ContentDatabase.AddProxy(_settingsProxy);

            Editor.Instance.ContentDatabase.Rebuild(true);

            Debug.Log("[AmplitudeAudio] Asset proxies registered.");
        }

        /// <summary>
        /// Removes Amplitude Audio asset proxies from the editor content database.
        /// </summary>
        public void Unregister()
        {
            if (_settingsProxy != null)
            {
                Editor.Instance.ContentDatabase.RemoveProxy(_settingsProxy);
                _settingsProxy = null;
            }
        }
    }
}

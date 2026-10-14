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
using FlaxEngine;
using FlaxEditor.Content.Settings;

namespace AmplitudeAudioEditor
{
    /// <summary>
    /// C# editor plugin for the Amplitude Audio integration.
    /// Registers the Amplitude Settings asset proxy with the content
    /// database and wires up play-mode event callbacks.
    /// </summary>
    public partial class AmplitudeAudioEditorPlugin : EditorPlugin
    {
        private static bool _initialized;

        private AmplitudeAssetProxies _assetProxies;

        /// <inheritdoc />
        public override void InitializeEditor()
        {
            base.InitializeEditor();

            if (_initialized)
                return;

            _assetProxies = new AmplitudeAssetProxies();
            _assetProxies.Register();

            Editor.PlayModeBegin += OnPlayModeBegin;
            Editor.PlayModeEnd += OnPlayModeEnd;

            var asset = GameSettings.LoadAsset<AmplitudeSettings>();
            if (asset == null)
                AmplitudeSettingsEditor.Save(new AmplitudeSettings());

            _initialized = true;
            Debug.Log("[AmplitudeAudio] C# editor plugin initialized.");
        }

        /// <inheritdoc />
        public override void DeinitializeEditor()
        {
            if (!_initialized)
            {
                base.DeinitializeEditor();
                return;
            }

            Editor.PlayModeBegin -= OnPlayModeBegin;
            Editor.PlayModeEnd -= OnPlayModeEnd;

            _assetProxies?.Unregister();
            _assetProxies = null;

            _initialized = false;
            Debug.Log("[AmplitudeAudio] C# editor plugin deinitialized.");

            base.DeinitializeEditor();
        }

        private void OnPlayModeBegin()
        {
        }

        private void OnPlayModeEnd()
        {
        }
    }
}

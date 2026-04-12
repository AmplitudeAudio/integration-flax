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

using AmplitudeAudio;
using FlaxEditor;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Editors;
using FlaxEngine;
using FlaxEngine.GUI;

namespace AmplitudeAudioEditor
{
    /// <summary>
    /// Custom editor for AudioControlsScript that adds a "Reload Controls"
    /// button. The button scans the Amplitude project directory for audio
    /// control definitions and writes them to the configured asset path.
    /// </summary>
    [CustomEditor(typeof(AudioControlsScript)), DefaultEditor]
    public class AudioControlsScriptEditor : GenericEditor
    {
        /// <inheritdoc />
        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            var group = layout.Group("Amplitude Controls Generator");
            group.Panel.Open();

            // --- Output asset path ---
            group.Label("Output Asset Path");
            var outputPathBox = group.TextBox();
            outputPathBox.TextBox.WatermarkText = "e.g. Content/Sounds/Amplitude/Controls.json";

            // --- Reload button ---
            var button = group.Button("Reload Controls");
            button.Button.Clicked += () =>
            {
                // Resolve the project sources path from the Amplitude config asset.
                var projectPath = AmplitudeControlsGenerator.GetProjectSourcesPath();
                var outputPath = outputPathBox.TextBox.Text;

                if (string.IsNullOrWhiteSpace(projectPath))
                {
                    Debug.LogWarning("[AudioControlsScriptEditor] ProjectSourcesPath is not configured. " +
                                     "Set it in the Amplitude Configuration asset (Config.json).");
                    return;
                }

                if (string.IsNullOrWhiteSpace(outputPath))
                {
                    Debug.LogWarning("[AudioControlsScriptEditor] Output asset path is empty.");
                    return;
                }

                var success = AmplitudeControlsGenerator.ReloadControls(projectPath, outputPath);
                if (success)
                {
                    Debug.Log($"[AudioControlsScriptEditor] Controls reloaded to: {outputPath}");

                    // Auto-fill the component's ControlsAssetPath with the output path.
                    foreach (var value in Values)
                        if (value is AudioControlsScript component)
                            component.ControlsAssetPath = outputPath;
                }
                else
                {
                    Debug.LogError("[AudioControlsScriptEditor] Failed to reload controls.");
                }
            };
        }
    }
}

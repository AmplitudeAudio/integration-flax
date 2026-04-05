using AmplitudeAudio;
using FlaxEditor;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Editors;
using FlaxEngine;
using FlaxEngine.GUI;

namespace AmplitudeAudioEditor
{
    /// <summary>
    /// Custom editor for AudioControlsComponent that adds a "Reload Controls"
    /// button. The button scans the Amplitude project directory for audio
    /// control definitions and writes them to the configured asset path.
    /// </summary>
    [CustomEditor(typeof(AudioControlsComponent)), DefaultEditor]
    public class AudioControlsComponentEditor : GenericEditor
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
                    Debug.LogWarning("[AudioControlsComponentEditor] ProjectSourcesPath is not configured. " +
                                     "Set it in the Amplitude Configuration asset (Config.json).");
                    return;
                }

                if (string.IsNullOrWhiteSpace(outputPath))
                {
                    Debug.LogWarning("[AudioControlsComponentEditor] Output asset path is empty.");
                    return;
                }

                var success = AmplitudeControlsGenerator.ReloadControls(projectPath, outputPath);
                if (success)
                {
                    Debug.Log($"[AudioControlsComponentEditor] Controls reloaded to: {outputPath}");

                    // Auto-fill the component's ControlsAssetPath with the output path.
                    foreach (var value in Values)
                        if (value is AudioControlsComponent component)
                            component.ControlsAssetPath = outputPath;
                }
                else
                {
                    Debug.LogError("[AudioControlsComponentEditor] Failed to reload controls.");
                }
            };
        }
    }
}

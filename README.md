# Amplitude Audio - Flax Engine Integration

Welcome to the official **Amplitude Audio SDK** integration plugin for the [Flax Engine](https://flaxengine.com/). This plugin provides a seamless middleware implementation that hooks Amplitude Audio directly into the custom [AudioSystem Plugin](https://github.com/SparkyStudios/FlaxAudioSystemPlugin) developed by Sparky Studios.

## Requirements

- **Flax Engine**
- **[Amplitude Audio SDK](https://github.com/AmplitudeAudio/sdk)**: You must have the SDK available on your system, and the `AM_SDK_PATH` environment variable must be set pointing to the SDK directory. This is required for the plugin to compile.
- **[AudioSystem Plugin](https://github.com/SparkyStudios/FlaxAudioSystemPlugin)**: This custom plugin must be installed and enabled in your project before using the Amplitude Audio integration.

## Features

- **AudioSystem Middleware Integration**: Implements the custom `AudioSystem` middleware interface, intercepting and processing audio calls through the Amplitude Audio SDK.
- **Flax-Native Subsystems**: Routes Amplitude's internal memory allocation, logging, and file I/O operations through Flax's high-performance native APIs (`FlaxMemoryAllocator`, `FlaxLogger`, and `AmplitudeFileSystem`).
- **Audio Controls Script**: Easily manage your audio at runtime using the `AudioControlsScript`. Attach it to any actor to automatically load/unload `AudioControlCollection` JSON assets based on the actor's lifecycle.
- **Editor Extensions**: Custom editor integrations (like the `AudioControlsScriptEditor`) to quickly reload and manage audio controls directly from the Flax Editor.

## Installation

1. Clone or download this repository into the `Plugins/` directory of your Flax Engine project:
   ```bash
   cd YourFlaxProject
   mkdir -p Plugins
   cd Plugins
   git clone https://github.com/AmplitudeAudio/integration-flax.git AmplitudeAudio
   ```
2. Open your project's `.flaxproj` file and ensure the plugin is referenced.
3. In your game's target build script (e.g., `Source/GameTarget.Build.cs`), add `AmplitudeAudio` to the module dependencies:
   ```csharp
   Modules.Add("AmplitudeAudio");
   ```
4. Regenerate your project's script files and build the project.

## Configuration

The plugin expects an Amplitude configuration file to be present in your project's content directory, which binds the platform profiles and points to your Amplitude project source and build outputs.

1. Create a valid configuration JSON asset at: `Content/Sounds/Amplitude/Config.json`. This can be done via the Flax Editor context menu (`New -> Audio System -> Amplitude -> Configuration`).
2. This asset must contain:
   - **Per-platform Profiles**: E.g., `InitSoundBank` and `EngineConfigFile` entries required to initialize the Amplitude SDK on each platform.
   - **ProjectBuildPath**: The directory where your built `.ambank` and `.amconfig` files are exported (e.g. `"build"`).
   - **ProjectSourcesPath**: The directory containing your Amplitude project source files (e.g. `"sources"`).

### Expected Directory Structure

The plugin operates within the `Content/Sounds/Amplitude/` base path. Below is an example of how your files should be structured if your `ProjectBuildPath` is set to `"build"` and `ProjectSourcesPath` is set to `"sources"`:

```text
YourFlaxProject/
└── Content/
    └── Sounds/
        └── Amplitude/
            ├── Config.json                 <-- Main configuration asset
            ├── MyControls.json             <-- Generated AudioControlCollection
            │
            ├── sources/                    <-- ProjectSourcesPath
            │   ├── .amproject              <-- Amplitude project file (no name)
            │   ├── pc.config.json          <-- Source JSON definitions
            │   ├── buses.json              
            │   ├── events/                 
            │   ├── soundbanks/
            │   └── ...
            │
            └── build/                      <-- ProjectBuildPath
                ├── init.ambank
                ├── desktop.amconfig
                ├── mobile.amconfig
                ├── events/
                ├── soundbanks/
                └── ...
```

At runtime, the middleware uses the `ProjectBuildPath` relative to `Content/Sounds/Amplitude/` to mount the file system and load your banks. In the Editor, the `ProjectSourcesPath` is scanned to regenerate audio controls.

## Usage

### Loading Audio Controls

To load an `AudioControlCollection` at runtime:
1. Create a new Actor in your scene.
2. Add the `AudioControlsScript` to the Actor.
3. In the properties panel, set the `ControlsAssetPath` to the project-relative path of your controls JSON file (e.g., `Content/Sounds/Amplitude/MyControls.json`).
4. Keep `AutoLoad` enabled to have the script automatically register all sound banks, triggers, switches, and RTPCs with the AudioSystem during `OnBeginPlay()`.

### Reloading Controls in Editor

The plugin includes a custom editor extension. When selecting an Actor with an `AudioControlsScript`, you will see a **"Reload Controls"** button. This button scans your Amplitude project directory and writes the updated control definitions to your configured asset path.

## License

This project is licensed under the **Apache License, Version 2.0**.

Copyright (c) 2026-present Sparky Studios. All rights reserved.

See the [LICENSE](LICENSE) file for more information.

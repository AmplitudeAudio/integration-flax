# Amplitude Audio - Flax Engine Integration

Welcome to the official **Amplitude Audio SDK** integration plugin for the [Flax Engine](https://flaxengine.com/). This native plugin starts the Amplitude engine with your game and exposes it through a static C++ API, a sound bank script, and an editor asset for its settings.

## Requirements

- **Flax Engine**
- **[Amplitude Audio SDK](https://github.com/AmplitudeAudio/sdk)**: The SDK must be available on your system, and the `AM_SDK_PATH` environment variable must point to the SDK directory. The SDK must be built with the release C runtime (`/MD`) in every configuration, including debug, because Flax always uses the release CRT.

## Installation

1. Clone or download this repository into the `Plugins/` directory of your Flax Engine project:
   ```bash
   cd YourFlaxProject
   mkdir -p Plugins
   cd Plugins
   git clone https://github.com/AmplitudeAudio/integration-flax.git AmplitudeAudio
   ```
2. Reference `AmplitudeAudio.flaxproj` from your game project's `.flaxproj` file.
3. In your game's target build script (e.g., `Source/GameTarget.Build.cs`), add `AmplitudeAudio` to the module dependencies:
   ```csharp
   Modules.Add("AmplitudeAudio");
   ```
4. Regenerate your project's script files and build the project.

## Setup

### Expected Directory Structure

The plugin works inside the fixed base folder `Content/Sounds/Amplitude/`. Every path in the settings is relative to it: `ProjectBuildPath` and `ProjectSourcesPath` name folders inside it (they may be nested, like `audio/build`, but can't point outside it). Below is the expected layout with `ProjectBuildPath` set to `build` and `ProjectSourcesPath` set to `sources`:

```text
YourFlaxProject/
└── Content/
    ├── Settings/
    │   └── AmplitudeSettings.json         <-- Amplitude Settings asset
    └── Sounds/
        └── Amplitude/                     <-- Base folder (fixed)
            ├── sources/                   <-- ProjectSourcesPath (editor only)
            │   ├── .amproject             <-- Amplitude project file (no name)
            │   ├── pc.config.json         <-- Source JSON definitions
            │   ├── events/
            │   ├── soundbanks/
            │   └── ...
            ├── build/                     <-- ProjectBuildPath
            │   ├── pc.config.amconfig     <-- A profile's EngineConfigFile
            │   ├── pc.buses.ambus
            │   ├── soundbanks/
            │   │   └── init.ambank        <-- Referenced by file name (init.ambank)
            │   ├── events/
            │   └── ...
            └── plugins/                   <-- Optional PluginSearchPaths entry
```

At runtime the engine reads assets from `Content/Sounds/Amplitude/<ProjectBuildPath>/`, or from `Content/Sounds/Amplitude/<ProjectBuildPath>.ampk` when that package exists (cooked games in `Ampk` mode). Plugin search paths always stay loose folders.

### Amplitude Settings asset

The editor creates the **Amplitude Settings** asset (`Content/Settings/AmplitudeSettings.json`) on first launch; you can also create one with `New > Amplitude > Settings`. It holds:

- **Project paths**: `ProjectBuildPath` (the build folder, relative to `Content/Sounds/Amplitude`) and `ProjectSourcesPath` (your Amplitude project sources, used by the editor).
- **Profiles**: a `DefaultProfile` and per-platform `Profiles`. The first profile matching the running platform is used. Each profile defines:
  - `EngineConfigFile`: the engine configuration file, relative to the build folder (for example `pc.config.amconfig`).
  - `InitSoundBanks`: banks loaded at startup, before any scene.
  - `Plugins`: Amplitude plugin libraries loaded before the engine initializes.
  - `PluginSearchPaths`: folders searched for plugin libraries, relative to `Content/Sounds/Amplitude`.
- **Cooking**: `PackagingMode` selects how the build ships with cooked games. `LooseFiles` copies the build folder as is. `Ampk` packages it into a single `.ampk` file with the ampk tool, which is found through `AmpkToolPath`, `$AM_SDK_PATH/bin`, or `PATH`. `Compression` and `BlockSize` configure the package.

## Usage

### Sound bank script

Add the `AmplitudeSoundBank` script to an actor and set its `Bank` to the bank file name. The bank is loaded while the script is enabled and released when it is disabled. Enable `LoadAsync` to load its sound files in the background. `IsLoaded` reports whether the bank and its sound files are loaded.

### Static API

Call the `AmplitudeAudio.Amplitude` static API from C++ or C#:

- `Startup()` and `Shutdown()` start and stop the engine.
- `IsRunning()` reports whether the engine is started.
- `LoadSoundBank(bank, wait)` and `UnloadSoundBank(bank)` load and release a sound bank by reference. `LoadSoundBank` blocks until the sound files are loaded when `wait` is true.
- `IsSoundBankLoaded(bank)` reports whether the bank and its sound files are loaded.

The engine starts with the plugin. In cooked games it runs for the whole run. In the editor it runs only during Play.

## Running Tests

Tests run through [xmake](https://xmake.io) tasks on Windows, Linux and macOS.

Native tests build the plugin and run inside a headless Flax Editor. They need:

- `FLAX_ENGINE_PATH`: the root of the Flax Engine used to build the plugin.
- `AMPLITUDE_TEST_PROJECT`: a built Amplitude project used as a fixture by the service tests.
- `AM_SDK_PATH`: the Amplitude SDK installation, whose `ampk` tool packs that fixture.

```bash
export FLAX_ENGINE_PATH=/path/to/FlaxEngine
export AMPLITUDE_TEST_PROJECT=/path/to/amplitude/build
xmake test-native
```

Use `xmake test-native -f "[service]"` to run a subset (any Catch test spec), and `xmake test-native --help` for the other options.

Editor tests (the ampk tool) only need .NET:

```bash
xmake test-editor
```

## License

This project is licensed under the **Apache License, Version 2.0**.

Copyright (c) 2026-present Sparky Studios. All rights reserved.

See the [LICENSE](LICENSE) file for more information.

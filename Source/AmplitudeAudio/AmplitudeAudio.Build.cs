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
using System.IO;
using Flax.Build;
using Flax.Build.NativeCpp;

public class AmplitudeAudio : GameModule
{
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);
        BuildNativeCode = true;
        options.CompileEnv.CppVersion = CppVersion.Cpp17;

        // Locate the Amplitude Audio SDK via AM_SDK_PATH environment variable
        var sdkRoot = Environment.GetEnvironmentVariable("AM_SDK_PATH");
        if (string.IsNullOrEmpty(sdkRoot))
        {
            Log.Error("AM_SDK_PATH environment variable is not set. Cannot locate the Amplitude Audio SDK.");
            return;
        }

        options.PublicIncludePaths.Add(Path.Combine(sdkRoot, "include"));

        // Amplitude SDK compile definitions (shared library linkage)
        options.PublicDefinitions.Add("AM_BUILDSYSTEM_SHARED");
        if (options.Configuration == TargetConfiguration.Release)
        {
            options.PublicDefinitions.Add("AM_NO_ASSERTS");
            options.PublicDefinitions.Add("AM_NO_MEMORY_STATS");
        }

        // Platform detection defines (mirroring DetectPlatform.cmake)
        var isWindows = options.Platform.Target == TargetPlatform.Windows;
        var isLinux = options.Platform.Target == TargetPlatform.Linux;
        var isMac = options.Platform.Target == TargetPlatform.Mac;
        var isIOS = options.Platform.Target == TargetPlatform.iOS;
        var isAndroid = options.Platform.Target == TargetPlatform.Android;
        var isApple = isMac || isIOS;
        var isUnix = isLinux || isApple || isAndroid;

        options.PublicDefinitions.Add($"AM_PLATFORM_WIN={(isWindows ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_PLATFORM_LINUX={(isLinux ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_PLATFORM_APPLE={(isApple ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_PLATFORM_MACOS={(isMac ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_PLATFORM_IOS={(isIOS ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_PLATFORM_ANDROID={(isAndroid ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_PLATFORM_UNIX={(isUnix ? 1 : 0)}");
        options.PublicDefinitions.Add("AM_PLATFORM_EMSCRIPTEN=0");

        // Compiler detection defines
        // Flax uses Clang on Mac/iOS/Android, MSVC on Windows, and GCC or Clang on Linux
        var isClang = isApple || isAndroid;
        var isMSVC = isWindows;
        var isGCC = isLinux;
        options.PublicDefinitions.Add($"AM_COMPILER_CLANG={(isClang ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_COMPILER_GCC={(isGCC ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_COMPILER_MSVC={(isMSVC ? 1 : 0)}");

        // Architecture detection defines
        var isX64 = options.Architecture == TargetArchitecture.x64;
        var isX86 = options.Architecture == TargetArchitecture.x86;
        var isARM64 = options.Architecture == TargetArchitecture.ARM64;
        var isARMv7 = options.Architecture == TargetArchitecture.ARM;
        var isARM = isARM64 || isARMv7;

        options.PublicDefinitions.Add($"AM_ARCH_X86_64={(isX64 ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_ARCH_X86={(isX86 ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_ARCH_ARM_64={(isARM64 ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_ARCH_ARM_V7={(isARMv7 ? 1 : 0)}");
        options.PublicDefinitions.Add($"AM_ARCH_ARM={(isARM ? 1 : 0)}");

        // Map Flax platform to Amplitude SDK platform name
        string sdkPlat;
        switch (options.Platform.Target)
        {
            case TargetPlatform.Windows:
                sdkPlat = "windows";
                break;
            case TargetPlatform.Linux:
                sdkPlat = "linux";
                break;
            case TargetPlatform.Mac:
                sdkPlat = "macosx";
                break;
            case TargetPlatform.iOS:
                sdkPlat = "iphoneos";
                break;
            case TargetPlatform.Android:
                sdkPlat = "android";
                break;
            default:
                Log.Error($"Amplitude Audio SDK does not support platform: {options.Platform.Target}");
                return;
        }

        // Map Flax architecture to Amplitude SDK architecture name
        var sdkArch = (options.Platform.Target, options.Architecture) switch
        {
            (TargetPlatform.Windows, TargetArchitecture.x64) => "x64",
            (TargetPlatform.Windows, TargetArchitecture.x86) => "x86",
            (TargetPlatform.Windows, TargetArchitecture.ARM64) => "arm64",
            (TargetPlatform.Linux, TargetArchitecture.x64) => "x86_64",
            (TargetPlatform.Linux, TargetArchitecture.x86) => "i386",
            (TargetPlatform.Linux, TargetArchitecture.ARM64) => "arm64-v8a",
            (TargetPlatform.Linux, TargetArchitecture.ARM) => "armv7",
            (TargetPlatform.Mac, TargetArchitecture.x64) => "x86_64",
            (TargetPlatform.Mac, TargetArchitecture.ARM64) => "arm64",
            (TargetPlatform.iOS, TargetArchitecture.ARM64) => "arm64",
            (TargetPlatform.iOS, TargetArchitecture.x64) => "x86_64",
            (TargetPlatform.Android, TargetArchitecture.ARM64) => "arm64-v8a",
            (TargetPlatform.Android, TargetArchitecture.ARM) => "armeabi-v7a",
            (TargetPlatform.Android, TargetArchitecture.x64) => "x86_64",
            (TargetPlatform.Android, TargetArchitecture.x86) => "x86",
            _ => null,
        };

        if (sdkArch == null)
        {
            Log.Error($"Amplitude Audio SDK does not support architecture {options.Architecture} on {options.Platform.Target}.");
            return;
        }

        // Use debug library for Debug and Development configurations, release for Release
        var libSuffix = options.Configuration != TargetConfiguration.Release ? "_d" : "";
        var sdkTarget = $"{sdkArch}-{sdkPlat}";
        var libDir = Path.Combine(sdkRoot, "lib", sdkTarget, "shared");

        if (!Directory.Exists(libDir))
        {
            Log.Error($"Amplitude Audio SDK libraries not found for target '{sdkTarget}' at: {libDir}");
            return;
        }

        // Link the Amplitude shared library
        switch (options.Platform.Target)
        {
            case TargetPlatform.Windows:
                options.Libraries.Add(Path.Combine(libDir, $"Amplitude{libSuffix}.lib"));
                options.DependencyFiles.Add(Path.Combine(libDir, $"Amplitude{libSuffix}.dll"));
                break;
            case TargetPlatform.Mac:
            case TargetPlatform.iOS:
                options.Libraries.Add(Path.Combine(libDir, $"libAmplitude{libSuffix}.dylib"));
                options.DependencyFiles.Add(Path.Combine(libDir, $"libAmplitude{libSuffix}.dylib"));
                // Add @loader_path to rpath so the dylib is found next to the plugin binary
                options.LinkEnv.CustomArgs.Add("-rpath @loader_path/");
                break;
            case TargetPlatform.Linux:
            case TargetPlatform.Android:
                options.Libraries.Add(Path.Combine(libDir, $"libAmplitude{libSuffix}.so"));
                options.DependencyFiles.Add(Path.Combine(libDir, $"libAmplitude{libSuffix}.so"));
                // Add $ORIGIN to rpath so libAmplitude.so is found next to the
                // plugin binary at load time. The $ORIGIN token must reach the
                // linker un-expanded.
                options.LinkEnv.CustomArgs.Add("-Wl,-rpath,$ORIGIN");
                break;
        }
    }
}

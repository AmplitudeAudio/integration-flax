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
using System.Diagnostics;
using System.IO;
using AmplitudeAudio;
using FlaxEditor;
using FlaxEditor.Content.Settings;
using FlaxEngine;
using Debug = FlaxEngine.Debug;

namespace AmplitudeAudioEditor.Cooking
{
    /// <summary>
    /// Deploys the Amplitude project build with cooked games, as loose files or an ampk package.
    /// </summary>
    public static class AmplitudeCooker
    {
        private const string AmplitudeFolder = "Content/Sounds/Amplitude";

        private static readonly TimeSpan PackageTimeout = TimeSpan.FromMinutes(10);

        /// <summary>
        /// Hooks the deployment into the game cooker.
        /// </summary>
        public static void Register()
        {
            GameCooker.DeployFiles += OnDeployFiles;
        }

        /// <summary>
        /// Removes the hook added by <see cref="Register"/>.
        /// </summary>
        public static void Unregister()
        {
            GameCooker.DeployFiles -= OnDeployFiles;
        }

        private static void OnDeployFiles()
        {
            try
            {
                Deploy();
            }
            catch (Exception e)
            {
                Fail($"Deploy failed: {e.Message}");
            }
        }

        private static void Deploy()
        {
            var settings = AmplitudeSettingsEditor.Get();
            if (!IsRelativeFolder(settings.ProjectBuildPath))
            {
                Fail($"Amplitude project build path must be a relative folder: '{settings.ProjectBuildPath}'.");
                return;
            }

            var sourceRoot = Path.Combine(Globals.ProjectFolder, AmplitudeFolder);
            var buildDirectory = Path.Combine(sourceRoot, settings.ProjectBuildPath);
            if (!Directory.Exists(buildDirectory))
            {
                Fail($"Amplitude build folder not found: {buildDirectory}");
                return;
            }

            var data = GameCooker.CurrentData;
            var outputRoot = Path.Combine(data.DataOutputPath, AmplitudeFolder);
            Directory.CreateDirectory(outputRoot);

            var buildOutput = Path.Combine(outputRoot, settings.ProjectBuildPath);
            var packageFile = buildOutput + ".ampk";

            // Output from a previous deploy in the other packaging mode must not be shipped alongside this one.
            if (settings.PackagingMode == AmplitudePackagingMode.Ampk)
            {
                DeleteDirectory(buildOutput);
                DeleteFile(packageFile);

                if (!Package(settings, buildDirectory, packageFile))
                    return;
            }
            else
            {
                DeleteFile(packageFile);
                CopyDirectory(buildDirectory, buildOutput);
                Debug.Log($"[Amplitude] Deployed {buildDirectory} as loose files.");
            }

            var platform = ToPlatformType(data.Platform);
            var profile = platform.HasValue ? AmplitudeSettingsEditor.GetProfile(settings, platform.Value) : settings.DefaultProfile;
            foreach (var searchPath in profile.PluginSearchPaths ?? Array.Empty<AmplitudePluginSearchPath>())
            {
                if (!IsRelativeFolder(searchPath.Path))
                {
                    Fail($"Amplitude plugin search path must be a relative folder: '{searchPath.Path}'.");
                    return;
                }

                var source = Path.Combine(sourceRoot, searchPath.Path);
                if (!Directory.Exists(source))
                {
                    Fail($"Amplitude plugin search path not found: {source}");
                    return;
                }

                CopyDirectory(source, Path.Combine(outputRoot, searchPath.Path));
            }
        }

        private static bool IsRelativeFolder(string path)
        {
            return !string.IsNullOrWhiteSpace(path) && !Path.IsPathRooted(path);
        }

        private static bool Package(AmplitudeSettings settings, string buildDirectory, string packageFile)
        {
            var tool = AmpkTool.Resolve(settings.AmpkToolPath, Environment.GetEnvironmentVariable, File.Exists, OperatingSystem.IsWindows());
            if (tool == null)
            {
                Fail("Could not find the ampk tool. Set Amplitude Settings > Cooking > Ampk Tool Path, or set AM_SDK_PATH to an Amplitude SDK installation.");
                return false;
            }

            var startInfo = new ProcessStartInfo(tool, AmpkTool.BuildArguments(buildDirectory, packageFile, (int)settings.Compression, settings.BlockSize))
            {
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
            };

            using var process = Process.Start(startInfo);
            if (process == null)
            {
                Fail($"Could not start the ampk tool: {tool}");
                return false;
            }

            // Read both streams concurrently: a full pipe would block ampk while we wait for it to exit.
            var outputTask = process.StandardOutput.ReadToEndAsync();
            var errorTask = process.StandardError.ReadToEndAsync();
            if (!process.WaitForExit((int)PackageTimeout.TotalMilliseconds))
            {
                process.Kill();
                Fail($"ampk did not finish within {PackageTimeout.TotalMinutes} minutes and was killed.");
                return false;
            }

            process.WaitForExit();
            var error = errorTask.Result;

            if (process.ExitCode != 0)
            {
                Fail($"ampk failed with exit code {process.ExitCode}: {error.Trim()}");
                return false;
            }

            Debug.Log($"[Amplitude] Packaged {buildDirectory} into {packageFile}.");
            return true;
        }

        private static void CopyDirectory(string source, string destination)
        {
            DeleteDirectory(destination);
            Directory.CreateDirectory(destination);

            foreach (var directory in Directory.GetDirectories(source, "*", SearchOption.AllDirectories))
                Directory.CreateDirectory(Path.Combine(destination, Path.GetRelativePath(source, directory)));

            foreach (var file in Directory.GetFiles(source, "*", SearchOption.AllDirectories))
                File.Copy(file, Path.Combine(destination, Path.GetRelativePath(source, file)), true);
        }

        private static void DeleteDirectory(string path)
        {
            if (Directory.Exists(path))
                Directory.Delete(path, true);
        }

        private static void DeleteFile(string path)
        {
            if (File.Exists(path))
                File.Delete(path);
        }

        private static PlatformType? ToPlatformType(BuildPlatform platform)
        {
            switch (platform)
            {
            case BuildPlatform.Windows32:
            case BuildPlatform.Windows64:
            case BuildPlatform.WindowsARM64:
                return PlatformType.Windows;
#pragma warning disable CS0612 // UWP is obsolete in Flax, but a configured UWP profile is still used by the runtime, so the cooker must match it.
            case BuildPlatform.UWPx86:
            case BuildPlatform.UWPx64:
                return PlatformType.UWP;
#pragma warning restore CS0612
            case BuildPlatform.XboxOne:
                return PlatformType.XboxOne;
            case BuildPlatform.XboxScarlett:
                return PlatformType.XboxScarlett;
            case BuildPlatform.LinuxX64:
                return PlatformType.Linux;
            case BuildPlatform.PS4:
                return PlatformType.PS4;
            case BuildPlatform.PS5:
                return PlatformType.PS5;
            case BuildPlatform.AndroidARM64:
                return PlatformType.Android;
            case BuildPlatform.Switch:
                return PlatformType.Switch;
            case BuildPlatform.MacOSx64:
            case BuildPlatform.MacOSARM64:
                return PlatformType.Mac;
            case BuildPlatform.iOSARM64:
                return PlatformType.iOS;
            case BuildPlatform.Web:
                return PlatformType.Web;
            default:
                return null;
            }
        }

        private static void Fail(string message)
        {
            Debug.LogError("[Amplitude] " + message);
            // DeployFiles has no failure channel; cancelling stops a cook that would ship without audio data.
            GameCooker.Cancel();
        }
    }
}

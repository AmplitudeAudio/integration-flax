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

namespace AmplitudeAudioEditor.Cooking
{
    /// <summary>
    /// Locates the ampk packaging tool and builds its command line.
    /// </summary>
    public static class AmpkTool
    {
        /// <summary>
        /// Environment variable pointing to the Amplitude SDK installation.
        /// </summary>
        public const string SdkPathVariable = "AM_SDK_PATH";

        /// <summary>
        /// Resolves the ampk executable: the configured path, then <c>$AM_SDK_PATH/bin</c>, then each <c>PATH</c> entry.
        /// </summary>
        /// <returns>The executable path, or <c>null</c> when it can't be found.</returns>
        public static string Resolve(string configuredPath, Func<string, string> getEnvironmentVariable, Func<string, bool> fileExists, bool isWindows)
        {
            if (!string.IsNullOrWhiteSpace(configuredPath))
                return fileExists(configuredPath) ? configuredPath : null;

            var executable = isWindows ? "ampk.exe" : "ampk";

            var sdkPath = getEnvironmentVariable(SdkPathVariable);
            if (!string.IsNullOrEmpty(sdkPath))
            {
                var candidate = Path.Combine(sdkPath, "bin", executable);
                if (fileExists(candidate))
                    return candidate;
            }

            var path = getEnvironmentVariable("PATH");
            if (string.IsNullOrEmpty(path))
                return null;

            foreach (var directory in path.Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries))
            {
                var candidate = Path.Combine(directory.Trim().Trim('"'), executable);
                if (fileExists(candidate))
                    return candidate;
            }

            return null;
        }

        /// <summary>
        /// Builds the arguments that package <paramref name="projectDirectory"/> into <paramref name="outputFile"/>.
        /// </summary>
        /// <param name="projectDirectory">Amplitude project build folder to package.</param>
        /// <param name="outputFile">Path of the .ampk file to write.</param>
        /// <param name="compression">0 for none, 1 for ZLib.</param>
        /// <param name="blockSizeKb">Compression block size, in KB.</param>
        public static string BuildArguments(string projectDirectory, string outputFile, int compression, int blockSizeKb)
        {
            // A trailing backslash would escape the closing quote on Windows. Trimming a root such as "C:\"
            // would leave a drive-relative "C:", so the root is kept with forward slashes instead.
            var root = Path.GetPathRoot(projectDirectory) ?? string.Empty;
            var project = projectDirectory.TrimEnd('/', '\\');
            if (project.Length < root.Length)
                project = root.Replace('\\', '/');
            return $"-q -c {compression} -s {blockSizeKb} \"{project}\" \"{outputFile}\"";
        }
    }
}

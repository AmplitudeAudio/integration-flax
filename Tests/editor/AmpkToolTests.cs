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
using System.Collections.Generic;
using System.IO;
using AmplitudeAudioEditor.Cooking;
using NUnit.Framework;

namespace AmplitudeAudioEditor.Tests
{
    [TestFixture]
    public class AmpkToolTests
    {
        private static readonly string SdkRoot = Path.Combine("sdk", "root");
        private static readonly string SdkTool = Path.Combine(SdkRoot, "bin", "ampk.exe");

        private static Func<string, bool> Existing(params string[] files)
        {
            var set = new HashSet<string>(files);
            return set.Contains;
        }

        private static Func<string, string> Environment(string sdkPath = null, string path = null)
        {
            return name => name switch
            {
                AmpkTool.SdkPathVariable => sdkPath,
                "PATH" => path,
                _ => null,
            };
        }

        [Test]
        public void Resolve_ReturnsConfiguredPath_WhenItExists()
        {
            var configured = Path.Combine("tools", "ampk.exe");

            var result = AmpkTool.Resolve(configured, Environment(SdkRoot), Existing(configured, SdkTool), true);

            Assert.That(result, Is.EqualTo(configured));
        }

        [Test]
        public void Resolve_ReturnsNull_WhenConfiguredPathIsMissing()
        {
            var result = AmpkTool.Resolve(Path.Combine("tools", "ampk.exe"), Environment(SdkRoot), Existing(SdkTool), true);

            Assert.That(result, Is.Null);
        }

        [Test]
        public void Resolve_UsesSdkBin_WhenNoPathIsConfigured()
        {
            var result = AmpkTool.Resolve("", Environment(SdkRoot), Existing(SdkTool), true);

            Assert.That(result, Is.EqualTo(SdkTool));
        }

        [Test]
        public void Resolve_UsesUnixExecutableName_OffWindows()
        {
            var unixTool = Path.Combine(SdkRoot, "bin", "ampk");

            var result = AmpkTool.Resolve(null, Environment(SdkRoot), Existing(unixTool), false);

            Assert.That(result, Is.EqualTo(unixTool));
        }

        [Test]
        public void Resolve_SearchesPathEntries_WhenTheSdkHasNoTool()
        {
            var expected = Path.Combine("second", "ampk.exe");
            var path = string.Join(Path.PathSeparator, "first", "second");

            var result = AmpkTool.Resolve(null, Environment(SdkRoot, path), Existing(expected), true);

            Assert.That(result, Is.EqualTo(expected));
        }

        [Test]
        public void Resolve_ReturnsNull_WhenTheToolIsNowhere()
        {
            var result = AmpkTool.Resolve(null, Environment(SdkRoot, "somewhere"), Existing(), true);

            Assert.That(result, Is.Null);
        }

        [Test]
        public void BuildArguments_QuotesPathsAndPassesOptions()
        {
            var project = Path.Combine("my project", "build");
            var output = Path.Combine("out", "build.ampk");

            var result = AmpkTool.BuildArguments(project, output, 1, 128);

            Assert.That(result, Is.EqualTo($"-q -c 1 -s 128 \"{project}\" \"{output}\""));
        }

        [Test]
        public void BuildArguments_TrimsTrailingSeparators()
        {
            var result = AmpkTool.BuildArguments("build" + Path.DirectorySeparatorChar, "build.ampk", 0, 64);

            Assert.That(result, Is.EqualTo("-q -c 0 -s 64 \"build\" \"build.ampk\""));
        }

        [Test]
        public void BuildArguments_KeepsRootDirectories()
        {
            var root = Path.GetPathRoot(Path.GetTempPath());
            var expected = root.Replace('\\', '/');

            var result = AmpkTool.BuildArguments(root, "out.ampk", 0, 64);

            Assert.That(result, Is.EqualTo($"-q -c 0 -s 64 \"{expected}\" \"out.ampk\""));
        }

        [Test]
        public void Resolve_StripsQuotesFromPathEntries()
        {
            var expected = Path.Combine("second", "ampk.exe");
            var path = string.Join(Path.PathSeparator, "first", "\"second\"");

            var result = AmpkTool.Resolve(null, Environment(SdkRoot, path), Existing(expected), true);

            Assert.That(result, Is.EqualTo(expected));
        }
    }
}

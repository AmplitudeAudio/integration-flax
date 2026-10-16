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

using System.IO;
using AmplitudeAudioEditor.Project;
using NUnit.Framework;

namespace AmplitudeAudioEditor.Tests
{
    public class AmplitudeProjectIndexTests
    {
        private static string Fixture => Path.Combine(TestContext.CurrentContext.TestDirectory, "Fixtures", "project");

        [Test]
        public void ListsObjectsByKind()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.GetObjects(AmplitudeObjectKind.Sound).Count, Is.EqualTo(1));
            Assert.That(index.GetObjects(AmplitudeObjectKind.Event)[0].Name, Is.EqualTo("play_throw"));
        }

        [Test]
        public void ReportsRelativePathsWithForwardSlashes()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.TryFind(AmplitudeObjectKind.Sound, 201, out var info));
            Assert.That(info.RelativePath, Is.EqualTo("sounds/footsteps/grass_a.json"));
        }

        [Test]
        public void SwitchStatesAreListedPerSwitch()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            var states = index.GetSwitchStates(2);
            Assert.That(states.Count, Is.EqualTo(2));
            Assert.That(states[1].Name, Is.EqualTo("desert"));
            Assert.That(index.GetSwitchStates(999), Is.Empty);
        }

        [Test]
        public void SkipsUnparsableFilesAndKeepsTheRest()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.Errors, Has.Count.EqualTo(1));
            Assert.That(index.GetObjects(AmplitudeObjectKind.Sound).Count, Is.EqualTo(1));
        }

        [Test]
        public void MissingFolderGivesAnEmptyIndex()
        {
            var index = AmplitudeProjectIndex.Load(Path.Combine(Fixture, "does-not-exist"));

            Assert.That(index.GetObjects(AmplitudeObjectKind.Sound), Is.Empty);
        }

        [Test]
        public void ListsEffects()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.GetObjects(AmplitudeObjectKind.Effect).Count, Is.EqualTo(1));
            Assert.That(index.TryFind(AmplitudeObjectKind.Effect, 5, out var info));
            Assert.That(info.Name, Is.EqualTo("bassboost"));
            Assert.That(info.RelativePath, Is.EqualTo("effects/bassboost.json"));
        }

        [Test]
        public void ResolvesTheAttenuationOfASound()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.TryGetAttenuation(201, out var shape));
            Assert.That(shape.Kind, Is.EqualTo(AmplitudeAttenuationKind.Sphere));
            Assert.That(shape.Inner, Is.EqualTo(new[] { 964f }));
            Assert.That(shape.Outer, Is.EqualTo(new[] { 1024f }));
        }

        [Test]
        public void ResolvesABoxAttenuationOfACollection()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.TryGetAttenuation(1003, out var shape));
            Assert.That(shape.Kind, Is.EqualTo(AmplitudeAttenuationKind.Box));
            Assert.That(shape.Inner, Is.EqualTo(new[] { 100f, 50f, 100f }));
        }

        [Test]
        public void NoAttenuationMeansNoShape()
        {
            var index = AmplitudeProjectIndex.Load(Fixture);

            Assert.That(index.TryGetAttenuation(200, out _), Is.False);   // switch container without one
            Assert.That(index.TryGetAttenuation(999999, out _), Is.False); // unknown id
        }
    }
}

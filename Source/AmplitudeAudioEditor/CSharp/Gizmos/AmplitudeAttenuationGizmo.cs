// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System.Collections.Generic;
using AmplitudeAudio;
using AmplitudeAudioEditor.Project;
using FlaxEditor;
using FlaxEditor.SceneGraph;
using FlaxEditor.SceneGraph.Actors;
using FlaxEngine;

namespace AmplitudeAudioEditor.Gizmos
{
    /// <summary>Pushes attenuation shapes to selected Amplitude emitters for debug drawing.</summary>
    public sealed class AmplitudeAttenuationGizmo
    {
        private static AmplitudeAttenuationGizmo _instance;
        public static AmplitudeAttenuationGizmo Instance => _instance ??= new AmplitudeAttenuationGizmo();

        private readonly List<Script> _trackedEmitters = new List<Script>();

        public void Initialize()
        {
            Editor.Instance.SceneEditing.SelectionChanged += OnSelectionChanged;
            AmplitudeProjectWatcher.Instance.Changed += OnProjectChanged;
        }

        public void Deinitialize()
        {
            ClearTracked();
            Editor.Instance.SceneEditing.SelectionChanged -= OnSelectionChanged;
            AmplitudeProjectWatcher.Instance.Changed -= OnProjectChanged;
        }

        private void OnSelectionChanged()
        {
            UpdateSelection();
        }

        private void OnProjectChanged()
        {
            UpdateSelection();
        }

        private void ClearTracked()
        {
            foreach (var emitter in _trackedEmitters)
            {
                if (emitter is AmplitudeEmitter ee)
                    ee.ClearEditorAttenuation();
                else if (emitter is AmplitudeInstancedEmitter ie)
                    ie.ClearEditorAttenuation();
            }
            _trackedEmitters.Clear();
        }

        private void UpdateSelection()
        {
            ClearTracked();

            var index = AmplitudeProjectWatcher.Instance.Index;
            if (index == null)
                return;

            var selection = Editor.Instance.SceneEditing.Selection;
            if (selection == null)
                return;

            foreach (var item in selection)
            {
                if (item is ActorNode actorNode && actorNode.Actor != null)
                {
                    foreach (var script in actorNode.Actor.Scripts)
                    {
                        ProcessScript(script, index);
                    }
                }
                else if (item.UndoRecordObject is Script script)
                {
                    ProcessScript(script, index);
                }
                else if (item.UndoRecordObject is Actor actor)
                {
                    foreach (var s in actor.Scripts)
                    {
                        ProcessScript(s, index);
                    }
                }
            }
        }

        private void ProcessScript(Script script, AmplitudeProjectIndex index)
        {
            if (script is AmplitudeEmitter emitter)
            {
                if (emitter.SoundObject.ID != 0 && index.TryGetAttenuation(emitter.SoundObject.ID, out var shape))
                {
                    emitter.SetEditorAttenuation((int)shape.Kind, shape.Inner, shape.Outer);
                    _trackedEmitters.Add(emitter);
                }
                else
                {
                    emitter.ClearEditorAttenuation();
                }
            }
            else if (script is AmplitudeInstancedEmitter instancedEmitter)
            {
                if (instancedEmitter.SoundObject.ID != 0 && index.TryGetAttenuation(instancedEmitter.SoundObject.ID, out var shape))
                {
                    instancedEmitter.SetEditorAttenuation((int)shape.Kind, shape.Inner, shape.Outer);
                    _trackedEmitters.Add(instancedEmitter);
                }
                else
                {
                    instancedEmitter.ClearEditorAttenuation();
                }
            }
        }
    }
}

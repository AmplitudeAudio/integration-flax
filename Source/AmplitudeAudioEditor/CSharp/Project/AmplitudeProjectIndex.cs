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
using System.Linq;
using Newtonsoft.Json.Linq;

namespace AmplitudeAudioEditor.Project
{
    /// <summary>The kind of Amplitude object.</summary>
    public enum AmplitudeObjectKind
    {
        Sound,
        Collection,
        SwitchContainer,
        Event,
        Rtpc,
        Switch,
        SoundBank,
    }

    /// <summary>Information about an Amplitude object from the project sources.</summary>
    public readonly struct AmplitudeObjectInfo
    {
        /// <summary>The kind of object.</summary>
        public AmplitudeObjectKind Kind { get; }

        /// <summary>The Amplitude ID.</summary>
        public ulong Id { get; }

        /// <summary>The display name.</summary>
        public string Name { get; }

        /// <summary>The relative path from the sources folder (forward slashes).</summary>
        public string RelativePath { get; }

        public AmplitudeObjectInfo(AmplitudeObjectKind kind, ulong id, string name, string relativePath)
        {
            Kind = kind;
            Id = id;
            Name = name;
            RelativePath = relativePath;
        }
    }

    /// <summary>Information about a switch state.</summary>
    public readonly struct AmplitudeSwitchStateInfo
    {
        /// <summary>The state ID.</summary>
        public ulong Id { get; }

        /// <summary>The state name.</summary>
        public string Name { get; }

        public AmplitudeSwitchStateInfo(ulong id, string name)
        {
            Id = id;
            Name = name;
        }
    }

    /// <summary>Parses Amplitude project JSON sources into an index for editor pickers.</summary>
    public sealed class AmplitudeProjectIndex
    {
        private static readonly (string Folder, AmplitudeObjectKind Kind)[] s_folders =
        {
            ("sounds", AmplitudeObjectKind.Sound),
            ("collections", AmplitudeObjectKind.Collection),
            ("switch_containers", AmplitudeObjectKind.SwitchContainer),
            ("events", AmplitudeObjectKind.Event),
            ("rtpc", AmplitudeObjectKind.Rtpc),
            ("switches", AmplitudeObjectKind.Switch),
            ("soundbanks", AmplitudeObjectKind.SoundBank),
        };

        private readonly Dictionary<AmplitudeObjectKind, List<AmplitudeObjectInfo>> _objects =
            new Dictionary<AmplitudeObjectKind, List<AmplitudeObjectInfo>>();
        private readonly Dictionary<ulong, List<AmplitudeSwitchStateInfo>> _switchStates =
            new Dictionary<ulong, List<AmplitudeSwitchStateInfo>>();
        private readonly List<string> _errors = new List<string>();

        /// <summary>Errors encountered while parsing.</summary>
        public IReadOnlyList<string> Errors => _errors;

        /// <summary>Loads an index from the given sources path.</summary>
        public static AmplitudeProjectIndex Load(string sourcesPath)
        {
            var index = new AmplitudeProjectIndex();
            if (!Directory.Exists(sourcesPath))
                return index;

            foreach (var (folder, kind) in s_folders)
            {
                var dirPath = Path.Combine(sourcesPath, folder);
                if (!Directory.Exists(dirPath))
                    continue;

                var list = new List<AmplitudeObjectInfo>();
                var files = Directory.EnumerateFiles(dirPath, "*.json", SearchOption.AllDirectories);

                foreach (var file in files)
                {
                    try
                    {
                        var relative = Path.GetRelativePath(sourcesPath, file).Replace('\\', '/');
                        var text = File.ReadAllText(file);
                        var obj = JObject.Parse(text);
                        if (obj == null)
                            continue;

                        if (obj["id"]?.Type != JTokenType.Integer)
                            continue;
                        var id = obj["id"].Value<ulong>();
                        if (obj["name"]?.Type != JTokenType.String)
                            continue;
                        var name = obj["name"].Value<string>();

                        list.Add(new AmplitudeObjectInfo(kind, id, name, relative));

                        // Parse switch states
                        if (kind == AmplitudeObjectKind.Switch && obj["states"] is JArray statesList)
                        {
                            var states = new List<AmplitudeSwitchStateInfo>();
                            foreach (var state in statesList.OfType<JObject>())
                            {
                                if (state["id"]?.Type == JTokenType.Integer && state["name"]?.Type == JTokenType.String)
                                {
                                    states.Add(new AmplitudeSwitchStateInfo(state["id"].Value<ulong>(), state["name"].Value<string>()));
                                }
                            }
                            index._switchStates[id] = states;
                        }
                    }
                    catch (Exception ex)
                    {
                        index._errors.Add(ex.Message);
                    }
                }

                list.Sort((a, b) => string.Compare(a.Name, b.Name, StringComparison.OrdinalIgnoreCase));
                index._objects[kind] = list;
            }

            return index;
        }

        /// <summary>Returns all objects of the given kind.</summary>
        public IReadOnlyList<AmplitudeObjectInfo> GetObjects(AmplitudeObjectKind kind)
        {
            return _objects.TryGetValue(kind, out var list) ? (IReadOnlyList<AmplitudeObjectInfo>)list : Array.Empty<AmplitudeObjectInfo>();
        }

        /// <summary>Tries to find an object by kind and ID.</summary>
        public bool TryFind(AmplitudeObjectKind kind, ulong id, out AmplitudeObjectInfo info)
        {
            if (_objects.TryGetValue(kind, out var list))
            {
                foreach (var item in list)
                {
                    if (item.Id == id)
                    {
                        info = item;
                        return true;
                    }
                }
            }
            info = default;
            return false;
        }

        /// <summary>Returns the switch states for the given switch ID.</summary>
        public IReadOnlyList<AmplitudeSwitchStateInfo> GetSwitchStates(ulong switchId)
        {
            return _switchStates.TryGetValue(switchId, out var states) ? (IReadOnlyList<AmplitudeSwitchStateInfo>)states : Array.Empty<AmplitudeSwitchStateInfo>();
        }
    }
}

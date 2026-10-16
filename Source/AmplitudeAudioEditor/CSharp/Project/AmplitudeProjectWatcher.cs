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
using System.Threading;
using AmplitudeAudioEditor.Project;
using FlaxEngine;

namespace AmplitudeAudioEditor.Project
{
    /// <summary>Watches an Amplitude project sources folder and provides a cached index.</summary>
    public sealed class AmplitudeProjectWatcher : IDisposable
    {
        private static AmplitudeProjectWatcher s_instance;
        private FileSystemWatcher _watcher;
        private AmplitudeProjectIndex _index;
        private string _sourcesPath;
        private Timer _debounceTimer;
        private readonly object _lock = new object();

        /// <summary>Gets the singleton instance.</summary>
        public static AmplitudeProjectWatcher Instance
        {
            get
            {
                if (s_instance == null)
                    s_instance = new AmplitudeProjectWatcher();
                return s_instance;
            }
        }

        /// <summary>The current project index (loaded lazily).</summary>
        public AmplitudeProjectIndex Index
        {
            get
            {
                var path = GetSourcesPath();
                if (_index == null || !string.Equals(_sourcesPath, path, StringComparison.OrdinalIgnoreCase))
                    Rebuild();
                return _index;
            }
        }

        /// <summary>Raised when the index changes.</summary>
        public event Action Changed;

        private AmplitudeProjectWatcher()
        {
        }

        private string GetSourcesPath()
        {
            var settings = AmplitudeAudioEditor.AmplitudeSettingsEditor.Get();
            if (settings == null || string.IsNullOrEmpty(settings.ProjectSourcesPath))
                return null;

            var relative = settings.ProjectSourcesPath;
            return Path.Combine(Globals.ProjectContentFolder, "Sounds", "Amplitude", relative);
        }

        private void Rebuild()
        {
            var path = GetSourcesPath();
            AmplitudeProjectIndex newIndex;
            if (path == null || !Directory.Exists(path))
                newIndex = AmplitudeProjectIndex.Load(Path.Combine(Globals.ProjectContentFolder, "nonexistent"));
            else
                newIndex = AmplitudeProjectIndex.Load(path);

            lock (_lock)
            {
                _index = newIndex;
                _sourcesPath = path;
            }

            // Set up file watcher
            SetupWatcher(path);

            Changed?.Invoke();
        }

        private void SetupWatcher(string path)
        {
            _watcher?.Dispose();
            _watcher = null;

            if (path == null || !Directory.Exists(path))
                return;

            _watcher = new FileSystemWatcher(path)
            {
                Filter = "*.json",
                IncludeSubdirectories = true,
                NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite,
            };

            _watcher.Changed += OnFileChanged;
            _watcher.Created += OnFileChanged;
            _watcher.Deleted += OnFileChanged;
            _watcher.Renamed += OnFileChanged;
            _watcher.EnableRaisingEvents = true;
        }

        private void OnFileChanged(object sender, FileSystemEventArgs e)
        {
            // Debounce 300ms on the editor main thread
            _debounceTimer?.Dispose();
            _debounceTimer = new Timer(_ =>
            {
                Scripting.InvokeOnUpdate(() => Rebuild());
            }, null, 300, Timeout.Infinite);
        }

        /// <summary>Disposes the watcher.</summary>
        public void Dispose()
        {
            _watcher?.Dispose();
            _watcher = null;
            _debounceTimer?.Dispose();
            _debounceTimer = null;
            lock (_lock)
            {
                _index = null;
                _sourcesPath = null;
            }
        }
    }
}

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
using AmplitudeAudio;
using AmplitudeAudioEditor.Project;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Elements;
using FlaxEditor.GUI;
using FlaxEngine;
using FlaxEngine.GUI;

namespace AmplitudeAudioEditor.Pickers
{
    /// <summary>One selectable object in a reference picker.</summary>
    internal readonly struct PickerEntry
    {
        public readonly ulong Id;
        public readonly string Label;
        public readonly string Tooltip;

        public PickerEntry(ulong id, string label, string tooltip)
        {
            Id = id;
            Label = label;
            Tooltip = tooltip;
        }
    }

    /// <summary>Name field with a clear button and a searchable popup, in the style of Flax's mesh reference picker.</summary>
    internal sealed class AmplitudeReferencePickerControl : Control
    {
        private Float2 _mousePos;
        private ulong _id;
        private string _label;

        public Func<IReadOnlyList<PickerEntry>> GetEntries;
        public event Action<ulong> ValueChanged;

        public AmplitudeReferencePickerControl()
        : base(0, 0, 50, 16)
        {
        }

        public void SetDisplay(ulong id, string label)
        {
            _id = id;
            _label = label;
        }

        private bool IsSelected => _id != 0;

        private void GetRects(out Rectangle frame, out Rectangle name, out Rectangle clear, out Rectangle picker)
        {
            frame = new Rectangle(0, 0, Width, 16);
            if (IsSelected)
                frame.Width -= 16;
            frame.Width -= 16;
            name = new Rectangle(2, 1, frame.Width - 4, 14);
            clear = new Rectangle(name.Right + 2, 1, 14, 14);
            picker = IsSelected ? new Rectangle(clear.Right + 2, 1, 14, 14) : clear;
        }

        private void ShowMenu()
        {
            Focus();
            var menu = new ItemsListContextMenu(320);
            menu.AddItem(new ItemsListContextMenu.Item { Name = "<none>", Tag = 0UL });

            var entries = GetEntries?.Invoke();
            if (entries != null)
            {
                foreach (var entry in entries)
                {
                    var item = new ItemsListContextMenu.Item
                    {
                        Name = entry.Label,
                        TooltipText = entry.Tooltip,
                        Tag = entry.Id,
                    };
                    if (entry.Id == _id)
                        item.BackgroundColor = FlaxEngine.GUI.Style.Current.BackgroundSelected;
                    menu.AddItem(item);
                }
            }

            menu.ItemClicked += item => ValueChanged?.Invoke((ulong)item.Tag);
            menu.Show(Parent, BottomLeft);
        }

        /// <inheritdoc />
        public override void Draw()
        {
            base.Draw();

            var style = FlaxEngine.GUI.Style.Current;
            var enabled = EnabledInHierarchy;
            GetRects(out var frame, out var name, out var clear, out var picker);

            Render2D.DrawRectangle(frame, enabled && (IsMouseOver || IsNavFocused) ? style.BorderHighlighted : style.BorderNormal);

            if (IsSelected)
            {
                Render2D.PushClip(name);
                Render2D.DrawText(style.FontMedium, _label, name, enabled ? style.Foreground : style.ForegroundDisabled, TextAlignment.Near, TextAlignment.Center);
                Render2D.PopClip();

                Render2D.DrawSprite(style.Cross, clear, enabled && clear.Contains(_mousePos) ? style.Foreground : style.ForegroundGrey);
            }
            else
            {
                Render2D.DrawText(style.FontMedium, "-", name, enabled ? Color.OrangeRed : Color.DarkOrange, TextAlignment.Near, TextAlignment.Center);
            }

            Render2D.DrawSprite(style.ArrowDown, picker, enabled && picker.Contains(_mousePos) ? style.Foreground : style.ForegroundGrey);
        }

        /// <inheritdoc />
        public override void OnMouseEnter(Float2 location)
        {
            _mousePos = location;
            base.OnMouseEnter(location);
        }

        /// <inheritdoc />
        public override void OnMouseLeave()
        {
            _mousePos = Float2.Minimum;
            base.OnMouseLeave();
        }

        /// <inheritdoc />
        public override void OnMouseMove(Float2 location)
        {
            _mousePos = location;
            base.OnMouseMove(location);
        }

        /// <inheritdoc />
        public override bool OnMouseUp(Float2 location, MouseButton button)
        {
            if (button == MouseButton.Left)
            {
                GetRects(out _, out _, out var clear, out _);
                if (IsSelected && clear.Contains(ref location))
                    ValueChanged?.Invoke(0);
                else
                    ShowMenu();
            }

            return base.OnMouseUp(location, button);
        }

        /// <inheritdoc />
        public override void OnSubmit()
        {
            base.OnSubmit();
            ShowMenu();
        }

        /// <inheritdoc />
        public override void OnDestroy()
        {
            GetEntries = null;
            ValueChanged = null;
            base.OnDestroy();
        }
    }

    /// <summary>Base editor for Amplitude reference structs: picks an object of the project index by name.</summary>
    public abstract class AmplitudeReferenceEditor : CustomEditor
    {
        private CustomElement<AmplitudeReferencePickerControl> _picker;
        private List<PickerEntry> _entries = new List<PickerEntry>();
        private readonly Dictionary<ulong, PickerEntry> _byId = new Dictionary<ulong, PickerEntry>();
        private bool _dirty = true;
        private ulong _context = ulong.MaxValue;

        /// <summary>The object kinds the picker lists, in display order.</summary>
        protected abstract AmplitudeObjectKind[] Kinds { get; }

        /// <summary>Reads the ID out of the boxed reference struct.</summary>
        protected abstract ulong GetId(object reference);

        /// <summary>Boxes a new reference struct holding the ID.</summary>
        protected abstract object CreateReference(ulong id);

        /// <summary>Extra state the entries depend on (the sibling switch for a state picker). The list is rebuilt when it changes.</summary>
        protected virtual ulong EntriesContext() => 0;

        /// <summary>Lists the entries to show.</summary>
        internal virtual List<PickerEntry> BuildEntries(AmplitudeProjectIndex index, ulong context)
        {
            var entries = new List<PickerEntry>();
            var prefixed = Kinds.Length > 1;
            foreach (var kind in Kinds)
            {
                foreach (var obj in index.GetObjects(kind))
                {
                    var label = prefixed && kind != AmplitudeObjectKind.Sound ? $"[{kind}] {obj.Name}" : obj.Name;
                    entries.Add(new PickerEntry(obj.Id, label, obj.RelativePath));
                }
            }
            return entries;
        }

        /// <summary>Label for an ID that is set but absent from the entries.</summary>
        protected virtual string MissingLabel(ulong id, ulong context) => $"<missing: {id}>";

        /// <inheritdoc />
        public override DisplayStyle Style => DisplayStyle.Inline;

        /// <inheritdoc />
        public override void Initialize(LayoutElementsContainer layout)
        {
            _picker = layout.Custom<AmplitudeReferencePickerControl>();
            _picker.CustomControl.GetEntries = () => _entries;
            _picker.CustomControl.ValueChanged += id => SetValue(CreateReference(id));
            AmplitudeProjectWatcher.Instance.Changed += OnProjectChanged;
            _dirty = true;
        }

        /// <inheritdoc />
        public override void Refresh()
        {
            base.Refresh();

            if (_picker == null || Values == null || Values.Count == 0)
                return;

            var context = EntriesContext();
            if (_dirty || context != _context)
            {
                _entries = BuildEntries(AmplitudeProjectWatcher.Instance.Index, context);
                _byId.Clear();
                foreach (var entry in _entries)
                    _byId[entry.Id] = entry;
                _context = context;
                _dirty = false;
            }

            var id = GetId(Values[0]);
            var label = string.Empty;
            if (id != 0)
                label = _byId.TryGetValue(id, out var entry) ? entry.Label : MissingLabel(id, context);

            _picker.CustomControl.SetDisplay(id, label);
            _picker.CustomControl.TooltipText = id != 0 && _byId.TryGetValue(id, out var found) ? found.Tooltip : null;
        }

        /// <inheritdoc />
        protected override void Deinitialize()
        {
            AmplitudeProjectWatcher.Instance.Changed -= OnProjectChanged;
            base.Deinitialize();
        }

        private void OnProjectChanged()
        {
            _dirty = true;
        }

        /// <summary>The ID of a sibling field of type <typeparamref name="T"/> on the parent editor, or 0.</summary>
        protected ulong FindSiblingId<T>(Func<T, ulong> read)
        {
            if (ParentEditor == null)
                return 0;

            foreach (var child in ParentEditor.ChildrenEditors)
            {
                if (child != this && child.Values != null && child.Values.Count > 0 && child.Values[0] is T value)
                    return read(value);
            }
            return 0;
        }
    }

    [CustomEditor(typeof(AmplitudeSoundRef)), DefaultEditor]
    public sealed class AmplitudeSoundRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.Sound };
        protected override ulong GetId(object reference) => ((AmplitudeSoundRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeSoundRef { ID = id };
    }

    [CustomEditor(typeof(AmplitudeCollectionRef)), DefaultEditor]
    public sealed class AmplitudeCollectionRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.Collection };
        protected override ulong GetId(object reference) => ((AmplitudeCollectionRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeCollectionRef { ID = id };
    }

    [CustomEditor(typeof(AmplitudeSwitchContainerRef)), DefaultEditor]
    public sealed class AmplitudeSwitchContainerRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.SwitchContainer };
        protected override ulong GetId(object reference) => ((AmplitudeSwitchContainerRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeSwitchContainerRef { ID = id };
    }

    [CustomEditor(typeof(AmplitudeSoundObjectRef)), DefaultEditor]
    public sealed class AmplitudeSoundObjectRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[]
        {
            AmplitudeObjectKind.Sound,
            AmplitudeObjectKind.Collection,
            AmplitudeObjectKind.SwitchContainer,
        };
        protected override ulong GetId(object reference) => ((AmplitudeSoundObjectRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeSoundObjectRef { ID = id };
    }

    [CustomEditor(typeof(AmplitudeEventRef)), DefaultEditor]
    public sealed class AmplitudeEventRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.Event };
        protected override ulong GetId(object reference) => ((AmplitudeEventRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeEventRef { ID = id };
    }

    [CustomEditor(typeof(AmplitudeRtpcRef)), DefaultEditor]
    public sealed class AmplitudeRtpcRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.Rtpc };
        protected override ulong GetId(object reference) => ((AmplitudeRtpcRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeRtpcRef { ID = id };
    }

    [CustomEditor(typeof(AmplitudeSwitchRef)), DefaultEditor]
    public sealed class AmplitudeSwitchRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.Switch };
        protected override ulong GetId(object reference) => ((AmplitudeSwitchRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeSwitchRef { ID = id };
    }

    /// <summary>Lists the states of the <see cref="AmplitudeSwitchRef"/> field declared next to this one.</summary>
    [CustomEditor(typeof(AmplitudeSwitchStateRef)), DefaultEditor]
    public sealed class AmplitudeSwitchStateRefEditor : AmplitudeReferenceEditor
    {
        protected override AmplitudeObjectKind[] Kinds => new[] { AmplitudeObjectKind.Switch };
        protected override ulong GetId(object reference) => ((AmplitudeSwitchStateRef)reference).ID;
        protected override object CreateReference(ulong id) => new AmplitudeSwitchStateRef { ID = id };

        protected override ulong EntriesContext() => FindSiblingId<AmplitudeSwitchRef>(sw => sw.ID);

        internal override List<PickerEntry> BuildEntries(AmplitudeProjectIndex index, ulong context)
        {
            var entries = new List<PickerEntry>();
            if (context == 0)
                return entries;

            foreach (var state in index.GetSwitchStates(context))
                entries.Add(new PickerEntry(state.Id, state.Name, null));
            return entries;
        }

        // Without a sibling switch the state can't be resolved; show the raw ID instead of flagging it missing.
        protected override string MissingLabel(ulong id, ulong context) => context == 0 ? $"ID: {id}" : base.MissingLabel(id, context);
    }
}

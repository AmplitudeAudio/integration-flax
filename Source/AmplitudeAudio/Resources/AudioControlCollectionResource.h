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

#pragma once

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingObject.h>
#include <Engine/Serialization/JsonTools.h>

#include <AmplitudeAudio/Core/Data.h>

/// <summary>
/// Discriminator for entries stored inside an AudioControlCollection.
/// </summary>
API_ENUM()
enum class AudioControlType : uint8
{
    Invalid     = 0,
    Trigger     = 1,
    Rtpc        = 2,
    SoundBank   = 3,
    Switch      = 4,
    SwitchState = 5,
    Environment = 6,
};

/// <summary>
/// A single named audio control stored inside an AudioControlCollection.
/// </summary>
/// <remarks>
/// <c>Name</c> is the authored control label used by editor tooling and runtime
/// lookup. For switch containers it matches the authored switch token; for
/// switch states it uses the fully qualified \c <switch>/<state> label while
/// the parent and child metadata remain split across the Amplitude fields.
/// </remarks>
API_STRUCT()
struct AMPLITUDEAUDIO_API AudioControlEntry : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioControlEntry);
    API_AUTO_SERIALIZATION();

    /// <summary>
    /// Authored control label used in editor UI, logging, and name-based lookup.
    /// Switch entries use the switch token; switch-state entries use
    /// \c <switch-token>/<state-token>.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0)")
    String Name;

    /// <summary>Discriminator that determines which Amplitude data type this entry maps to.</summary>
    API_FIELD(Attributes = "EditorOrder(1)")
    AudioControlType Type = AudioControlType::Invalid;

    /// <summary>
    /// Numeric Amplitude asset ID for this entry's primary control object
    /// (e.g. event ID, RTPC ID, switch ID, or switch-state ID).
    /// For switch-state entries this stores the child state ID; the parent
    /// switch ID lives in <c>ParentAmplitudeId</c>.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2)")
    uint64 AmplitudeId = 0;

    /// <summary>
    /// Authored Amplitude token for this entry's primary control object
    /// (e.g. event name, bank filename, or switch-state name).
    /// For switch-state entries this stores only the child state token; the
    /// parent switch token lives in <c>ParentAmplitudeName</c>.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(3)")
    String AmplitudeName;

    /// <summary>
    /// Parent Amplitude asset ID for child controls such as switch states.
    /// Leave at 0 for top-level controls that do not belong to a parent container.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(4)")
    uint64 ParentAmplitudeId = 0;

    /// <summary>
    /// Parent Amplitude asset token for child controls such as switch states.
    /// Leave empty for top-level controls that do not belong to a parent container.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(5)")
    String ParentAmplitudeName;
};

/// <summary>
/// An ordered collection of AudioControlEntry values loaded from JSON.
/// </summary>
/// <remarks>
/// Create via the editor's "New > Audio > Audio Control Collection" context
/// menu entry.
/// </remarks>
API_CLASS(Attributes = "ContentContextMenu(\"New/Audio System/Amplitude/Audio Control Collection\")")
class AMPLITUDEAUDIO_API AudioControlCollection : public ScriptingObject, public ISerializable
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(AudioControlCollection, ScriptingObject);

  public:
    // ========================================================================
    //  Data access
    // ========================================================================

    /// <summary>The audio control entries in this collection.</summary>
    API_PROPERTY(Attributes = "EditorOrder(0)")
    const Array<AudioControlEntry>& GetControls() const;

    /// <summary>Sets the audio control entries in this collection.</summary>
    API_PROPERTY()
    void SetControls(const Array<AudioControlEntry>& value);

    // ========================================================================
    //  ISerializable
    // ========================================================================

    void Serialize(SerializeStream& stream, const void* otherObj) override;
    void Deserialize(DeserializeStream& stream, ISerializeModifier* modifier) override;

  private:
    Array<AudioControlEntry> _controls;
};

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>
    /// Static helpers that construct Amplitude data wrappers from AudioControlEntry values.
    /// </summary>
    /// <remarks>
    /// Each method allocates a new wrapper on the heap; the caller takes
    /// ownership and is responsible for deleting the returned pointer.
    /// </remarks>
    struct AudioControlEntryDeserializer
    {
        static AmplitudeTriggerData*     DeserializeTriggerEntry(const AudioControlEntry& entry);
        static AmplitudeRtpcData*        DeserializeRtpcEntry(const AudioControlEntry& entry);
        static AmplitudeSwitchData*      DeserializeSwitchEntry(const AudioControlEntry& entry);
        static AmplitudeSwitchStateData* DeserializeSwitchStateEntry(const AudioControlEntry& entry);
        static AmplitudeEnvironmentData* DeserializeEnvironmentEntry(const AudioControlEntry& entry);
        static AmplitudeBankData*        DeserializeBankEntry(const AudioControlEntry& entry);

        AudioControlEntryDeserializer() = delete;
    };
} // namespace AmplitudeAudio

#pragma once

// ============================================================================
//  AudioControlCollectionResource — Named audio control collection asset
//
//  Defines the AudioControlType enum, AudioControlEntry struct, and the
//  AudioControlCollection class. The collection bundles named audio controls
//  (triggers, RTPCs, banks, switches, environments) into a single loadable
//  unit that can be edited in the Flax editor and loaded at runtime.
// ============================================================================

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingObject.h>
#include <Engine/Serialization/JsonTools.h>

#include "AmplitudeAudio/Core/AmplitudeAudioData.h"

// ============================================================================
//  AudioControlType
// ============================================================================

/// \brief Discriminator for entries stored inside an AudioControlCollection.
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

// ============================================================================
//  AudioControlEntry
// ============================================================================

/// \brief A single named audio control stored inside an AudioControlCollection.
///
/// \c Name is the authored control label used by editor tooling and runtime
/// lookup. For switch containers it matches the authored switch token; for
/// switch states it uses the fully qualified \c <switch>/<state> label while
/// the parent and child metadata remain split across the Amplitude fields.
API_STRUCT()
struct AMPLITUDEAUDIO_API AudioControlEntry : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioControlEntry);
    API_AUTO_SERIALIZATION();

    /// Authored control label used in editor UI, logging, and name-based lookup.
    /// Switch entries use the switch token; switch-state entries use
    /// \c <switch-token>/<state-token>.
    API_FIELD(Attributes="EditorOrder(0)")
    String Name;

    /// Discriminator that determines which Amplitude data type this entry maps to.
    API_FIELD(Attributes="EditorOrder(1)")
    AudioControlType Type = AudioControlType::Invalid;

    /// Numeric Amplitude asset ID for this entry's primary control object
    /// (e.g. event ID, RTPC ID, switch ID, or switch-state ID).
    /// For switch-state entries this stores the child state ID; the parent
    /// switch ID lives in \c ParentAmplitudeId.
    API_FIELD(Attributes="EditorOrder(2)")
    uint64 AmplitudeId = 0;

    /// Authored Amplitude token for this entry's primary control object
    /// (e.g. event name, bank filename, or switch-state name).
    /// For switch-state entries this stores only the child state token; the
    /// parent switch token lives in \c ParentAmplitudeName.
    API_FIELD(Attributes="EditorOrder(3)")
    String AmplitudeName;

    /// Parent Amplitude asset ID for child controls such as switch states.
    /// Leave at 0 for top-level controls that do not belong to a parent container.
    API_FIELD(Attributes="EditorOrder(4)")
    uint64 ParentAmplitudeId = 0;

    /// Parent Amplitude asset token for child controls such as switch states.
    /// Leave empty for top-level controls that do not belong to a parent container.
    API_FIELD(Attributes="EditorOrder(5)")
    String ParentAmplitudeName;
};

// ============================================================================
//  AudioControlCollection
// ============================================================================

/// \brief An ordered collection of AudioControlEntry values loaded from JSON.
///
/// Create via the editor's "New > Audio > Audio Control Collection" context
/// menu entry.
API_CLASS(Attributes="ContentContextMenu(\"New/Audio/Audio Control Collection\")")
class AMPLITUDEAUDIO_API AudioControlCollection : public ScriptingObject, public ISerializable
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(AudioControlCollection, ScriptingObject);

public:
    // ========================================================================
    //  Data access (exposed to editor via API_PROPERTY)
    // ========================================================================

    /// The audio control entries in this collection.
    API_PROPERTY(Attributes="EditorOrder(0)")
    const Array<AudioControlEntry>& GetControls() const;

    /// Sets the audio control entries in this collection.
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

// ============================================================================
//  AudioControlEntryDeserializer (internal, not exposed to scripting)
// ============================================================================

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// \brief Static helpers that construct Amplitude data wrappers from
    ///        AudioControlEntry values.
    ///
    /// Each method allocates a new wrapper on the heap; the caller takes
    /// ownership and is responsible for deleting the returned pointer.
    struct AudioControlEntryDeserializer
    {
        static AmplitudeTriggerData* DeserializeTriggerEntry(const AudioControlEntry& entry);
        static AmplitudeRtpcData* DeserializeRtpcEntry(const AudioControlEntry& entry);
        static AmplitudeSwitchData* DeserializeSwitchEntry(const AudioControlEntry& entry);
        static AmplitudeSwitchStateData* DeserializeSwitchStateEntry(const AudioControlEntry& entry);
        static AmplitudeEnvironmentData* DeserializeEnvironmentEntry(const AudioControlEntry& entry);
        static AmplitudeBankData* DeserializeBankEntry(const AudioControlEntry& entry);

    private:
        AudioControlEntryDeserializer() = delete;
    };

} // namespace AmplitudeAudio

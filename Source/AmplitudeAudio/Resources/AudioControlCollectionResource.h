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
/// Exactly one of \c AmplitudeId or \c AmplitudeName needs to be set,
/// depending on whether the backing Amplitude asset is looked up by
/// numeric ID or by string name.
API_STRUCT()
struct AMPLITUDEAUDIO_API AudioControlEntry : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioControlEntry);
    API_AUTO_SERIALIZATION();

    /// Human-readable label for this control (used in editor UI and logging).
    API_FIELD(Attributes="EditorOrder(0)")
    String Name;

    /// Discriminator that determines which Amplitude data type this entry maps to.
    API_FIELD(Attributes="EditorOrder(1)")
    AudioControlType Type = AudioControlType::Invalid;

    /// Numeric Amplitude asset ID (e.g. AmEventID, AmRtpcID).
    /// Use 0 when the entry is identified by \c AmplitudeName instead.
    API_FIELD(Attributes="EditorOrder(2)")
    uint64 AmplitudeId = 0;

    /// String name used by Amplitude for name-based lookup (e.g. "init.ambank").
    /// Leave empty when the entry is identified by \c AmplitudeId instead.
    API_FIELD(Attributes="EditorOrder(3)")
    String AmplitudeName;
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
        static AmplitudeSwitchStateData* DeserializeSwitchStateEntry(const AudioControlEntry& entry);
        static AmplitudeEnvironmentData* DeserializeEnvironmentEntry(const AudioControlEntry& entry);
        static AmplitudeBankData* DeserializeBankEntry(const AudioControlEntry& entry);

    private:
        AudioControlEntryDeserializer() = delete;
    };

} // namespace AmplitudeAudio

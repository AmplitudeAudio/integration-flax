#pragma once

// ============================================================================
//  AudioControlCollectionResource — Named audio control collection asset
//
//  Defines the AudioControlType enum, AudioControlEntry struct, and the
//  AudioControlCollection class. The collection bundles named audio controls
//  (triggers, RTPCs, banks, switches, environments) into a single loadable
//  unit that can be parsed from JSON.
//
//  JSON format:
//      {
//          "Controls": [
//              { "Name": "PlayExplosion", "Type": "Trigger",   "AmplitudeId": 12345 },
//              { "Name": "MusicIntensity","Type": "Rtpc",      "AmplitudeId": 67890 },
//              { "Name": "InitBank",      "Type": "SoundBank", "AmplitudeName": "init.ambank" }
//          ]
//      }
// ============================================================================

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Serialization/JsonTools.h>

#include "AmplitudeAudio/Core/AmplitudeAudioData.h"

namespace AmplitudeAudio
{
    // ========================================================================
    //  AudioControlType
    // ========================================================================

    /// \brief Discriminator for entries stored inside an AudioControlCollection.
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

    // ========================================================================
    //  AudioControlEntry
    // ========================================================================

    /// \brief A single named audio control stored inside an AudioControlCollection.
    ///
    /// Exactly one of \c AmplitudeId or \c AmplitudeName needs to be set,
    /// depending on whether the backing Amplitude asset is looked up by
    /// numeric ID or by string name.
    struct AudioControlEntry
    {
        /// Human-readable label for this control (used in editor UI and logging).
        String Name;

        /// Discriminator that determines which Amplitude data type this entry maps to.
        AudioControlType Type = AudioControlType::Invalid;

        /// Numeric Amplitude asset ID (e.g. AmEventID, AmRtpcID).
        /// Use 0 when the entry is identified by \c AmplitudeName instead.
        uint64 AmplitudeId = 0;

        /// String name used by Amplitude for name-based lookup (e.g. "init.ambank").
        /// Leave empty when the entry is identified by \c AmplitudeId instead.
        String AmplitudeName;
    };

    // ========================================================================
    //  AudioControlEntryDeserializer
    // ========================================================================

    /// \brief Static helpers that construct Amplitude data wrappers from
    ///        AudioControlEntry values.
    ///
    /// Each method allocates a new wrapper on the heap; the caller takes
    /// ownership and is responsible for deleting the returned pointer.
    struct AudioControlEntryDeserializer
    {
        /// \brief Creates an AmplitudeTriggerData from a Trigger entry.
        /// \param entry  Must have Type == AudioControlType::Trigger.
        /// \return Heap-allocated wrapper; caller owns the pointer.
        static AmplitudeTriggerData* DeserializeTriggerEntry(const AudioControlEntry& entry);

        /// \brief Creates an AmplitudeRtpcData from an Rtpc entry.
        /// \param entry  Must have Type == AudioControlType::Rtpc.
        /// \return Heap-allocated wrapper; caller owns the pointer.
        static AmplitudeRtpcData* DeserializeRtpcEntry(const AudioControlEntry& entry);

        /// \brief Creates an AmplitudeSwitchStateData from a SwitchState entry.
        /// \param entry  Must have Type == AudioControlType::SwitchState.
        /// \return Heap-allocated wrapper; caller owns the pointer.
        static AmplitudeSwitchStateData* DeserializeSwitchStateEntry(const AudioControlEntry& entry);

        /// \brief Creates an AmplitudeEnvironmentData from an Environment entry.
        /// \param entry  Must have Type == AudioControlType::Environment.
        /// \return Heap-allocated wrapper; caller owns the pointer.
        static AmplitudeEnvironmentData* DeserializeEnvironmentEntry(const AudioControlEntry& entry);

        /// \brief Creates an AmplitudeBankData from a SoundBank entry.
        /// \param entry  Must have Type == AudioControlType::SoundBank.
        /// \return Heap-allocated wrapper; caller owns the pointer.
        static AmplitudeBankData* DeserializeBankEntry(const AudioControlEntry& entry);

    private:
        // Non-instantiable utility struct.
        AudioControlEntryDeserializer() = delete;
    };

    // ========================================================================
    //  AudioControlCollection
    // ========================================================================

    /// \brief An ordered collection of AudioControlEntry values loaded from JSON.
    ///
    /// Typical usage:
    /// \code
    ///   AudioControlCollection collection;
    ///   collection.LoadFromJson(jsonRootValue);
    ///   for (const AudioControlEntry& e : collection.GetEntries())
    ///       /* process e */;
    /// \endcode
    class AudioControlCollection
    {
    public:
        AudioControlCollection() = default;
        ~AudioControlCollection() = default;

        // ====================================================================
        //  Data access
        // ====================================================================

        /// \brief Returns a const reference to the stored entries.
        [[nodiscard]] const Array<AudioControlEntry>& GetEntries() const;

        /// \brief Removes all entries from the collection.
        void Clear();

        // ====================================================================
        //  JSON loading
        // ====================================================================

        /// \brief Populates this collection from a rapidjson Value node.
        ///
        /// The node must be the root JSON object containing a "Controls" array.
        /// Each element of the array may have the following fields:
        ///   - "Name"          (string, required)
        ///   - "Type"          (string, required) — one of the AudioControlType names
        ///   - "AmplitudeId"   (uint64, optional)
        ///   - "AmplitudeName" (string, optional)
        ///
        /// Unknown type strings result in an entry with Type == AudioControlType::Invalid.
        /// Malformed or missing optional fields are silently skipped.
        ///
        /// Any previously loaded entries are replaced.
        ///
        /// \param node  The root JSON value to read from.
        void LoadFromJson(const JsonTools::Value& node);

    private:
        Array<AudioControlEntry> m_entries;
    };

} // namespace AmplitudeAudio

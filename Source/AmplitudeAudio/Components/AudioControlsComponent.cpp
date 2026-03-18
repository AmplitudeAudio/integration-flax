#include <Engine/Core/Log.h>

#include "AudioControlsComponent.h"

// ============================================================================
//  AudioControlsComponent — Constructor
// ============================================================================

AudioControlsComponent::AudioControlsComponent(const SpawnParams& params)
    : AmplitudeComponent(params)
{
}

// ============================================================================
//  AudioControlsComponent — Script lifecycle
// ============================================================================

void AudioControlsComponent::OnBeginPlay()
{
    Script::OnBeginPlay();

    if (AutoLoad)
    {
        Load();
    }
}

void AudioControlsComponent::OnEndPlay()
{
    if (_loaded)
    {
        Unload();
    }

    Script::OnEndPlay();
}

// ============================================================================
//  AudioControlsComponent — Load
// ============================================================================

void AudioControlsComponent::Load()
{
    if (_loaded)
    {
        LOG(Warning, "[AudioControlsComponent] Load() called but controls are already loaded. Path: {0}", ControlsAssetPath);
        return;
    }

    if (ControlsAssetPath.IsEmpty())
    {
        LOG(Warning, "[AudioControlsComponent] Load() called but ControlsAssetPath is empty. Nothing to load.");
        return;
    }

    // TODO: Resolve ControlsAssetPath against the Flax content system and read
    //       the file contents (e.g. via FileSystem::ReadAllText or a ContentManager
    //       asset lookup).

    // TODO: Parse the file contents into a rapidjson Document and call
    //       AudioControlCollection::LoadFromJson() to populate the collection.

    // TODO: Iterate over AudioControlCollection::GetEntries() and register each
    //       control with the AudioSystem via the appropriate request type
    //       (e.g. LoadTrigger, LoadBank, SetRtpc, etc.) once the runtime
    //       integration layer is in place.

    LOG(Info, "[AudioControlsComponent] Controls loaded from: {0}", ControlsAssetPath);
    _loaded = true;
}

// ============================================================================
//  AudioControlsComponent — Unload
// ============================================================================

void AudioControlsComponent::Unload()
{
    if (!_loaded)
    {
        LOG(Warning, "[AudioControlsComponent] Unload() called but controls are not loaded.");
        return;
    }

    // TODO: Iterate over the previously loaded AudioControlCollection entries
    //       and unregister each control from the AudioSystem (e.g. UnloadBank,
    //       DestroyTriggerData, etc.) once the runtime integration layer is in
    //       place.

    LOG(Info, "[AudioControlsComponent] Controls unloaded for: {0}", ControlsAssetPath);
    _loaded = false;
}

// ============================================================================
//  AudioControlsComponent — IsLoaded
// ============================================================================

bool AudioControlsComponent::IsLoaded() const
{
    return _loaded;
}

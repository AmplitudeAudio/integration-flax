#include <Engine/Core/Log.h>

#include <AudioSystem/Core/AudioSystem.h>

#include "AmplitudeAudioPlugin.h"
#include "AmplitudeMiddleware.h"
#include "Core/Common.h"

// ============================================================================
//  AmplitudeAudioPlugin — Constructor
// ============================================================================

AmplitudeAudioPlugin::AmplitudeAudioPlugin(const SpawnParams& params)
    : GamePlugin(params)
{
    _description.Name        = TEXT("Amplitude Audio");
    _description.Category    = TEXT("Audio");
    _description.Author      = TEXT("Amplitude Audio");
    _description.Description = TEXT("Amplitude Audio SDK integration for Flax Engine.");
    _description.RepositoryUrl = TEXT("https://github.com/AmplitudeAudio/integration-flax");
}

// ============================================================================
//  AmplitudeAudioPlugin — Initialize
// ============================================================================

void AmplitudeAudioPlugin::Initialize()
{
    GamePlugin::Initialize();

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr)
    {
        LOG(Error, "[AmplitudeAudioPlugin] Initialize: AudioSystem is not available. "
                   "Ensure the AudioSystem plugin is loaded before AmplitudeAudio.");
        return;
    }

    _middleware = New<AmplitudeAudio::AmplitudeMiddleware>();
    audioSystem->RegisterMiddleware(_middleware);

    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware registered.");
}

// ============================================================================
//  AmplitudeAudioPlugin — Deinitialize
// ============================================================================

void AmplitudeAudioPlugin::Deinitialize()
{
    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem != nullptr)
    {
        audioSystem->UnregisterMiddleware();
    }

    if (_middleware != nullptr)
    {
        Delete(_middleware);
        _middleware = nullptr;
    }

    LOG(Info, "[AmplitudeAudioPlugin] Amplitude Audio middleware unregistered.");

    GamePlugin::Deinitialize();
}

// ============================================================================
//  AmplitudeAudioPlugin — Metadata
// ============================================================================

String AmplitudeAudioPlugin::GetName() const
{
    return TEXT("Amplitude Audio");
}

String AmplitudeAudioPlugin::GetDescription() const
{
    return TEXT("Amplitude Audio SDK integration for Flax Engine.");
}

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

#include "Amplitude.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Core/Math/Vector3.h>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>
#include <AmplitudeAudio/Types/AmplitudeChannel.h>
#include <AmplitudeAudio/Types/AmplitudeEventHandle.h>

using AmplitudeAudio::AmplitudeService;
using SparkyStudios::Audio::Amplitude::AmObjectID;
using SparkyStudios::Audio::Amplitude::Channel;
using SparkyStudios::Audio::Amplitude::EventCanceler;

namespace
{
    AmplitudeChannel* InvalidChannel()
    {
        return AmplitudeChannel::Create(Channel());
    }

    AmplitudeEventHandle* InvalidHandle()
    {
        return AmplitudeEventHandle::Create(EventCanceler());
    }
}

bool Amplitude::Startup()
{
    return AmplitudeService::Get().Startup();
}

void Amplitude::Shutdown()
{
    AmplitudeService::Get().Shutdown();
}

bool Amplitude::IsRunning()
{
    return AmplitudeService::Get().IsRunning();
}

bool Amplitude::LoadSoundBank(const AmplitudeSoundBankRef& bank, bool wait)
{
    return AmplitudeService::Get().LoadSoundBank(bank.File, wait);
}

void Amplitude::UnloadSoundBank(const AmplitudeSoundBankRef& bank)
{
    AmplitudeService::Get().UnloadSoundBank(bank.File);
}

bool Amplitude::IsSoundBankLoaded(const AmplitudeSoundBankRef& bank)
{
    return AmplitudeService::Get().IsSoundBankLoaded(bank.File);
}

AmplitudeChannel* Amplitude::Play(const AmplitudeSoundObjectRef& sound)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !sound.IsValid())
        return InvalidChannel();
    return AmplitudeChannel::Create(engine->Play(static_cast<AmObjectID>(sound.ID)));
}

AmplitudeChannel* Amplitude::Play(const AmplitudeSoundObjectRef& sound, AmplitudeEntity* entity)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !sound.IsValid() || !entity || !entity->IsValid())
        return InvalidChannel();
    return AmplitudeChannel::Create(engine->Play(static_cast<AmObjectID>(sound.ID), entity->GetHandle()));
}

AmplitudeChannel* Amplitude::PlayAt(const AmplitudeSoundObjectRef& sound, const Vector3& position)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !sound.IsValid())
        return InvalidChannel();
    return AmplitudeChannel::Create(engine->Play(static_cast<AmObjectID>(sound.ID), AmplitudeAudio::FlaxVec3ToAmVec3(position)));
}

AmplitudeEventHandle* Amplitude::TriggerEvent(const AmplitudeEventRef& ev, AmplitudeEntity* entity)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !ev.IsValid() || !entity || !entity->IsValid())
        return InvalidHandle();
    return AmplitudeEventHandle::Create(engine->Trigger(static_cast<SparkyStudios::Audio::Amplitude::AmEventID>(ev.ID), entity->GetHandle()));
}

void Amplitude::CancelEvents(AmplitudeEntity* entity)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !entity || !entity->IsValid())
        return;
    engine->CancelEvents(entity->GetHandle());
}

void Amplitude::CancelAllEvents()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;
    engine->CancelAllEvents();
}

void Amplitude::SetRtpcValue(const AmplitudeRtpcRef& rtpc, double value)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !rtpc.IsValid())
        return;
    engine->SetRtpcValue(static_cast<SparkyStudios::Audio::Amplitude::AmRtpcID>(rtpc.ID), value);
}

double Amplitude::GetRtpcValue(const AmplitudeRtpcRef& rtpc)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !rtpc.IsValid())
        return 0.0;
    auto* handle = engine->GetRtpcHandle(static_cast<SparkyStudios::Audio::Amplitude::AmRtpcID>(rtpc.ID));
    if (!handle)
        return 0.0;
    return handle->GetValue();
}

void Amplitude::ResetRtpcValue(const AmplitudeRtpcRef& rtpc)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !rtpc.IsValid())
        return;
    auto* handle = engine->GetRtpcHandle(static_cast<SparkyStudios::Audio::Amplitude::AmRtpcID>(rtpc.ID));
    if (handle)
        handle->Reset();
}

void Amplitude::SetSwitchState(const AmplitudeSwitchRef& switchRef, const AmplitudeSwitchStateRef& state)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !switchRef.IsValid() || !state.IsValid())
        return;
    engine->SetSwitchState(static_cast<SparkyStudios::Audio::Amplitude::AmSwitchID>(switchRef.ID), static_cast<AmObjectID>(state.ID));
}

AmplitudeSwitchStateRef Amplitude::GetSwitchState(const AmplitudeSwitchRef& switchRef)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine || !switchRef.IsValid())
        return AmplitudeSwitchStateRef();
    auto* handle = engine->GetSwitchHandle(static_cast<SparkyStudios::Audio::Amplitude::AmSwitchID>(switchRef.ID));
    if (!handle)
        return AmplitudeSwitchStateRef();
    AmplitudeSwitchStateRef result;
    result.ID = static_cast<uint64>(handle->GetState().m_id);
    return result;
}

void Amplitude::SetMasterGain(float value)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;
    engine->SetMasterGain(static_cast<double>(value));
}

float Amplitude::GetMasterGain()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return 1.0f;
    return static_cast<float>(engine->GetMasterGain());
}

void Amplitude::SetIsMuted(bool value)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;
    engine->SetMute(value);
}

bool Amplitude::GetIsMuted()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return false;
    return engine->IsMuted();
}

void Amplitude::SetIsPaused(bool value)
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;
    engine->Pause(value);
}

bool Amplitude::GetIsPaused()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return false;
    return engine->IsPaused();
}

void Amplitude::StopAll()
{
    auto* engine = AmplitudeService::Get().GetEngine();
    if (!engine)
        return;
    engine->StopAll();
}

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

#include "AmplitudeChannel.h"

#include <AmplitudeAudio/Core/Helpers.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

namespace
{
    std::atomic<int> s_live{ 0 };
    using SparkyStudios::Audio::Amplitude::Channel;

    // Converts SDK playback state to the scripting enum
    AmplitudeChannelState ToChannelState(SparkyStudios::Audio::Amplitude::eChannelPlaybackState state)
    {
        switch (state)
        {
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_Stopped: return AmplitudeChannelState::Stopped;
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_Playing: return AmplitudeChannelState::Playing;
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_FadingIn: return AmplitudeChannelState::FadingIn;
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_FadingOut: return AmplitudeChannelState::FadingOut;
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_SwitchingState: return AmplitudeChannelState::SwitchingState;
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_Paused: return AmplitudeChannelState::Paused;
        case SparkyStudios::Audio::Amplitude::eChannelPlaybackState_Pending: return AmplitudeChannelState::Pending;
        default: return AmplitudeChannelState::Stopped;
        }
    }

    void HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent event, Delegate<> AmplitudeChannel::*target,
        const std::shared_ptr<AmplitudeChannel::CallbackState>& state, const Channel& ch)
    {
        ch.On(event, [state, target](SparkyStudios::Audio::Amplitude::ChannelEventInfo)
        {
            AmplitudeAudio::AmplitudeService::Get().Post([state, target]
            {
                if (state->Owner)
                    (state->Owner->*target)();
            });
        });
    }
} // namespace

AmplitudeChannel* AmplitudeChannel::Create(const Channel& channel)
{
    auto* wrapper = ::New<AmplitudeChannel>(SpawnParams(Guid::New(), AmplitudeChannel::TypeInitializer));
    ++s_live;
    wrapper->_channel = channel;
    wrapper->_state = std::make_shared<CallbackState>(CallbackState{ wrapper });
    if (!channel.Valid())
        return wrapper;

    // Channel::Valid() is only a null check, so the wrapper must drop the handle itself when the engine stops.
    AmplitudeAudio::AmplitudeService::Get().AddClient(wrapper);

    HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent_Begin, &AmplitudeChannel::OnBegin, wrapper->_state, channel);
    HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent_End, &AmplitudeChannel::OnEnd, wrapper->_state, channel);
    HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent_Pause, &AmplitudeChannel::OnPause, wrapper->_state, channel);
    HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent_Resume, &AmplitudeChannel::OnResume, wrapper->_state, channel);
    HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent_Stop, &AmplitudeChannel::OnStop, wrapper->_state, channel);
    HookChannelEvent(SparkyStudios::Audio::Amplitude::eChannelEvent_Loop, &AmplitudeChannel::OnLoop, wrapper->_state, channel);
    return wrapper;
}

AmplitudeChannel::~AmplitudeChannel()
{
    --s_live;
    AmplitudeAudio::AmplitudeService::Get().RemoveClient(this);
    if (_state)
        _state->Owner = nullptr;
}

void AmplitudeChannel::OnEngineStarted()
{
}

void AmplitudeChannel::OnEngineStopping()
{
    _channel = Channel();
}

int AmplitudeChannel::GetLiveCount()
{
    return s_live.load();
}

bool AmplitudeChannel::IsValid() const
{
    return _channel.Valid();
}

bool AmplitudeChannel::IsPlaying() const
{
    return _channel.Valid() && _channel.GetPlaybackState() == SparkyStudios::Audio::Amplitude::eChannelPlaybackState_Playing;
}

AmplitudeChannelState AmplitudeChannel::GetState() const
{
    if (!_channel.Valid())
        return AmplitudeChannelState::Stopped;
    return ToChannelState(_channel.GetPlaybackState());
}

float AmplitudeChannel::GetGain() const
{
    if (!_channel.Valid())
        return 1.0f;
    return static_cast<float>(_channel.GetGain());
}

void AmplitudeChannel::SetGain(float value)
{
    if (_channel.Valid())
        _channel.SetGain(static_cast<double>(value));
}

float AmplitudeChannel::GetPlaybackPosition() const
{
    if (!_channel.Valid())
        return 0.0f;
    // SDK returns milliseconds; convert to seconds
    return static_cast<float>(_channel.GetPlaybackPosition()) / 1000.0f;
}

Vector3 AmplitudeChannel::GetLocation() const
{
    if (!_channel.Valid())
        return Vector3::Zero;
    return AmplitudeAudio::AmVec3ToFlaxVec3(_channel.GetLocation());
}

void AmplitudeChannel::SetLocation(const Vector3& value)
{
    if (_channel.Valid())
        _channel.SetLocation(AmplitudeAudio::FlaxVec3ToAmVec3(value));
}

void AmplitudeChannel::Stop(float fadeSeconds)
{
    if (!_channel.Valid())
        return;
    if (fadeSeconds == 0.0f)
        _channel.Stop();
    else
        _channel.Stop(AmplitudeAudio::SecondsToAmTime(fadeSeconds));
}

void AmplitudeChannel::Pause(float fadeSeconds)
{
    if (!_channel.Valid())
        return;
    if (fadeSeconds == 0.0f)
        _channel.Pause();
    else
        _channel.Pause(AmplitudeAudio::SecondsToAmTime(fadeSeconds));
}

void AmplitudeChannel::Resume(float fadeSeconds)
{
    if (!_channel.Valid())
        return;
    if (fadeSeconds == 0.0f)
        _channel.Resume();
    else
        _channel.Resume(AmplitudeAudio::SecondsToAmTime(fadeSeconds));
}

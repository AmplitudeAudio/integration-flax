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

#include "AmplitudeEventHandle.h"

#include <AmplitudeAudio/Runtime/AmplitudeService.h>

using AmplitudeAudio::AmplitudeService;
using SparkyStudios::Audio::Amplitude::EventCanceler;

AmplitudeEventHandle* AmplitudeEventHandle::Create(const EventCanceler& canceler)
{
    auto* wrapper = ScriptingObject::NewObject<AmplitudeEventHandle>();
    wrapper->_canceler = canceler;
    wrapper->_state = std::make_shared<CallbackState>(CallbackState{ wrapper });
    if (!canceler.Valid())
        return wrapper;

    ForwardFinish(canceler, [state = wrapper->_state](bool aborted)
    {
        AmplitudeAudio::AmplitudeService::Get().Post([state, aborted]
        {
            if (state->Owner)
                state->Owner->OnFinished(aborted);
        });
    });
    return wrapper;
}

AmplitudeEventHandle::~AmplitudeEventHandle()
{
    if (_state)
        _state->Owner = nullptr;
}

bool AmplitudeEventHandle::IsValid() const
{
    return _canceler.Valid();
}

void AmplitudeEventHandle::Cancel()
{
    if (_canceler.Valid())
        _canceler.Cancel();
}

void AmplitudeEventHandle::ForwardFinish(const EventCanceler& canceler,
    std::function<void(bool aborted)> callback)
{
    // GetEvent() returns a shared_ptr<EventInstance>; when the SDK forwards OnFinish itself,
    // only this helper's body needs to change.
    auto event = canceler.GetEvent();
    if (event)
        event->OnFinish(callback);
}

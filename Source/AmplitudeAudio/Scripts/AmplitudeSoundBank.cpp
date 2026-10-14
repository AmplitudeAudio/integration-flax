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

#include "AmplitudeSoundBank.h"

#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>

#include <AmplitudeAudio/Amplitude.h>

namespace
{
    AmplitudeSoundBankRef MakeRef(const String& file)
    {
        AmplitudeSoundBankRef ref;
        ref.File = file;
        return ref;
    }
} // namespace

AmplitudeSoundBank::AmplitudeSoundBank(const SpawnParams& params)
    : Script(params)
{
}

bool AmplitudeSoundBank::IsLoaded() const
{
    return _acquiredFile.HasChars() && Amplitude::IsSoundBankLoaded(MakeRef(_acquiredFile));
}

void AmplitudeSoundBank::OnEnable()
{
    if (_acquiredFile.HasChars())
        return;

    if (Bank.File.IsEmpty())
    {
        LOG(Warning, "[Amplitude] AmplitudeSoundBank on '{0}' has no bank set.", GetActor() ? GetActor()->GetName() : String::Empty);
        return;
    }

    if (Amplitude::LoadSoundBank(Bank, !LoadAsync))
        _acquiredFile = Bank.File;
}

void AmplitudeSoundBank::OnDisable()
{
    if (_acquiredFile.IsEmpty())
        return;

    Amplitude::UnloadSoundBank(MakeRef(_acquiredFile));
    _acquiredFile.Clear();
}

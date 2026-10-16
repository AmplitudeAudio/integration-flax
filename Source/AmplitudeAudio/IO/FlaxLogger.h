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

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

namespace AmplitudeAudio
{
    using namespace SparkyStudios::Audio::Amplitude;

    /// <summary>
    /// Amplitude Logger implementation that writes to Flax's log system.
    /// </summary>
    /// <remarks>
    /// Level mapping:
    ///   Debug / Info / Success -> LogType::Info
    ///   Warning                -> LogType::Warning
    ///   Error                  -> LogType::Error
    ///   Critical               -> LogType::Fatal
    /// </remarks>
    class FlaxLogger final : public Logger
    {
    protected:
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override;
    };
}
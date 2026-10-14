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

#include "FlaxLogger.h"

#include <Engine/Core/Log.h>
#include <Engine/Platform/Platform.h>

namespace AmplitudeAudio
{
    void FlaxLogger::Log(eLogMessageLevel level, const char* /*file*/, int /*line*/, const AmString& message)
    {
        // Convert the narrow AmString (std::string) to a Flax StringView for
        // the LOG_STR macro. We build a wide String on the stack.
        const ::String flaxMessage(message.c_str());

        switch (level)
        {
            case eLogMessageLevel_Debug:
            case eLogMessageLevel_Info:
            case eLogMessageLevel_Success:
                LOG_STR(Info, flaxMessage);
                break;

            case eLogMessageLevel_Warning:
                LOG_STR(Warning, flaxMessage);
                break;

            case eLogMessageLevel_Error:
                LOG_STR(Error, flaxMessage);
                break;

            case eLogMessageLevel_Critical:
                LOG_STR(Fatal, flaxMessage);
                break;

            default:
                LOG_STR(Info, flaxMessage);
                break;
        }
    }
} // namespace AmplitudeAudio
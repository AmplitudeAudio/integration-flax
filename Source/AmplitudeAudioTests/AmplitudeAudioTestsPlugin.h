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

#include <Engine/Scripting/Plugins/EditorPlugin.h>

/// <summary>Runs the native tests when the editor is started with AMPLITUDE_RUN_TESTS=1, then exits with the result.</summary>
/// <remarks>An editor plugin because the editor initializes game plugins only during Play.</remarks>
API_CLASS()
class AMPLITUDEAUDIOTESTS_API AmplitudeAudioTestsPlugin : public EditorPlugin
{
    DECLARE_SCRIPTING_TYPE(AmplitudeAudioTestsPlugin);

    void Initialize() override;
    void Deinitialize() override;
};

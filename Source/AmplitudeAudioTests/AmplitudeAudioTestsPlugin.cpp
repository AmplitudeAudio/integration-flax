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

#include "AmplitudeAudioTestsPlugin.h"

#include <Engine/Core/Log.h>
#include <Engine/Engine/Engine.h>
#include <Engine/Engine/Globals.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Scripting/Scripting.h>

#include <string>
#include <vector>

#define CATCH_CONFIG_RUNNER
#include <ThirdParty/catch2/catch.hpp>

// Catch2 includes <windows.h> after Flax's headers, which redefines GetEnvironmentVariable as a macro.
#undef GetEnvironmentVariable

namespace
{
    bool IsRunRequested()
    {
        String value;
        return !Platform::GetEnvironmentVariable(TEXT("AMPLITUDE_RUN_TESTS"), value) && value == TEXT("1");
    }

    void RunTests()
    {
        // Same readiness check as the engine's TestMain.cpp: tests may touch scripting types.
        if (!Scripting::IsEveryAssemblyLoaded() || !Scripting::HasGameModulesLoaded())
            return;

        Engine::Update.Unbind<&RunTests>();

        Catch::Session session;

        // Without -w NoTests, Catch returns 0 when the filter matches no test case.
        String filter;
        StringAnsi filterAnsi;
        std::vector<const char*> argv = {"AmplitudeAudioTests", "-w", "NoTests"};
        if (!Platform::GetEnvironmentVariable(TEXT("AMPLITUDE_TESTS_FILTER"), filter) && filter.HasChars())
        {
            filterAnsi = filter.ToStringAnsi();
            argv.push_back(filterAnsi.Get());
        }

        const int parseResult = session.applyCommandLine(static_cast<int>(argv.size()), argv.data());
        if (parseResult != 0)
        {
            LOG(Error, "Invalid Amplitude Audio tests arguments: {0}", parseResult);
            Engine::RequestExit(parseResult);
            return;
        }

        // Catch writes its report to this file; the runner script prints it, since the editor's stdout is not reliable.
        const String reportPath = Globals::ProjectFolder / TEXT("Logs") / TEXT("AmplitudeAudioTests.txt");
        const StringAnsi reportPathAnsi = reportPath.ToStringAnsi();
        session.configData().outputFilename = std::string(reportPathAnsi.Get());

        LOG(Info, "Running Amplitude Audio tests...");
        const int result = session.run();
        if (result == 0)
            LOG(Info, "Amplitude Audio tests result: {0}", result);
        else
            LOG(Error, "Amplitude Audio tests result: {0}", result);

        Engine::RequestExit(result);
    }
} // namespace

AmplitudeAudioTestsPlugin::AmplitudeAudioTestsPlugin(const SpawnParams& params)
    : EditorPlugin(params)
{
    _description.Name        = TEXT("Amplitude Audio Tests");
    _description.Category    = TEXT("Audio");
    _description.Author      = TEXT("Sparky Studios");
    _description.Description = TEXT("Native tests for the Amplitude Audio plugin.");
}

void AmplitudeAudioTestsPlugin::Initialize()
{
    EditorPlugin::Initialize();

    if (IsRunRequested())
        Engine::Update.Bind<&RunTests>();
}

void AmplitudeAudioTestsPlugin::Deinitialize()
{
    Engine::Update.Unbind<&RunTests>();
    EditorPlugin::Deinitialize();
}

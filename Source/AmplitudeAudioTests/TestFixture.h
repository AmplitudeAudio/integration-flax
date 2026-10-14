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

#include <initializer_list>

#include <Engine/Core/Types/String.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Platform/StringUtils.h>

#include <AmplitudeAudio/AmplitudeSettings.h>
#include <AmplitudeAudio/Runtime/AmplitudeService.h>

namespace AmplitudeAudioTests
{
    /// <summary>Splits AMPLITUDE_TEST_PROJECT into the root folder and build folder name the service expects.</summary>
    inline bool GetTestProject(String& root, String& buildPath)
    {
        String project;
        if (Platform::GetEnvironmentVariable(TEXT("AMPLITUDE_TEST_PROJECT"), project) || project.IsEmpty())
            return false;

        project.Replace('\\', '/');
        while (project.EndsWith(StringView(TEXT("/"))))
            project = project.Left(project.Length() - 1);

        root      = String(StringUtils::GetDirectoryName(project));
        buildPath = String(StringUtils::GetFileName(project));
        return true;
    }

    /// <summary>Folder holding am_build.ampk, prepared by Tests/run-native-tests.ps1.</summary>
    inline bool GetTestPackageRoot(String& root)
    {
        return !Platform::GetEnvironmentVariable(TEXT("AMPLITUDE_TEST_PACKAGE_ROOT"), root) && root.HasChars();
    }

    inline AmplitudePlatformProfile MakeProfile(const Char* config, std::initializer_list<const Char*> initBanks = {})
    {
        AmplitudePlatformProfile profile;
        profile.EngineConfigFile = config;
        for (const Char* bank : initBanks)
        {
            AmplitudeSoundBankRef ref;
            ref.File = bank;
            profile.InitSoundBanks.Add(ref);
        }
        return profile;
    }

    /// <summary>Stops the service at scope exit so a failing test can't leak a running engine into the next one.</summary>
    struct ServiceGuard
    {
        ~ServiceGuard()
        {
            AmplitudeAudio::AmplitudeService::Get().Shutdown();
        }
    };
} // namespace AmplitudeAudioTests

#define REQUIRE_TEST_PROJECT(root, buildPath)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!AmplitudeAudioTests::GetTestProject(root, buildPath))                                                     \
            FAIL("Set AMPLITUDE_TEST_PROJECT to a built Amplitude project (see README, Running Tests).");          \
    } while (false)

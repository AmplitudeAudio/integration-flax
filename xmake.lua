-- Copyright (c) 2026-present Sparky Studios. All rights reserved.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     `http://www.apache.org/licenses/LICENSE-2.0`
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- Test runners only: the plugin itself is built by Flax.Build.
--
-- Environment:
--   FLAX_ENGINE_PATH        Root of the Flax Engine used to build this plugin (required by test-native).
--   AMPLITUDE_TEST_PROJECT  A built Amplitude project used as fixture by the service tests (optional).
--   AM_SDK_PATH             Amplitude SDK installation; its ampk tool packs the fixture (required with AMPLITUDE_TEST_PROJECT).

set_project("AmplitudeAudio")
set_xmakever("2.8.5")

task("test-native")
    set_category("plugin")
    on_run(function ()
        import("core.base.option")
        import("core.base.process")

        local engine = os.getenv("FLAX_ENGINE_PATH")
        if not engine or not os.isdir(engine) then
            raise("Set FLAX_ENGINE_PATH to the root of the Flax Engine used to build this plugin.")
        end

        local platform, editorPlatform, exe = "Linux", "Linux", ""
        if is_host("windows") then
            platform, editorPlatform, exe = "Windows", "Win64", ".exe"
        elseif is_host("macosx") then
            platform, editorPlatform = "Mac", "Mac"
        end

        local hostArch = os.arch()
        local arch = (hostArch == "arm64" or hostArch == "aarch64") and "ARM64" or "x64"
        local configuration = option.get("configuration")
        local repo = os.projectdir()

        local testProject = os.getenv("AMPLITUDE_TEST_PROJECT")
        local ampk
        if testProject then
            local sdk = os.getenv("AM_SDK_PATH")
            ampk = sdk and path.join(sdk, "bin", "ampk" .. exe)
            if not ampk or not os.isfile(ampk) then
                raise("AMPLITUDE_TEST_PROJECT is set, but the ampk tool was not found. Set AM_SDK_PATH to an Amplitude SDK installation containing bin/ampk%s.", exe)
            end
        end

        os.execv("dotnet", {path.join(engine, "Binaries", "Tools", "Flax.Build.dll"),
            "-build", "-log", "-mutex", "-workspace=" .. repo, "-buildtargets=AmplitudeAudioEditorTarget",
            "-platform=" .. platform, "-arch=" .. arch, "-configuration=" .. configuration})

        -- Package fixture for the package file system tests: the fixture build packed next to an otherwise empty root.
        if testProject then
            local packageRoot = path.join(os.tmpdir(), "amplitude-audio-tests")
            os.mkdir(packageRoot)
            os.execv(ampk, {"-q", testProject, path.join(packageRoot, "am_build.ampk")})
            os.setenv("AMPLITUDE_TEST_PACKAGE_ROOT", packageRoot)
        end

        -- Catch writes its report here: the editor's stdout does not reliably reach the console.
        local report = path.join(repo, "Logs", "AmplitudeAudioTests.txt")
        os.tryrm(report)

        os.setenv("AMPLITUDE_RUN_TESTS", "1")
        os.setenv("AMPLITUDE_TESTS_FILTER", option.get("filter") or "")

        local editor = path.join(engine, "Binaries", "Editor", editorPlatform, configuration, "FlaxEditor" .. exe)
        -- -project=<path> as one argument: Flax's parser stops a bare value at '-', which would split "integration-flax".
        -- -skipcompile: the build above already ran; a compile error in a headless editor would never exit.
        local proc = process.openv(editor, {"-std", "-headless", "-null", "-mute", "-skipcompile", "-project=" .. repo})
        local ok, status = proc:wait(tonumber(option.get("timeout")) * 1000)
        if ok ~= 1 then
            proc:kill()
            proc:close()
            raise("Native tests timed out after %s s.", option.get("timeout"))
        end
        proc:close()

        -- A crash in the editor exits 0, so the exit code alone can't say whether the tests passed.
        if not os.isfile(report) then
            raise("Native tests produced no Catch report at %s; the editor likely crashed.", report)
        end
        local content = io.readfile(report)
        print(content)
        if status ~= 0 then
            raise("Native tests failed (editor exit code %d).", status)
        end
        if not content:find("All tests passed", 1, true) then
            raise("Native tests did not report 'All tests passed'.")
        end
    end)
    set_menu({
        usage = "xmake test-native [options]",
        description = "Build the plugin and run its native tests inside a headless Flax Editor.",
        options = {
            {"f", "filter", "kv", nil, "Catch test spec to run, e.g. \"[service]\"."},
            {nil, "configuration", "kv", "Development", "Flax build configuration of the plugin and the editor."},
            {nil, "timeout", "kv", "600", "Seconds to wait for the editor before failing."}
        }
    })
task_end()

task("test-editor")
    set_category("plugin")
    on_run(function ()
        os.execv("dotnet", {"test", path.join(os.projectdir(), "Tests", "editor", "AmplitudeAudioEditor.Tests.csproj")})
    end)
    set_menu({
        usage = "xmake test-editor",
        description = "Run the editor's .NET tests (no Flax required).",
        options = {}
    })
task_end()

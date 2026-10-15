// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <AmplitudeAudio/AmplitudeSettings.h>
#include <AmplitudeAudio/Runtime/Occlusion/IOcclusionSource.h>
#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Delegate.h>
#include <Engine/Core/Math/Vector2.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Platform/Defines.h>

namespace AmplitudeAudio
{
    class AMPLITUDEAUDIO_API AmplitudeOcclusionProcessor
    {
    public:
        /// <summary>Returns the combined amount (0..1) of everything between two points, for this source.</summary>
        using CastFunction = Function<float(IOcclusionSource&, const Vector3& from, const Vector3& to)>;

        /// <summary>Callback invoked for each ray cast (from, to, amount, isDirect).</summary>
        using RayObserver = Function<void(const Vector3& from, const Vector3& to, float amount, bool isDirect)>;

        void AddSource(IOcclusionSource* source);
        void RemoveSource(IOcclusionSource* source);
        int32 GetSourceCount() const;
        void Clear() { _states.Clear(); _lastRays = 0; }

        void Update(const Vector3& listener, float deltaSeconds, const AmplitudeObstructionSettings& settings, const CastFunction& cast);

        /// <summary>Rays cast by the last Update.</summary>
        int32 GetLastRayCount() const { return _lastRays; }

        /// <summary>Sets or clears an observer callback for cast rays (e.g. for debug drawing).</summary>
        void SetRayObserver(const RayObserver& observer) { _rayObserver = observer; }
        void ClearRayObserver() { _rayObserver.Unbind(); }

    private:
        struct SourceState
        {
            IOcclusionSource* Source = nullptr;
            float TargetObstruction = 0.0f;
            float TargetOcclusion = 0.0f;
            float Obstruction = 0.0f;
            float Occlusion = 0.0f;
            float WrittenObstruction = 0.0f;
            float WrittenOcclusion = 0.0f;
            float DirectAmount = 0.0f;
            float Distance = 0.0f;
            float Priority = 0.0f;
            int32 FramesSinceCast = 0;
            int32 Cursor = 0;
            Array<int8> IndirectOpen;
        };

        static bool ComparePriority(SourceState* const& a, SourceState* const& b)
        {
            return a->Priority > b->Priority;
        }

        Array<SourceState> _states;
        Array<Float2> _offsets;
        uint32 _seed = 0;
        int32 _lastRays = 0;
        RayObserver _rayObserver;
    };
} // namespace AmplitudeAudio

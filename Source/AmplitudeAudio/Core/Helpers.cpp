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

#include <AmplitudeAudio/Core/Helpers.h>

#include <Engine/Engine/Units.h>

#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>

namespace AmplitudeAudio
{
    AmVector3 FlaxVec3ToAmVec3(const Vector3& vec)
    {
        static const CartesianCoordinateSystem flax(
            CartesianCoordinateSystem::Axis::PositiveX,
            CartesianCoordinateSystem::Axis::PositiveZ,
            CartesianCoordinateSystem::Axis::PositiveY);

        const AmVector3 result = {vec.X * UNITS_TO_METERS_SCALE, vec.Y * UNITS_TO_METERS_SCALE, vec.Z * UNITS_TO_METERS_SCALE};
        return CartesianCoordinateSystem::ConvertToDefault(result, flax);
    }

    std::string FlaxStringToStdString(const ::String& value)
    {
        const ::StringAnsi ansi = value.ToStringAnsi();
        return std::string(ansi.Get(), ansi.Length());
    }

    AmOsString FlaxStringToAmOsString(const ::String& value)
    {
        const std::string narrow = FlaxStringToStdString(value);
        return AmOsString(AM_STRING_TO_OS_STRING(narrow));
    }

    ::String AmOsStringToFlaxString(const AmOsString& path)
    {
#ifdef AM_WCHAR_SUPPORTED
        const std::string narrow = am_wstring_narrow(path);
        return ::String(::StringAnsi(narrow.c_str()));
#else
        return ::String(::StringAnsi(path.c_str()));
#endif
    }
} // namespace AmplitudeAudio

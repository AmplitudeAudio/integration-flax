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

#include "AmplitudeSettings.h"

#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Config/GameSettings.h>
#include <Engine/Serialization/Serialization.h>

IMPLEMENT_GAME_SETTINGS_GETTER(AmplitudeSettings, "Amplitude Settings");

AmplitudePlatformProfile AmplitudeSettings::GetProfile(PlatformType platform) const
{
    for (const AmplitudePlatformProfile& profile : Profiles)
    {
        if (profile.Platform == platform)
            return profile;
    }

    return DefaultProfile;
}

void AmplitudeObstructionSettings::Serialize(SerializeStream& stream, const void* otherObj)
{
    SERIALIZE_GET_OTHER_OBJ(AmplitudeObstructionSettings);
    SERIALIZE(DefaultAmount);
    SERIALIZE(MaterialAmounts);
    SERIALIZE(TagAmounts);
    SERIALIZE(MaxDistance);
    SERIALIZE(FullObstructionDistance);
    SERIALIZE(RayBudgetPerFrame);
    SERIALIZE(IndirectRayCount);
    SERIALIZE(IndirectRadius);
    SERIALIZE(RandomSeed);
    SERIALIZE(AttackSmoothing);
    SERIALIZE(ReleaseSmoothing);
    SERIALIZE(DebugDraw);

    if (!other || Layers.Mask != other->Layers.Mask)
    {
        stream.JKEY("Layers");
        stream.StartObject();
        stream.JKEY("Mask");
        stream.Uint(Layers.Mask);
        stream.EndObject();
    }
}

void AmplitudeObstructionSettings::Deserialize(DeserializeStream& stream, ISerializeModifier* modifier)
{
    DESERIALIZE(DefaultAmount);
    DESERIALIZE(MaterialAmounts);
    DESERIALIZE(TagAmounts);
    DESERIALIZE(MaxDistance);
    DESERIALIZE(FullObstructionDistance);
    DESERIALIZE(RayBudgetPerFrame);
    DESERIALIZE(IndirectRayCount);
    DESERIALIZE(IndirectRadius);
    DESERIALIZE(RandomSeed);
    DESERIALIZE(AttackSmoothing);
    DESERIALIZE(ReleaseSmoothing);
    DESERIALIZE(DebugDraw);

    // Accepts the C# form {"Mask": n} and the native bare number; anything else keeps the default.
    const auto layers = SERIALIZE_FIND_MEMBER(stream, "Layers");
    if (layers != stream.MemberEnd())
    {
        const auto& value = layers->value;
        if (value.IsUint())
        {
            Layers.Mask = value.GetUint();
        }
        else if (value.IsObject())
        {
            const auto mask = value.FindMember("Mask");
            if (mask != value.MemberEnd() && mask->value.IsUint())
                Layers.Mask = mask->value.GetUint();
        }
    }
}


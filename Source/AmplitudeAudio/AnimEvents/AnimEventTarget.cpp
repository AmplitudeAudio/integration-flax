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

#include "AnimEventTarget.h"

// clang-format off
#include <AmplitudeAudio/Core/Cpp20Compat.h>
// clang-format on

#include <Engine/Content/Assets/SkinnedModel.h>
#include <Engine/Core/Log.h>
#include <Engine/Level/Actors/AnimatedModel.h>

#include <AmplitudeAudio/Actors/AmplitudeEntity.h>
#include <AmplitudeAudio/Amplitude.h>

namespace AmplitudeAudio
{
    AmplitudeEntity* FindAnimEventEntity(AnimatedModel* model, const ::StringView& entityName)
    {
        if (!model)
            return nullptr;
        for (Actor* actor = model->GetParent(); actor; actor = actor->GetParent())
        {
            auto* entity = ScriptingObject::Cast<AmplitudeEntity>(actor);
            if (entity && (entityName.IsEmpty() || entity->GetName() == entityName))
                return entity;
        }
        return nullptr;
    }

    AmplitudeEntity* ResolveTriggerEntity(AnimatedModel* model, const ::StringView& entityName, bool& reported, const Char* what)
    {
        AmplitudeEntity* entity = FindAnimEventEntity(model, entityName);
        if (!entity && !reported)
        {
            reported = true;
            LOG(Error, "Amplitude {0} animation event on '{1}' found no AmplitudeEntity ancestor (EntityName '{2}').", what, model ? model->GetName() : TEXT("<null>"), entityName);
        }
        return entity;
    }

    Vector3 GetAnimEventLocation(AnimatedModel* model, const ::StringView& boneName)
    {
        if (!model)
            return Vector3::Zero;
        if (boneName.HasChars() && model->SkinnedModel && model->SkinnedModel->FindNode(boneName) >= 0)
        {
            Matrix node;
            model->GetNodeTransformation(boneName, node, true);
            return node.GetTranslation();
        }
        return model->GetPosition();
    }

    bool HasAnimEventBone(AnimatedModel* model, const ::StringView& boneName)
    {
        return model && boneName.HasChars() && model->SkinnedModel && model->SkinnedModel->FindNode(boneName) >= 0;
    }

    AmplitudeChannel* PlayFromAnimEvent(AnimatedModel* model, const AmplitudeSoundObjectRef& sound, const ::StringView& entityName, const ::StringView& boneName)
    {
        AmplitudeEntity* entity = FindAnimEventEntity(model, entityName);
        if (entity && boneName.IsEmpty())
            return Amplitude::Play(sound, entity);
        return Amplitude::PlayAt(sound, GetAnimEventLocation(model, boneName));
    }
}
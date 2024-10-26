// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <Sound/AttenuationShapes.h>

#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>

#include <Core/EngineInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    AttenuationZoneImpl::AttenuationZoneImpl()
        : m_maxAttenuationFactor(1.0f)
    {}

    AmReal32 AttenuationZoneImpl::GetAttenuationFactor(const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener)
    {
        return m_maxAttenuationFactor;
    }

    AmReal32 AttenuationZoneImpl::GetAttenuationFactor(const Attenuation*, const Entity&, const Listener&)
    {
        return m_maxAttenuationFactor;
    }

    AttenuationZone* AttenuationZoneImpl::Create(const AttenuationShapeDefinition* definition)
    {
        if (definition == nullptr)
            return nullptr;

        AttenuationZoneImpl* shape = nullptr;

        switch (definition->zone_type())
        {
        default:
        case ZoneDefinition_Cone:
            shape = ampoolnew(eMemoryPoolKind_Engine, ConeAttenuationZone, definition->zone_as_Cone());
            break;
        case ZoneDefinition_Sphere:
            shape = ampoolnew(eMemoryPoolKind_Engine, SphereAttenuationZone, definition->zone_as_Sphere());
            break;
        case ZoneDefinition_Box:
            shape = ampoolnew(eMemoryPoolKind_Engine, BoxAttenuationZone, definition->zone_as_Box());
            break;
        case ZoneDefinition_Capsule:
            shape = ampoolnew(eMemoryPoolKind_Engine, CapsuleAttenuationZone, definition->zone_as_Capsule());
            break;
        }

        shape->m_maxAttenuationFactor = definition->max_attenuation_factor();

        return shape;
    }

    AmReal32 ConeAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        if (const AmVec3& soundToListener = listener.GetLocation() - soundLocation;
            AM_Len(soundToListener) >= attenuation->GetMaxDistance())
            return 0.0f;

        // Cone attenuation works only with PositionOrientation spatial sounds
        return 1.0f;
    }

    AmReal32 ConeAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetLocation(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetOrientation());

        m_outerShape->SetLocation(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetOrientation());

        const AmVec3& soundToListener = listener.GetLocation() - entity.GetLocation();
        const AmReal32 distance = AM_Len(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        if (!m_outerShape->Contains(listener.GetLocation()))
            return gain * m_maxAttenuationFactor;

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    AmReal32 SphereAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        const AmVec3& soundToListener = listener.GetLocation() - soundLocation;
        const AmReal32 distance = AM_Len(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    AmReal32 SphereAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        return GetAttenuationFactor(attenuation, entity.GetLocation(), listener);
    }

    AmReal32 BoxAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        const AmVec3& soundToListener = listener.GetLocation() - soundLocation;
        const AmReal32 distance = AM_Len(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    AmReal32 BoxAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetLocation(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetOrientation());

        m_outerShape->SetLocation(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetOrientation());

        const AmVec3& soundToListener = listener.GetLocation() - entity.GetLocation();
        const AmReal32 distance = AM_Len(soundToListener);
        if (distance >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(distance);

        return gain * AM_Lerp(m_maxAttenuationFactor, Zone::GetFactor(listener), 1.0f);
    }

    // AmReal32 BoxAttenuationZone::GetFactor(const AmVec3& soundLocation, const Listener& listener, AmMat4 lookAt)
    // {
    //     const auto* inner = static_cast<BoxShape*>(m_innerShape);
    //     const auto* outer = static_cast<BoxShape*>(m_outerShape);
    //     const eGameEngineUpAxis upAxis = amEngine->GetState()->up_axis;

    //     lookAt = AM_Mul(AM_Translate(soundLocation), lookAt);

    //     const AmVec3& x = listener.GetLocation();

    //     AmVec3 iP1, iP2, iP3, iP4, oP1, oP2, oP3, oP4;

    //     switch (upAxis)
    //     {
    //     default:
    //     case eGameEngineUpAxis_Y:
    //         iP1 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP2 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfHeight(), inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP3 = AM_Mul(lookAt, AM_V4(inner->GetHalfWidth(), -inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;
    //         iP4 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), inner->GetHalfHeight(), -inner->GetHalfDepth(), 1.0f)).XYZ;

    //         oP1 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP2 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfHeight(), outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP3 = AM_Mul(lookAt, AM_V4(outer->GetHalfWidth(), -outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         oP4 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), outer->GetHalfHeight(), -outer->GetHalfDepth(), 1.0f)).XYZ;
    //         break;

    //     case eGameEngineUpAxis_Z:
    //         iP1 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP2 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP3 = AM_Mul(lookAt, AM_V4(inner->GetHalfWidth(), -inner->GetHalfDepth(), -inner->GetHalfHeight(), 1.0f)).XYZ;
    //         iP4 = AM_Mul(lookAt, AM_V4(-inner->GetHalfWidth(), -inner->GetHalfDepth(), inner->GetHalfHeight(), 1.0f)).XYZ;

    //         oP1 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP2 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP3 = AM_Mul(lookAt, AM_V4(outer->GetHalfWidth(), -outer->GetHalfDepth(), -outer->GetHalfHeight(), 1.0f)).XYZ;
    //         oP4 = AM_Mul(lookAt, AM_V4(-outer->GetHalfWidth(), -outer->GetHalfDepth(), outer->GetHalfHeight(), 1.0f)).XYZ;
    //         break;
    //     }

    //     AmVec3 iU = AM_Norm(iP2 - iP1);
    //     AmVec3 iV = AM_Norm(iP3 - iP1);
    //     AmVec3 iW = AM_Norm(iP4 - iP1);

    //     AmVec3 oU = AM_Norm(oP2 - oP1);
    //     AmVec3 oV = AM_Norm(oP3 - oP1);
    //     AmVec3 oW = AM_Norm(oP4 - oP1);

    //     const AmReal32 iUX = AM_Dot(iU, x);
    //     const AmReal32 iVX = AM_Dot(iV, x);
    //     const AmReal32 iWX = AM_Dot(iW, x);

    //     const AmReal32 oUX = AM_Dot(oU, x);
    //     const AmReal32 oVX = AM_Dot(oV, x);
    //     const AmReal32 oWX = AM_Dot(oW, x);

    //     const AmReal32 iUP1 = AM_Dot(iU, iP1);
    //     const AmReal32 iVP1 = AM_Dot(iV, iP1);
    //     const AmReal32 iWP1 = AM_Dot(iW, iP1);

    //     const AmReal32 iUP2 = AM_Dot(iU, iP2);
    //     const AmReal32 iVP3 = AM_Dot(iV, iP3);
    //     const AmReal32 iWP4 = AM_Dot(iW, iP4);

    //     const AmReal32 oUP1 = AM_Dot(oU, oP1);
    //     const AmReal32 oVP1 = AM_Dot(oV, oP1);
    //     const AmReal32 oWP1 = AM_Dot(oW, oP1);

    //     const AmReal32 oUP2 = AM_Dot(oU, oP2);
    //     const AmReal32 oVP3 = AM_Dot(oV, oP3);
    //     const AmReal32 oWP4 = AM_Dot(oW, oP4);

    //     if (AM_BETWEEN(iUX, iUP1, iUP2) && AM_BETWEEN(iVX, iVP1, iVP3) && AM_BETWEEN(iWX, iWP1, iWP4))
    //         return 1.0f;

    //     if (!(AM_BETWEEN(oUX, oUP1, oUP2) && AM_BETWEEN(oVX, oVP1, oVP3) && AM_BETWEEN(oWX, oWP1, oWP4)))
    //         return m_maxAttenuationFactor;

    //     switch (upAxis)
    //     {
    //     default:
    //     case eGameEngineUpAxis_Y:
    //         {
    //             const AmReal32 dP1 = AM_ABS(AM_Dot(x - oP1, AM_Norm(oP2 - oP1))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
    //             const AmReal32 dP2 = AM_ABS(AM_Dot(x - oP2, AM_Norm(oP1 - oP2))) / (outer->GetHalfDepth() - inner->GetHalfDepth());
    //             const AmReal32 dP3 = AM_ABS(AM_Dot(x - oP3, AM_Norm(oP1 - oP3))) / (outer->GetHalfWidth() - inner->GetHalfWidth());
    //             const AmReal32 dP4 = AM_ABS(AM_Dot(x - oP4, AM_Norm(oP1 - oP4))) / (outer->GetHalfHeight() -
    //             inner->GetHalfHeight()); const AmReal32 dP5 = AM_ABS(AM_Dot(x - oP1, AM_Norm(oP3 - oP1))) / (outer->GetHalfWidth()
    //             - inner->GetHalfWidth()); const AmReal32 dP6 = AM_ABS(AM_Dot(x - oP1, AM_Norm(oP4 - oP1))) /
    //             (outer->GetHalfHeight() - inner->GetHalfHeight());

    //             const AmReal32 shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

    //             return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    //         }

    //     case eGameEngineUpAxis_Z:
    //         {
    //             const AmReal32 dP1 = AM_ABS(AM_Dot(x - oP1, AM_Norm(oP2 - oP1))) / (outer->GetHalfHeight() -
    //             inner->GetHalfHeight()); const AmReal32 dP2 = AM_ABS(AM_Dot(x - oP2, AM_Norm(oP1 - oP2))) / (outer->GetHalfHeight()
    //             - inner->GetHalfHeight()); const AmReal32 dP3 = AM_ABS(AM_Dot(x - oP3, AM_Norm(oP1 - oP3))) /
    //             (outer->GetHalfWidth() - inner->GetHalfWidth()); const AmReal32 dP4 = AM_ABS(AM_Dot(x - oP4, AM_Norm(oP1 - oP4))) /
    //             (outer->GetHalfDepth() - inner->GetHalfDepth()); const AmReal32 dP5 = AM_ABS(AM_Dot(x - oP1, AM_Norm(oP3 - oP1))) /
    //             (outer->GetHalfWidth() - inner->GetHalfWidth()); const AmReal32 dP6 = AM_ABS(AM_Dot(x - oP1, AM_Norm(oP4 - oP1))) /
    //             (outer->GetHalfDepth() - inner->GetHalfDepth());

    //             const AmReal32 shortestRoad = AM_MIN(dP1, AM_MIN(dP2, AM_MIN(dP3, AM_MIN(dP4, AM_MIN(dP5, dP6)))));

    //             return AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(shortestRoad, 0.0f, 1.0f), 1.0f);
    //         }
    //     }
    // }

    AmReal32 CapsuleAttenuationZone::GetAttenuationFactor(
        const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener)
    {
        m_innerShape->SetLocation(soundLocation);
        m_innerShape->SetOrientation(Orientation::Zero());

        m_outerShape->SetLocation(soundLocation);
        m_outerShape->SetOrientation(Orientation::Zero());

        return GetFactor(attenuation, soundLocation, listener, AM_M4D(1.0f));
    }

    AmReal32 CapsuleAttenuationZone::GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener)
    {
        m_innerShape->SetLocation(entity.GetLocation());
        m_innerShape->SetOrientation(entity.GetOrientation());

        m_outerShape->SetLocation(entity.GetLocation());
        m_outerShape->SetOrientation(entity.GetOrientation());

        return GetFactor(attenuation, entity.GetLocation(), listener, entity.GetOrientation().GetLookAtMatrix(AM_V3(0, 0, 0)));
    }

    AmReal32 CapsuleAttenuationZone::GetFactor(
        const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener, AmMat4 lookAt)
    {
        const auto* inner = dynamic_cast<CapsuleShape*>(m_innerShape);
        const auto* outer = dynamic_cast<CapsuleShape*>(m_outerShape);

        lookAt = AM_Mul(AM_Translate(soundLocation), lookAt);

        const AmVec3& x = listener.GetLocation();

        const AmReal32 distanceToOrigin = AM_Len(x - soundLocation);

        const AmReal32 innerHalfHeight = inner->GetHalfHeight() - inner->GetRadius();
        const AmReal32 outerHalfHeight = outer->GetHalfHeight() - outer->GetRadius();

        const AmVec3 iA = AM_Mul(lookAt, AM_V4(0.0f, 0.0f, innerHalfHeight, 1.0f)).XYZ;
        const AmVec3 iB = AM_Mul(lookAt, AM_V4(0.0f, 0.0f, -innerHalfHeight, 1.0f)).XYZ;

        const AmVec3 oA = AM_Mul(lookAt, AM_V4(0.0f, 0.0f, outerHalfHeight, 1.0f)).XYZ;
        const AmVec3 oB = AM_Mul(lookAt, AM_V4(0.0f, 0.0f, -outerHalfHeight, 1.0f)).XYZ;

        const AmVec3 iE = iB - iA;
        const AmVec3 iM = AM_Cross(iA, iB);

        const AmVec3 oE = oB - oA;
        const AmVec3 oM = AM_Cross(oA, oB);

        const AmReal32 iDistanceToAxis = AM_Len(iM + AM_Cross(iE, x)) / AM_Len(iE);
        const AmReal32 oDistanceToAxis = AM_Len(oM + AM_Cross(oE, x)) / AM_Len(oE);

        if (oDistanceToAxis >= attenuation->GetMaxDistance())
            return 0.0f;

        const AmReal32 gain = attenuation->GetGainCurve().Get(iDistanceToAxis);

        const AmReal32 iDistanceToA = AM_Len(x - iA);
        const AmReal32 iDistanceToB = AM_Len(x - iB);

        if (iDistanceToAxis <= inner->GetRadius() && distanceToOrigin <= innerHalfHeight)
            return gain * 1.0f;

        if (iDistanceToA <= inner->GetRadius() || iDistanceToB <= inner->GetRadius())
            return gain * 1.0f;

        if (oDistanceToAxis >= outer->GetRadius() && distanceToOrigin >= outerHalfHeight)
            return gain * m_maxAttenuationFactor;

        const AmReal32 rDelta = 1.0f - (oDistanceToAxis - inner->GetRadius()) / (outer->GetRadius() - inner->GetRadius());
        const AmReal32 hDelta = 1.0f - (distanceToOrigin - inner->GetHalfHeight()) / (outer->GetHalfHeight() - inner->GetHalfHeight());

        const AmReal32 delta = AM_MIN(rDelta, hDelta);

        return gain * AM_Lerp(m_maxAttenuationFactor, AM_CLAMP(delta, 0.0f, 1.0f), 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude

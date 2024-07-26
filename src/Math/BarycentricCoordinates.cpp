// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Math/BarycentricCoordinates.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool BarycentricCoordinates::RayTriangleIntersection(
        const AmVec3& rayOrigin, const AmVec3& rayDirection, const std::array<AmVec3, 3>& triangle, BarycentricCoordinates& result)
    {
        const AmVec3 ba = triangle[1] - triangle[0];
        const AmVec3 ca = triangle[2] - triangle[0];
        const AmVec3 nm = AM_Norm(AM_Cross(ba, ca));

        const AmReal32 d = -AM_Dot(triangle[0], nm);
        const AmReal32 u = -(AM_Dot(rayOrigin, nm) + d);
        const AmReal32 v = AM_Dot(rayDirection, nm);
        const AmReal32 t = u / v;

        if (t >= 0.0f && t <= 1.0f)
        {
            const AmVec3 p = rayOrigin + AM_Mul(rayDirection, t);
            result = BarycentricCoordinates(p, triangle);
            return result.IsValid();
        }

        return false;
    }

    BarycentricCoordinates::BarycentricCoordinates()
        : m_U(-kEpsilon)
        , m_V(-kEpsilon)
        , m_W(-kEpsilon)
    {}

    BarycentricCoordinates::BarycentricCoordinates(const AmVec3& p, const std::array<AmVec3, 3>& triangle)
        : BarycentricCoordinates()
    {
        const AmVec3 ab = triangle[1] - triangle[0];
        const AmVec3 ac = triangle[2] - triangle[0];
        const AmVec3 ap = p - triangle[0];

        const AmReal32 d1 = AM_Dot(ab, ab);
        const AmReal32 d2 = AM_Dot(ab, ac);
        const AmReal32 d3 = AM_Dot(ac, ac);
        const AmReal32 d4 = AM_Dot(ap, ab);
        const AmReal32 d5 = AM_Dot(ap, ac);

        const AmReal32 d = d1 * d3 - d2 * d2;

        m_V = (d3 * d4 - d2 * d5) / d;
        m_W = (d1 * d5 - d2 * d4) / d;
        m_U = 1.0f - m_V - m_W;
    }

    bool BarycentricCoordinates::IsValid() const
    {
        constexpr AmReal32 kE = std::numeric_limits<AmReal32>::epsilon();
        return m_U >= -kE && m_V >= -kE && m_U + m_V <= 1.0f + kE;
    }
} // namespace SparkyStudios::Audio::Amplitude
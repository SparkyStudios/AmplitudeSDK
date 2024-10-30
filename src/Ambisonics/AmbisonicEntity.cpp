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

#include <Ambisonics/AmbisonicEntity.h>
#include <Utils/Utils.h>

#define sqrt32 std::sqrt(3.f) / 2.f
#define sqrt58 std::sqrt(5.f / 8.f)
#define sqrt152 std::sqrt(15.f) / 2.f
#define sqrt38 std::sqrt(3.f / 8.f)

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicEntity::AmbisonicEntity()
        : m_position(0.0f, 0.0f, 1.0f)
        , m_gain(1.0f)
        , m_coefficients()
        , m_orderWeights()
    {}

    AmbisonicEntity::~AmbisonicEntity()
    {
        m_coefficients.Release();
        m_orderWeights.Release();
    }

    bool AmbisonicEntity::Configure(AmUInt32 order, bool is3D)
    {
        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        m_coefficients.Resize(m_channelCount, true);
        m_orderWeights.Resize(m_order + 1, true);

        for (AmUInt32 i = 0; i <= m_order; ++i)
            m_orderWeights[i] = 1.0f;

        return true;
    }

    void AmbisonicEntity::Reset()
    {
        std::memset(m_coefficients.GetBuffer(), 0, m_channelCount * sizeof(AmReal32));
    }

    void AmbisonicEntity::Refresh()
    {
        const AmReal32 cosAzim = std::cos(m_position.GetAzimuth());
        const AmReal32 sinAzim = std::sin(m_position.GetAzimuth());
        const AmReal32 cosElev = std::cos(m_position.GetElevation());
        const AmReal32 sinElev = std::sin(m_position.GetElevation());

        const AmReal32 cos2Azim = std::cos(2.0f * m_position.GetAzimuth());
        const AmReal32 sin2Azim = std::sin(2.0f * m_position.GetAzimuth());
        const AmReal32 sin2Elev = std::sin(2.0f * m_position.GetElevation());

        if (m_is3D)
        {
            // Uses ACN channel ordering and SN3D normalization scheme (AmbiX format)
            m_coefficients[eBFormatChannel_W] = 1.0f * m_orderWeights[0]; // W

            if (m_order >= 1)
            {
                m_coefficients[eBFormatChannel_Y] = sinAzim * cosElev * m_orderWeights[1]; // Y
                m_coefficients[eBFormatChannel_Z] = sinElev * m_orderWeights[1]; // Z
                m_coefficients[eBFormatChannel_X] = cosAzim * cosElev * m_orderWeights[1]; // X
            }

            if (m_order >= 2)
            {
                m_coefficients[eBFormatChannel_V] = sqrt32 * (sin2Azim * std::pow(cosElev, 2.0f)) * m_orderWeights[2]; // V
                m_coefficients[eBFormatChannel_T] = sqrt32 * (sinAzim * sin2Elev) * m_orderWeights[2]; // T
                m_coefficients[eBFormatChannel_R] = (1.5f * std::pow(sinElev, 2.0f) - 0.5f) * m_orderWeights[2]; // R
                m_coefficients[eBFormatChannel_S] = sqrt32 * (cosAzim * sin2Elev) * m_orderWeights[2]; // S
                m_coefficients[eBFormatChannel_U] = sqrt32 * (cos2Azim * std::pow(cosElev, 2.0f)) * m_orderWeights[2]; // U
            }

            if (m_order >= 3)
            {
                m_coefficients[eBFormatChannel_Q] =
                    sqrt58 * (std::sin(3.0f * m_position.GetAzimuth()) * std::pow(cosElev, 3.0f)) * m_orderWeights[3]; // Q
                m_coefficients[eBFormatChannel_O] = sqrt152 * (sin2Azim * sinElev * std::pow(cosElev, 2.f)) * m_orderWeights[3]; // O
                m_coefficients[eBFormatChannel_M] =
                    sqrt38 * (sinAzim * cosElev * (5.f * std::pow(sinElev, 2.f) - 1.f)) * m_orderWeights[3]; // M
                m_coefficients[eBFormatChannel_K] = sinElev * (5.f * std::pow(sinElev, 2.f) - 3.f) * 0.5f * m_orderWeights[3]; // K
                m_coefficients[eBFormatChannel_L] =
                    sqrt38 * (cosAzim * cosElev * (5.f * std::pow(sinElev, 2.f) - 1.f)) * m_orderWeights[3]; // L
                m_coefficients[eBFormatChannel_N] = sqrt152 * (cos2Azim * sinElev * std::pow(cosElev, 2.f)) * m_orderWeights[3]; // N
                m_coefficients[eBFormatChannel_P] =
                    sqrt58 * (std::cos(3.f * m_position.GetAzimuth()) * std::pow(cosElev, 3.f)) * m_orderWeights[3]; // P
            }
        }
        else
        {
            m_coefficients[0] = 1.0f * m_orderWeights[0];

            if (m_order >= 1)
            {
                m_coefficients[1] = cosAzim * cosElev * m_orderWeights[1];
                m_coefficients[2] = sinAzim * cosElev * m_orderWeights[1];
            }

            if (m_order >= 2)
            {
                m_coefficients[3] = cos2Azim * std::pow(cosElev, 2.0f) * m_orderWeights[2];
                m_coefficients[4] = sin2Azim * std::pow(cosElev, 2.0f) * m_orderWeights[2];
            }

            if (m_order >= 3)
            {
                m_coefficients[5] = std::cos(3.0f * m_position.GetAzimuth()) * std::pow(cosElev, 3) * m_orderWeights[3];
                m_coefficients[6] = std::sin(3.0f * m_position.GetAzimuth()) * std::pow(cosElev, 3) * m_orderWeights[3];
            }
        }

        ScalarMultiply(m_coefficients.GetBuffer(), m_coefficients.GetBuffer(), m_gain, m_channelCount);
    }
} // namespace SparkyStudios::Audio::Amplitude

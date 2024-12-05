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

#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

#include <Core/RoomInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmReal32 kCutOffFrequency = 800.0f;

    constexpr AmUInt32 kReflectionStartingBand = 4;
    constexpr AmUInt32 kReflectionAveragingBandsCount = 3;

    constexpr AmReal32 kDefaultAbsorptionCoefficients[static_cast<AmUInt32>(RoomMaterialType::Custom)][9] = {
        { 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f },
        { 0.11f, 0.21f, 0.41f, 0.71f, 0.76f, 0.86f, 0.86f, 0.91f, 0.91f },
        { 0.03f, 0.06f, 0.11f, 0.16f, 0.21f, 0.26f, 0.31f, 0.41f, 0.61f },
        { 0.15f, 0.36f, 0.56f, 0.71f, 0.71f, 0.66f, 0.61f, 0.51f, 0.36f },
        { 0.21f, 0.11f, 0.06f, 0.11f, 0.06f, 0.05f, 0.08f, 0.10f, 0.11f },
        { 0.01f, 0.01f, 0.02f, 0.02f, 0.03f, 0.04f, 0.05f, 0.07f, 0.09f },
        { 0.29f, 0.23f, 0.18f, 0.10f, 0.11f, 0.08f, 0.10f, 0.09f, 0.11f },
        { 0.03f, 0.03f, 0.03f, 0.04f, 0.05f, 0.04f, 0.05f, 0.07f, 0.09f },
        { 0.15f, 0.30f, 0.45f, 0.60f, 0.85f, 0.90f, 0.95f, 0.95f, 0.90f },
        { 0.07f, 0.06f, 0.05f, 0.04f, 0.03f, 0.02f, 0.02f, 0.02f, 0.02f },
        { 0.03f, 0.03f, 0.04f, 0.04f, 0.05f, 0.05f, 0.04f, 0.05f, 0.06f },
        { 0.01f, 0.01f, 0.01f, 0.01f, 0.02f, 0.02f, 0.03f, 0.03f, 0.03f },
        { 0.01f, 0.01f, 0.01f, 0.02f, 0.02f, 0.02f, 0.03f, 0.04f, 0.05f },
        { 0.01f, 0.01f, 0.01f, 0.02f, 0.02f, 0.03f, 0.04f, 0.05f, 0.06f },
        { 0.01f, 0.01f, 0.02f, 0.02f, 0.03f, 0.03f, 0.04f, 0.05f, 0.06f },
    };

    RoomMaterial::RoomMaterial()
        : m_type(RoomMaterialType::Custom)
        , m_absorptionCoefficients{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
    {}

    RoomMaterial::RoomMaterial(RoomMaterialType type)
        : m_type(type)
        , m_absorptionCoefficients{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
    {
        const auto& coefficients = kDefaultAbsorptionCoefficients[static_cast<AmUInt32>(type)];
        std::copy_n(std::begin(coefficients), 9, m_absorptionCoefficients);
    }

    void RoomInternalState::Update()
    {
        if (!_needUpdate)
            return;

        _reflectionsProperties.SetCutOffFrequency(kCutOffFrequency);

        for (AmSize i = 0; i < kAmRoomSurfaceCount; ++i)
        {
            const AmSize materialIndex = static_cast<AmSize>(_materials[i].m_type);
            const auto& coefficients = _materials[i].m_absorptionCoefficients;

            // Compute average absorption coefficients
            const AmReal32 averageAbsorptionCoefficients =
                std::accumulate(
                    std::begin(coefficients) + kReflectionStartingBand,
                    std::begin(coefficients) + kReflectionStartingBand + kReflectionAveragingBandsCount, 0.0f) /
                static_cast<AmReal32>(kReflectionAveragingBandsCount);

            AmReal32* output = _reflectionsProperties.GetCoefficients();
            output[i] = AM_MIN(1.0f, AM_SqrtF(1.0f - averageAbsorptionCoefficients));
        }

        _needUpdate = false;
        _wasUpdated = true;
    }
} // namespace SparkyStudios::Audio::Amplitude

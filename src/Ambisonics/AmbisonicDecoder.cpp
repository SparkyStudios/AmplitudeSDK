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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Ambisonics/AmbisonicDecoder.h>
#include <Ambisonics/AmbisonicSpeaker.h>
#include <Utils/lebedev-quadrature/lebedev_quadrature.hpp>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    // Decoder coefficients for Ambisonic to stereo. Useful for conversion to 2-channels when not using headphone.
    constexpr AmReal32 kDecoderCoefficientStereo[][16] = {
        { 0.5f, 0.5f / 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
        { 0.5f, -0.5f / 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
    };

    // First order decoder for 5.1 loudspeaker array
    constexpr AmReal32 kDecoderCoefficientFirst_5_1[][4] = {
        { 0.300520f, 0.135000f, 0.000000f, 0.120000f },  { 0.300520f, -0.135000f, 0.000000f, 0.120000f },
        { 0.332340f, 0.138333f, 0.000000f, -0.110000f }, { 0.332340f, -0.138333f, 0.000000f, -0.110000f },
        { 0.141421f, 0.000000f, 0.000000f, 0.053333f },  { 0.500000f, 0.000000f, 0.000000f, 0.000000f }
    };

    // Second order decoder for 5.1 loudspeaker array
    constexpr AmReal32 kDecoderCoefficientSecond_5_1[][9] = {
        { 0.286378f, 0.103333f, -0.000000f, 0.106667f, 0.028868f, 0.000000f, 0.000000f, 0.000000f, 0.019630f },
        { 0.286378f, -0.103333f, -0.000000f, 0.106667f, -0.028868f, 0.000000f, 0.000000f, -0.000000f, 0.019630f },
        { 0.449013f, 0.093333f, -0.000000f, -0.111667f, 0.018475f, -0.000000f, -0.000000f, 0.000000f, -0.018475f },
        { 0.449013f, -0.093333f, -0.000000f, -0.111667f, -0.018475f, -0.000000f, -0.000000f, 0.000000f, -0.018475f },
        { 0.060104f, 0.000000f, 0.000000f, 0.013333f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.010392f },
        { 0.500000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f }
    };

    // Third order decoder for 5.1 loudspeaker array
    constexpr AmReal32 kDecoderCoefficientThird_5_1[][16] = {
        { 0.219203f, 0.095000f, 0.000000f, 0.103333f, 0.042724f, 0.000000f, 0.000000f, 0.000000f, 0.001155f, 0.010842f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.004518f },
        { 0.219203f, -0.095000f, 0.000000f, 0.103333f, -0.042724f, 0.000000f, 0.000000f, 0.000000f, 0.001155f, -0.010842f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.004518f },
        { 0.417193f, 0.128333f, 0.000000f, -0.111667f, 0.004619f, 0.000000f, 0.000000f, 0.000000f, -0.005774f, -0.011746f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.004518f },
        { 0.417193f, -0.128333f, 0.000000f, -0.111667f, -0.004619f, 0.000000f, 0.000000f, 0.000000f, -0.005774f, 0.011746f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.004518f },
        { 0.095459f, 0.000000f, 0.000000f, 0.088333f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049652f, 0.000000f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018974f },
        { 0.500000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f }
    };

    // First order decoder for 7.1 loudspeaker array
    constexpr AmReal32 kDecoderCoefficientFirst_7_1[][4] = {
        { 0.303082f, 0.095958f, 0.000000f, 0.114243f },  { 0.303082f, -0.095958f, 0.000000f, 0.114243f },
        { 0.300098f, 0.124767f, 0.000000f, -0.017447f }, { 0.300098f, -0.124767f, 0.000000f, -0.017447f },
        { 0.259458f, 0.053266f, 0.000000f, -0.117329f }, { 0.259458f, -0.053266f, 0.000000f, -0.117329f },
        { 0.066262f, 0.000000f, 0.000000f, 0.031737f },  { 0.500000f, 0.000000f, 0.000000f, 0.000000f }
    };

    // Second order decoder for 7.1 loudspeaker array
    constexpr AmReal32 kDecoderCoefficientSecond_7_1[][9] = {
        { 0.268964f, 0.090325f, 0.000000f, 0.111024f, 0.044867f, 0.000000f, 0.000000f, 0.000000f, 0.015736f },
        { 0.268964f, -0.090325f, 0.000000f, 0.111024f, -0.044867f, -0.000000f, 0.000000f, 0.000000f, 0.015736f },
        { 0.229483f, 0.136694f, 0.000000f, -0.018120f, -0.020953f, 0.000000f, 0.000000f, 0.000000f, -0.049001f },
        { 0.229483f, -0.136694f, 0.000000f, -0.018120f, 0.020953f, -0.000000f, 0.000000f, 0.000000f, -0.049001f },
        { 0.216456f, 0.042012f, 0.000000f, -0.116220f, -0.038878f, 0.000000f, 0.000000f, 0.000000f, 0.032005f },
        { 0.216456f, -0.042012f, 0.000000f, -0.116220f, 0.038878f, -0.000000f, 0.000000f, 0.000000f, 0.032005f },
        { 0.058222f, 0.000000f, 0.000000f, 0.048933f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025293f },
        { 0.500000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f }
    };

    // Third order decoder for 7.1 loudspeaker array
    constexpr AmReal32 kDecoderCoefficientThird_7_1[][16] = {
        { 0.238475f, 0.085873f, 0.000000f, 0.114877f, 0.054573f, 0.000000f, 0.000000f, 0.000000f, 0.015163f, 0.006254f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.006185f },
        { 0.238475f, -0.085873f, 0.000000f, 0.114877f, -0.054573f, -0.000000f, 0.000000f, 0.000000f, 0.015163f, -0.006254f, -0.000000f,
          -0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.006185f },
        { 0.214882f, 0.124042f, 0.000000f, -0.017580f, -0.018064f, 0.000000f, 0.000000f, 0.000000f, -0.060255f, -0.011908f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.008159f },
        { 0.214882f, -0.124042f, 0.000000f, -0.017580f, 0.018064f, -0.000000f, 0.000000f, 0.000000f, -0.060255f, 0.011908f, -0.000000f,
          -0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.008159f },
        { 0.197904f, 0.043357f, 0.000000f, -0.115673f, -0.048364f, 0.000000f, 0.000000f, 0.000000f, 0.034129f, 0.017198f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.001868f },
        { 0.197904f, -0.043357f, 0.000000f, -0.115673f, 0.048364f, -0.000000f, 0.000000f, 0.000000f, 0.034129f, -0.017198f, -0.000000f,
          -0.000000f, 0.000000f, 0.000000f, 0.000000f, -0.001868f },
        { 0.077144f, 0.000000f, 0.000000f, 0.045620f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030548f, 0.000000f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025329f },
        { 0.500000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
          0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f }
    };

    AmbisonicDecoder::AmbisonicDecoder()
        : _speakersPreset(eSpeakerSetUp_COUNT)
        , _speakerCount(0)
        , _speakers()
        , _isLoaded(false)
    {}

    AmbisonicDecoder::~AmbisonicDecoder()
    {
        _speakers.clear();
    }

    bool AmbisonicDecoder::Configure(AmUInt32 order, bool is3D, SpeakersPreset setUp, AmUInt32 speakerCount)
    {
        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        SetUpSpeakers(setUp, speakerCount);
        Refresh();

        return true;
    }

    void AmbisonicDecoder::Reset()
    {
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].Reset();
    }

    void AmbisonicDecoder::Refresh()
    {
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].Refresh();

        DetectSpeakersPreset();
        LoadDecoderPreset();
    }

    void AmbisonicDecoder::Process(BFormat* input, AmUInt32 samples, AudioBuffer& output)
    {
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].Process(input, samples, output[i]);
    }

    void AmbisonicDecoder::SetSpeakerPosition(AmUInt32 speaker, const SphericalPosition& position)
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        _speakers[speaker].SetPosition(position);
    }

    SphericalPosition AmbisonicDecoder::GetSpeakerPosition(AmUInt32 speaker) const
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        return _speakers[speaker].GetPosition();
    }

    void AmbisonicDecoder::SetSpeakerOrderWeight(AmUInt32 speaker, AmUInt32 order, AmReal32 weight)
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        _speakers[speaker].SetOrderWeight(order, weight);
    }

    AmReal32 AmbisonicDecoder::GetSpeakerOrderWeight(AmUInt32 speaker, AmUInt32 order) const
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        return _speakers[speaker].GetOrderWeight(order);
    }

    void AmbisonicDecoder::SetSpeakerCoefficient(AmUInt32 speaker, AmUInt32 channel, AmReal32 coefficient)
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        _speakers[speaker].SetCoefficient(channel, coefficient);
    }

    AmReal32 AmbisonicDecoder::GetSpeakerCoefficient(AmUInt32 speaker, AmUInt32 channel) const
    {
        AMPLITUDE_ASSERT(speaker < _speakerCount);
        return _speakers[speaker].GetCoefficient(channel);
    }

    void AmbisonicDecoder::SetUpSpeakers(SpeakersPreset setUp, AmUInt32 speakerCount)
    {
        _speakersPreset = setUp;

        _speakers.clear();

        _isLoaded = false;

        switch (_speakersPreset)
        {
        case eSpeakersPreset_Custom:
            _speakerCount = speakerCount;
            _speakers.resize(_speakerCount);
            for (AmUInt32 i = 0; i < _speakerCount; ++i)
                _speakers[i].Configure(m_order, m_is3D);
            break;

        case eSpeakersPreset_Mono:
            _speakerCount = 1;
            _speakers.resize(_speakerCount);
            _speakers[0].Configure(m_order, m_is3D);
            _speakers[0].SetPosition({ 0.0f, 0.0f, 1.0f });
            break;

        case eSpeakersPreset_Stereo:
            _speakerCount = 2;
            _speakers.resize(_speakerCount);
            _speakers[0].Configure(m_order, m_is3D);
            _speakers[0].SetPosition({ +30.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[1].Configure(m_order, m_is3D);
            _speakers[1].SetPosition({ -30.0f * AM_DegToRad, 0.0f, 1.0f });
            break;

        case eSpeakersPreset_Surround_5_1:
            _speakerCount = 6;
            _speakers.resize(_speakerCount);
            _speakers[0].Configure(m_order, m_is3D);
            _speakers[0].SetPosition({ +30.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[1].Configure(m_order, m_is3D);
            _speakers[1].SetPosition({ -30.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[2].Configure(m_order, m_is3D);
            _speakers[2].SetPosition({ +110.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[3].Configure(m_order, m_is3D);
            _speakers[3].SetPosition({ -110.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[4].Configure(m_order, m_is3D);
            _speakers[4].SetPosition({ 0.0f, 0.0f, 1.0f });
            // LFE channel
            _speakers[5].Configure(m_order, m_is3D);
            _speakers[5].SetPosition({ 0.0f, 0.0f, 0.0f });
            break;

        case eSpeakersPreset_Surround_7_1:
            _speakerCount = 8;
            _speakers.resize(_speakerCount);
            _speakers[0].Configure(m_order, m_is3D);
            _speakers[0].SetPosition({ +30.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[1].Configure(m_order, m_is3D);
            _speakers[1].SetPosition({ -30.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[2].Configure(m_order, m_is3D);
            _speakers[2].SetPosition({ +110.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[3].Configure(m_order, m_is3D);
            _speakers[3].SetPosition({ -110.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[4].Configure(m_order, m_is3D);
            _speakers[4].SetPosition({ +145.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[5].Configure(m_order, m_is3D);
            _speakers[5].SetPosition({ -145.0f * AM_DegToRad, 0.0f, 1.0f });
            _speakers[6].Configure(m_order, m_is3D);
            _speakers[6].SetPosition({ 0.0f, 0.0f, 1.0f });
            // LFE channel
            _speakers[7].Configure(m_order, m_is3D);
            _speakers[7].SetPosition({ 0.0f, 0.0f, 0.0f });
            break;

        case eSpeakersPreset_CubePoints:
            {
                _speakerCount = 8;
                _speakers.resize(_speakerCount);
                SphericalPosition position = { 0.0f, +35.2f * AM_DegToRad, 1.0f };
                for (AmUInt32 i = 0; i < 4; ++i)
                {
                    position.SetAzimuth(-(static_cast<AmReal32>(i) * 360.f / 4.0f + 45.f) * AM_DegToRad);
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(position);
                }
                position.SetElevation(-35.2f * AM_DegToRad);
                for (AmUInt32 i = 4; i < 8; ++i)
                {
                    position.SetAzimuth(-(static_cast<AmReal32>(i - 4) * 360.f / 4.0f + 45.f) * AM_DegToRad);
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(position);
                }
                break;
            }

        case eSpeakersPreset_DodecahedronFaces:
            {
                _speakerCount = 12;
                _speakers.resize(_speakerCount);
                SphericalPosition position = { 0.0f, 0.0f, 1.0f };
                for (AmUInt32 i = 0; i < 12; i++)
                {
                    position.SetAzimuth(-(i * 360.f / 12) * AM_DegToRad);
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(position);
                }
                break;
            }

        case eSpeakersPreset_LebedevGridOrder26:
            {
                _speakerCount = 26;
                _speakers.resize(_speakerCount);

                const auto quad_points = lebedev::generate_quadrature_points(lebedev::QuadratureOrder::order_26);

                const auto& xAxis = std::get<0>(quad_points);
                const auto& yAxis = std::get<1>(quad_points);
                const auto& zAxis = std::get<2>(quad_points);

                for (AmUInt32 i = 0; i < 26; ++i)
                {
                    _speakers[i].Configure(m_order, m_is3D);
                    _speakers[i].SetPosition(SphericalPosition::ForHRTF(AM_V3(xAxis.at(i), yAxis.at(i), zAxis.at(i))));
                }
                break;
            }

        default:
            amLogError("Invalid speaker setup: %u", static_cast<unsigned int>(_speakersPreset));
            AMPLITUDE_ASSERT(false);
            break;
        }

        const AmReal32 speakerGain = 1.0f / static_cast<AmReal32>(_speakerCount);
        for (AmUInt32 i = 0; i < _speakerCount; ++i)
            _speakers[i].SetGain(speakerGain);
    }

    void AmbisonicDecoder::DetectSpeakersPreset()
    {
        if (_speakersPreset != eSpeakersPreset_Custom)
            return;

        AmUInt32 speakerMatchCount = 0;

        // clang-format off
        constexpr AmReal32 azimuthStereo[] = { 30.0f * AM_DegToRad, -30.0f * AM_DegToRad };
        constexpr AmReal32 azimuthQuad[] = { 45.0f * AM_DegToRad, -45.0f * AM_DegToRad, 135.0f * AM_DegToRad, -135.0f * AM_DegToRad };
        constexpr AmReal32 azimuthSurround51[] = { 30.0f * AM_DegToRad, -30.0f * AM_DegToRad, 110.0f * AM_DegToRad, -110.0f * AM_DegToRad, 0.0f, 0.0f };
        constexpr AmReal32 azimuthSurround71[] = { 30.0f * AM_DegToRad, -30.0f * AM_DegToRad, 110.0f * AM_DegToRad, -110.0f * AM_DegToRad, 145.0f * AM_DegToRad, -145.0f * AM_DegToRad, 0.0f, 0.0f };
        // clang-format on

        switch (_speakerCount)
        {
        case 1:
            _speakersPreset = eSpeakersPreset_Mono;
            break;

        case 2:
            for (AmUInt32 i = 0; i < 2; ++i)
                if (const auto position = _speakers[i].GetPosition(); position.GetElevation() == 0.0f)
                    if (std::abs(position.GetAzimuth() - azimuthStereo[i]) < kEpsilon)
                        speakerMatchCount++;

            if (speakerMatchCount == 2)
                _speakersPreset = eSpeakersPreset_Stereo;
            break;

        case 6:
            for (AmUInt32 i = 0; i < 6; ++i)
                if (const auto position = _speakers[i].GetPosition(); position.GetElevation() == 0.0f)
                    if (std::abs(position.GetAzimuth() - azimuthSurround51[i]) < kEpsilon)
                        speakerMatchCount++;

            if (speakerMatchCount == 6)
                _speakersPreset = eSpeakersPreset_Surround_5_1;
            break;

        case 8:
            for (AmUInt32 i = 0; i < 8; ++i)
                if (const auto position = _speakers[i].GetPosition(); position.GetElevation() == 0.0f)
                    if (std::abs(position.GetAzimuth() - azimuthSurround71[i]) < kEpsilon)
                        speakerMatchCount++;

            if (speakerMatchCount == 8)
                _speakersPreset = eSpeakersPreset_Surround_7_1;
            break;

        default:
            break;
        }
    }

    void AmbisonicDecoder::LoadDecoderPreset()
    {
        const AmUInt32 ambisonicComponents = OrderToComponents(m_order, m_is3D);

        switch (_speakersPreset)
        {
        case eSpeakersPreset_Mono:
            [[fallthrough]];
        case eSpeakersPreset_CubePoints:
            [[fallthrough]];
        case eSpeakersPreset_DodecahedronFaces:
            [[fallthrough]];
        case eSpeakersPreset_LebedevGridOrder26:
            _isLoaded = true;
            break;

        case eSpeakersPreset_Stereo:
            {
                for (AmUInt32 i = 0; i < 2; ++i)
                    for (AmUInt32 j = 0; j < ambisonicComponents; ++j)
                        _speakers[i].SetCoefficient(j, kDecoderCoefficientStereo[i][j]);

                _isLoaded = true;
            }
            break;

        case eSpeakersPreset_Surround_5_1:
            {
                for (AmUInt32 i = 0; i < 6; ++i)
                {
                    for (AmUInt32 j = 0; j < ambisonicComponents; ++j)
                    {
                        if (m_order <= 1)
                            _speakers[i].SetCoefficient(j, kDecoderCoefficientFirst_5_1[i][j]);
                        else if (m_order <= 2)
                            _speakers[i].SetCoefficient(j, kDecoderCoefficientSecond_5_1[i][j]);
                        else if (m_order == 3)
                            _speakers[i].SetCoefficient(j, kDecoderCoefficientThird_5_1[i][j]);
                    }
                }

                _isLoaded = true;
            }
            break;

        case eSpeakersPreset_Surround_7_1:
            {
                for (AmUInt32 i = 0; i < 8; ++i)
                {
                    for (AmUInt32 j = 0; j < ambisonicComponents; ++j)
                    {
                        if (m_order == 1)
                            _speakers[i].SetCoefficient(j, kDecoderCoefficientFirst_7_1[i][j]);
                        else if (m_order == 2)
                            _speakers[i].SetCoefficient(j, kDecoderCoefficientSecond_7_1[i][j]);
                        else if (m_order == 3)
                            _speakers[i].SetCoefficient(j, kDecoderCoefficientThird_7_1[i][j]);
                    }
                }

                _isLoaded = true;
            }
            break;

        default:
            _isLoaded = false;
            break;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude

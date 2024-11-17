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

#pragma once

#ifndef _AM_IMPLEMENTATION_CORE_DEFAULT_PLUGINS_H
#define _AM_IMPLEMENTATION_CORE_DEFAULT_PLUGINS_H

#pragma region Default Resamplers

#include <DSP/Resamplers/DefaultResampler.h>
#include <DSP/Resamplers/R8BrainResampler.h>

#pragma endregion

#pragma region Default Faders

#include <Sound/Faders/ConstantFader.h>
#include <Sound/Faders/EaseFader.h>
#include <Sound/Faders/EaseInFader.h>
#include <Sound/Faders/EaseInOutFader.h>
#include <Sound/Faders/EaseOutFader.h>
#include <Sound/Faders/ExponentialFader.h>
#include <Sound/Faders/LinearFader.h>
#include <Sound/Faders/SCurveFader.h>

#pragma endregion

#pragma region Default Codecs

#include <Core/Codecs/AMS/Codec.h>
#include <Core/Codecs/MP3/Codec.h>
#include <Core/Codecs/WAV/Codec.h>

#pragma endregion

#pragma region Default Drivers

#include <Core/Drivers/MiniAudio/Driver.h>
#include <Core/Drivers/Null/Driver.h>

#pragma endregion

#pragma region Default Filters

#include <DSP/Filters/BassBoostFilter.h>
#include <DSP/Filters/BiquadResonantFilter.h>
#include <DSP/Filters/DCRemovalFilter.h>
#include <DSP/Filters/DelayFilter.h>
#include <DSP/Filters/EqualizerFilter.h>
#include <DSP/Filters/FlangerFilter.h>
#include <DSP/Filters/LofiFilter.h>
#include <DSP/Filters/MonoPoleFilter.h>
#include <DSP/Filters/RobotizeFilter.h>
#include <DSP/Filters/WaveShaperFilter.h>

#pragma endregion

#pragma region Default Pipeline Nodes

#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>
#include <Mixer/Nodes/AmbisonicMixerNode.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Mixer/Nodes/AmbisonicRotatorNode.h>
#include <Mixer/Nodes/AttenuationNode.h>
#include <Mixer/Nodes/ClampNode.h>
#include <Mixer/Nodes/ClipNode.h>
#include <Mixer/Nodes/EnvironmentEffectNode.h>
#include <Mixer/Nodes/InputNode.h>
#include <Mixer/Nodes/NearFieldEffectNode.h>
#include <Mixer/Nodes/ObstructionNode.h>
#include <Mixer/Nodes/OcclusionNode.h>
#include <Mixer/Nodes/OutputNode.h>
#include <Mixer/Nodes/ReflectionsNode.h>
#include <Mixer/Nodes/ReverbNode.h>
#include <Mixer/Nodes/StereoMixerNode.h>
#include <Mixer/Nodes/StereoPanningNode.h>

#pragma endregion

#endif // _AM_IMPLEMENTATION_CORE_DEFAULT_PLUGINS_H

# Copyright (c) 2021-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0048 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DetectAmplitudeVersion)

option(BUILD_SAMPLES "Build samples" OFF)
option(BUILD_TOOLS "Build official CLI tools" ON)
option(UNIT_TESTS "Enable Unit Testing" OFF)

if(UNIT_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
    set(BUILD_SAMPLES ON)
endif ()

if (BUILD_SAMPLES)
    list(APPEND VCPKG_MANIFEST_FEATURES "samples")
endif ()

if (BUILD_TOOLS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tools")
endif ()

project(Amplitude
    VERSION ${DETECTED_AM_VERSION}
    DESCRIPTION "A cross-platform audio engine for games"
    HOMEPAGE_URL "https://amplitudeaudiosdk.com"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIT_TESTS AND NOT MSVC)
    include(CodeCoverage)
    append_coverage_compiler_flags()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif ()

set(CMAKE_DEBUG_POSTFIX "_d")

set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OBJ_DIR ${CMAKE_CURRENT_BINARY_DIR}/obj)

set(AM_LIB_DESTINATION "lib/${VCPKG_TARGET_TRIPLET}")
set(AM_INC_DESTINATION "include")
set(AM_SHM_DESTINATION "schemas")
set(AM_PYS_DESTINATION "scripts")
set(AM_BIN_DESTINATION "bin/${VCPKG_TARGET_TRIPLET}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${AM_BIN_DESTINATION}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${AM_LIB_DESTINATION}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${AM_BIN_DESTINATION}")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/sdk")

if (UNIX)
    find_package(Threads REQUIRED)
endif ()

# Setup FFT and Accelerate Framework
# ------------------------------------------------------------------------------
if (APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
endif ()

# ------------------------------------------------------------------------------

# Setup flatbuffers
# ------------------------------------------------------------------------------
find_package(flatbuffers CONFIG REQUIRED)

# ------------------------------------------------------------------------------

# Setup xsimd
# ------------------------------------------------------------------------------
find_package(xsimd CONFIG REQUIRED)

# ------------------------------------------------------------------------------

# Setup miniaudio
# ------------------------------------------------------------------------------
find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")

# Check what instruction sets the current host supports.
include(DetectCPUArchitecture)
am_buildsystem_get_runnable_archs(NATIVE_ARCHS)

foreach (ARCH ${NATIVE_ARCHS})
    am_buildsystem_get_arch_info(CXX_FLAGS DEFINES_LIST SUFFIX ${ARCH})

    separate_arguments(CXX_FLAGS)
    add_compile_options(${CXX_FLAGS})
endforeach ()

# ------------------------------------------------------------------------------

# Generate source files for all FlatBuffers schema files under the src
# directory.
set(AM_FLATBUFFERS_GENERATED_INCLUDES_DIR ${OBJ_DIR}/include/SparkyStudios/Audio/Amplitude)
file(GLOB_RECURSE FLATBUFFERS_SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.fbs)

# Generate rules to build the set of output files from the set of input
# schema files.
build_flatbuffers("${FLATBUFFERS_SCHEMAS}"
    ""
    generated_includes
    ""
    ${AM_FLATBUFFERS_GENERATED_INCLUDES_DIR}
    ""
    ""
)

# Setup project source files.
set(SA_SOURCE
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Apple/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Apple/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/UNIX/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/UNIX/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Config.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Constants.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/SIMD.h
    include/SparkyStudios/Audio/Amplitude/Core/Common/Types.h
    include/SparkyStudios/Audio/Amplitude/Core/Playback/Bus.h
    include/SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h
    include/SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h
    include/SparkyStudios/Audio/Amplitude/Core/Asset.h
    include/SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h
    include/SparkyStudios/Audio/Amplitude/Core/Codec.h
    include/SparkyStudios/Audio/Amplitude/Core/Common.h
    include/SparkyStudios/Audio/Amplitude/Core/Device.h
    include/SparkyStudios/Audio/Amplitude/Core/Driver.h
    include/SparkyStudios/Audio/Amplitude/Core/Engine.h
    include/SparkyStudios/Audio/Amplitude/Core/Entity.h
    include/SparkyStudios/Audio/Amplitude/Core/Environment.h
    include/SparkyStudios/Audio/Amplitude/Core/Event.h
    include/SparkyStudios/Audio/Amplitude/Core/Listener.h
    include/SparkyStudios/Audio/Amplitude/Core/Log.h
    include/SparkyStudios/Audio/Amplitude/Core/Memory.h
    include/SparkyStudios/Audio/Amplitude/Core/RefCounter.h
    include/SparkyStudios/Audio/Amplitude/Core/Room.h
    include/SparkyStudios/Audio/Amplitude/Core/Thread.h
    include/SparkyStudios/Audio/Amplitude/Core/Version.h
    include/SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h
    include/SparkyStudios/Audio/Amplitude/DSP/Convolver.h
    include/SparkyStudios/Audio/Amplitude/DSP/FFT.h
    include/SparkyStudios/Audio/Amplitude/DSP/Filter.h
    include/SparkyStudios/Audio/Amplitude/DSP/Resampler.h
    include/SparkyStudios/Audio/Amplitude/DSP/SplitComplex.h
    include/SparkyStudios/Audio/Amplitude/HRTF/HRIRSphere.h
    include/SparkyStudios/Audio/Amplitude/IO/DiskFile.h
    include/SparkyStudios/Audio/Amplitude/IO/DiskFileSystem.h
    include/SparkyStudios/Audio/Amplitude/IO/File.h
    include/SparkyStudios/Audio/Amplitude/IO/FileSystem.h
    include/SparkyStudios/Audio/Amplitude/IO/MemoryFile.h
    include/SparkyStudios/Audio/Amplitude/IO/PackageItemFile.h
    include/SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h
    include/SparkyStudios/Audio/Amplitude/IO/Resource.h
    include/SparkyStudios/Audio/Amplitude/Math/BarycentricCoordinates.h
    include/SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h
    include/SparkyStudios/Audio/Amplitude/Math/Curve.h
    include/SparkyStudios/Audio/Amplitude/Math/Geometry.h
    include/SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h
    include/SparkyStudios/Audio/Amplitude/Math/Orientation.h
    include/SparkyStudios/Audio/Amplitude/Math/Shape.h
    include/SparkyStudios/Audio/Amplitude/Math/SphericalPosition.h
    include/SparkyStudios/Audio/Amplitude/Math/Utils.h
    include/SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h
    include/SparkyStudios/Audio/Amplitude/Mixer/Node.h
    include/SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h
    include/SparkyStudios/Audio/Amplitude/Sound/Attenuation.h
    include/SparkyStudios/Audio/Amplitude/Sound/Collection.h
    include/SparkyStudios/Audio/Amplitude/Sound/Effect.h
    include/SparkyStudios/Audio/Amplitude/Sound/Fader.h
    include/SparkyStudios/Audio/Amplitude/Sound/Rtpc.h
    include/SparkyStudios/Audio/Amplitude/Sound/Sound.h
    include/SparkyStudios/Audio/Amplitude/Sound/SoundBank.h
    include/SparkyStudios/Audio/Amplitude/Sound/SoundObject.h
    include/SparkyStudios/Audio/Amplitude/Sound/Switch.h
    include/SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h
    include/SparkyStudios/Audio/Amplitude/Amplitude.h

    src/Ambisonics/AmbisonicBinauralizer.cpp
    src/Ambisonics/AmbisonicBinauralizer.h
    src/Ambisonics/AmbisonicComponent.cpp
    src/Ambisonics/AmbisonicComponent.h
    src/Ambisonics/AmbisonicDecoder.cpp
    src/Ambisonics/AmbisonicDecoder.h
    src/Ambisonics/AmbisonicEntity.cpp
    src/Ambisonics/AmbisonicEntity.h
    src/Ambisonics/AmbisonicOrientationProcessor.cpp
    src/Ambisonics/AmbisonicOrientationProcessor.h
    src/Ambisonics/AmbisonicSource.cpp
    src/Ambisonics/AmbisonicSource.cpp
    src/Ambisonics/AmbisonicSpeaker.cpp
    src/Ambisonics/AmbisonicSpeaker.h
    src/Ambisonics/BFormat.cpp
    src/Ambisonics/BFormat.h

    src/Core/Codecs/AMS/Codec.cpp
    src/Core/Codecs/AMS/Codec.h

    src/Core/Codecs/MP3/Codec.cpp
    src/Core/Codecs/MP3/Codec.h
    src/Core/Codecs/MP3/dr_mp3.h

    src/Core/Codecs/WAV/Codec.cpp
    src/Core/Codecs/WAV/Codec.h
    src/Core/Codecs/WAV/dr_wav.h

    src/Core/Drivers/MiniAudio/Driver.cpp
    src/Core/Drivers/MiniAudio/Driver.h

    src/Core/Drivers/Null/Driver.cpp
    src/Core/Drivers/Null/Driver.h

    src/Core/Playback/Bus.cpp
    src/Core/Playback/BusInternalState.cpp
    src/Core/Playback/BusInternalState.h
    src/Core/Playback/Channel.cpp
    src/Core/Playback/ChannelEventListener.cpp
    src/Core/Playback/ChannelInternalState.cpp
    src/Core/Playback/ChannelInternalState.h

    src/Core/Asset.cpp
    src/Core/Asset.h
    src/Core/AudioBuffer.cpp
    src/Core/AudioBufferCrossFader.cpp
    src/Core/AudioBufferCrossFader.h
    src/Core/Codec.cpp
    src/Core/Common.cpp
    src/Core/DefaultPlugins.h
    src/Core/Device.cpp
    src/Core/Driver.cpp
    src/Core/Engine.cpp
    src/Core/Engine.h
    src/Core/EngineInternalState.h
    src/Core/Entity.cpp
    src/Core/EntityInternalState.cpp
    src/Core/EntityInternalState.h
    src/Core/Environment.cpp
    src/Core/EnvironmentInternalState.cpp
    src/Core/EnvironmentInternalState.h
    src/Core/Event.cpp
    src/Core/Event.h
    src/Core/ListenerInternalState.cpp
    src/Core/ListenerInternalState.h
    src/Core/Listener.cpp
    src/Core/Log.cpp
    src/Core/Memory.cpp
    src/Core/Room.cpp
    src/Core/RoomInternalState.cpp
    src/Core/RoomInternalState.h
    src/Core/Thread.cpp
    src/Core/Version.cpp

    src/DSP/Filters/BassBoostFilter.cpp
    src/DSP/Filters/BassBoostFilter.h
    src/DSP/Filters/BiquadResonantFilter.cpp
    src/DSP/Filters/BiquadResonantFilter.h
    src/DSP/Filters/DCRemovalFilter.cpp
    src/DSP/Filters/DCRemovalFilter.h
    src/DSP/Filters/DelayFilter.cpp
    src/DSP/Filters/DelayFilter.h
    src/DSP/Filters/EqualizerFilter.cpp
    src/DSP/Filters/EqualizerFilter.h
    src/DSP/Filters/FFTFilter.cpp
    src/DSP/Filters/FFTFilter.h
    src/DSP/Filters/FlangerFilter.cpp
    src/DSP/Filters/FlangerFilter.h
    src/DSP/Filters/LofiFilter.cpp
    src/DSP/Filters/LofiFilter.h
    src/DSP/Filters/MonoPoleFilter.cpp
    src/DSP/Filters/MonoPoleFilter.h
    src/DSP/Filters/RobotizeFilter.cpp
    src/DSP/Filters/RobotizeFilter.h
    src/DSP/Filters/WaveShaperFilter.cpp
    src/DSP/Filters/WaveShaperFilter.h
    src/DSP/Resamplers/DefaultResampler.cpp
    src/DSP/Resamplers/DefaultResampler.h
    src/DSP/Resamplers/R8BrainResampler.h
    src/DSP/AudioConverter.cpp
    src/DSP/Convolver.cpp
    src/DSP/Delay.cpp
    src/DSP/Delay.h
    src/DSP/FFT.cpp
    src/DSP/Filter.cpp
    src/DSP/Gain.cpp
    src/DSP/Gain.h
    src/DSP/NearFieldProcessor.cpp
    src/DSP/NearFieldProcessor.h
    src/DSP/ReflectionsProcessor.cpp
    src/DSP/ReflectionsProcessor.h
    src/DSP/Resampler.cpp
    src/DSP/SplitComplex.cpp

    src/HRTF/HRIRSphere.cpp
    src/HRTF/HRIRSphere.h
    src/IO/DiskFile.cpp
    src/IO/DiskFileSystem.cpp
    src/IO/File.cpp
    src/IO/MemoryFile.cpp
    src/IO/PackageItemFile.cpp
    src/IO/PackageFileSystem.cpp
    src/IO/Resource.h

    src/Math/BarycentricCoordinates.cpp
    src/Math/CartesianCoordinateSystem.cpp
    src/Math/Curve.cpp
    src/Math/Orientation.cpp
    src/Math/FaceBSPTree.cpp
    src/Math/FaceBSPTree.h
    src/Math/Geometry.cpp
    src/Math/Shape.cpp
    src/Math/SphericalPosition.cpp

    src/Mixer/Nodes/AmbisonicBinauralDecoderNode.cpp
    src/Mixer/Nodes/AmbisonicBinauralDecoderNode.h
    src/Mixer/Nodes/AmbisonicMixerNode.cpp
    src/Mixer/Nodes/AmbisonicMixerNode.h
    src/Mixer/Nodes/AmbisonicPanningNode.cpp
    src/Mixer/Nodes/AmbisonicPanningNode.h
    src/Mixer/Nodes/AmbisonicRotatorNode.cpp
    src/Mixer/Nodes/AmbisonicRotatorNode.h
    src/Mixer/Nodes/AttenuationNode.cpp
    src/Mixer/Nodes/AttenuationNode.h
    src/Mixer/Nodes/ClampNode.cpp
    src/Mixer/Nodes/ClampNode.h
    src/Mixer/Nodes/ClipNode.cpp
    src/Mixer/Nodes/ClipNode.h
    src/Mixer/Nodes/EnvironmentEffectNode.cpp
    src/Mixer/Nodes/EnvironmentEffectNode.h
    src/Mixer/Nodes/InputNode.cpp
    src/Mixer/Nodes/InputNode.h
    src/Mixer/Nodes/NearFieldEffectNode.cpp
    src/Mixer/Nodes/NearFieldEffectNode.h
    src/Mixer/Nodes/ObstructionNode.cpp
    src/Mixer/Nodes/ObstructionNode.h
    src/Mixer/Nodes/OcclusionNode.cpp
    src/Mixer/Nodes/OcclusionNode.h
    src/Mixer/Nodes/OutputNode.cpp
    src/Mixer/Nodes/OutputNode.h
    src/Mixer/Nodes/ReflectionsNode.cpp
    src/Mixer/Nodes/ReflectionsNode.h
    src/Mixer/Nodes/ReverbNode.cpp
    src/Mixer/Nodes/ReverbNode.h
    src/Mixer/Nodes/StereoPanningNode.cpp
    src/Mixer/Nodes/StereoPanningNode.h
    src/Mixer/Nodes/StereoMixerNode.cpp
    src/Mixer/Nodes/StereoMixerNode.h

    src/Mixer/Amplimix.h
    src/Mixer/Amplimix.cpp
    src/Mixer/Node.cpp
    src/Mixer/Pipeline.cpp
    src/Mixer/RealChannel.cpp
    src/Mixer/RealChannel.h
    src/Mixer/SoundData.cpp
    src/Mixer/SoundData.h

    src/Sound/Faders/ConstantFader.h
    src/Sound/Faders/EaseFader.h
    src/Sound/Faders/EaseInFader.h
    src/Sound/Faders/EaseInOutFader.h
    src/Sound/Faders/EaseOutFader.h
    src/Sound/Faders/ExponentialFader.h
    src/Sound/Faders/LinearFader.h
    src/Sound/Faders/SCurveFader.h
    src/Sound/Schedulers/RandomScheduler.cpp
    src/Sound/Schedulers/RandomScheduler.h
    src/Sound/Schedulers/SequenceScheduler.cpp
    src/Sound/Schedulers/SequenceScheduler.h
    src/Sound/Attenuation.cpp
    src/Sound/Attenuation.h
    src/Sound/AttenuationShapes.cpp
    src/Sound/AttenuationShapes.h
    src/Sound/Collection.cpp
    src/Sound/Collection.h
    src/Sound/Effect.cpp
    src/Sound/Effect.h
    src/Sound/Fader.cpp
    src/Sound/RefCounter.cpp
    src/Sound/Rtpc.cpp
    src/Sound/Rtpc.h
    src/Sound/Scheduler.h
    src/Sound/Sound.cpp
    src/Sound/Sound.h
    src/Sound/SoundBank.cpp
    src/Sound/SoundObject.cpp
    src/Sound/SoundObject.h
    src/Sound/Switch.cpp
    src/Sound/Switch.h
    src/Sound/SwitchContainer.cpp
    src/Sound/SwitchContainer.h

    src/Utils/Audio/Compression/ADPCM/ADPCM.cpp
    src/Utils/Audio/Compression/ADPCM/ADPCM.h
    src/Utils/Audio/FFT/AudioFFT.cpp
    src/Utils/Audio/FFT/AudioFFT.h
    src/Utils/Audio/Resampling/CDSPBlockConvolver.h
    src/Utils/Audio/Resampling/CDSPFIRFilter.h
    src/Utils/Audio/Resampling/CDSPFracInterpolator.h
    src/Utils/Audio/Resampling/CDSPHBDownsampler.h
    src/Utils/Audio/Resampling/CDSPHBUpsampler.h
    src/Utils/Audio/Resampling/CDSPProcessor.h
    src/Utils/Audio/Resampling/CDSPRealFFT.h
    src/Utils/Audio/Resampling/CDSPResampler.h
    src/Utils/Audio/Resampling/CDSPSincFilterGen.h
    src/Utils/Audio/Resampling/r8bbase.cpp
    src/Utils/Audio/Resampling/r8bbase.h
    src/Utils/Audio/Resampling/r8bconf.h
    src/Utils/Audio/Resampling/r8butil.h
    src/Utils/Freeverb/AllPass.cpp
    src/Utils/Freeverb/AllPass.h
    src/Utils/Freeverb/Comb.cpp
    src/Utils/Freeverb/Comb.h
    src/Utils/Freeverb/denormals.h
    src/Utils/Freeverb/ReverbModel.cpp
    src/Utils/Freeverb/ReverbModel.h
    src/Utils/Freeverb/tuning.h
    src/Utils/lebedev-quadrature/generator_point.hpp
    src/Utils/lebedev-quadrature/generator_point.inl
    src/Utils/lebedev-quadrature/lebedev_quadrature.hpp
    src/Utils/lebedev-quadrature/preprocessor.hpp
    src/Utils/lebedev-quadrature/quadrature_order.hpp
    src/Utils/lebedev-quadrature/quadrature_order.inl
    src/Utils/lebedev-quadrature/quadrature_points.hpp
    src/Utils/lebedev-quadrature/quadrature_points.inl
    src/Utils/miniaudio/miniaudio.cpp
    src/Utils/miniaudio/miniaudio_utils.h
    src/Utils/pffft/simd/pf_altivec_float.h
    src/Utils/pffft/simd/pf_avx_double.h
    src/Utils/pffft/simd/pf_double.h
    src/Utils/pffft/simd/pf_float.h
    src/Utils/pffft/simd/pf_neon_double.h
    src/Utils/pffft/simd/pf_neon_double_from_avx.h
    src/Utils/pffft/simd/pf_neon_float.h
    src/Utils/pffft/simd/pf_scalar_double.h
    src/Utils/pffft/simd/pf_scalar_float.h
    src/Utils/pffft/simd/pf_sse1_float.h
    src/Utils/pffft/simd/pf_sse2_double.h
    src/Utils/pffft/pffft.cpp
    src/Utils/pffft/pffft.h
    src/Utils/pffft/pffft_double.cpp
    src/Utils/pffft/pffft_double.h
    src/Utils/pffft/pffft_common.cpp
    src/Utils/pffft/pffft_priv_impl.h
    src/Utils/SmMalloc/smmalloc.cpp
    src/Utils/SmMalloc/smmalloc.h
    src/Utils/SmMalloc/smmalloc_generic.cpp
    src/Utils/SmMalloc/smmalloc_tls.cpp
    src/Utils/intrusive_list.h
    src/Utils/Utils.cpp
    src/Utils/Utils.h
)

# Includes for this project
include_directories(src
    include
    ${AM_FLATBUFFERS_GENERATED_INCLUDES_DIR}
    ${MINIAUDIO_INCLUDE_DIRS}
)

add_library(Static STATIC ${SA_SOURCE})
target_compile_definitions(Static PUBLIC AM_BUILDSYSTEM_STATIC)

add_library(Shared SHARED ${SA_SOURCE})
target_compile_definitions(Shared PUBLIC AM_BUILDSYSTEM_SHARED)

add_custom_target(amplitude_build_binary_schemas
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_schemas.py --output ${CMAKE_CURRENT_SOURCE_DIR}/schemas
)

set(AM_BUILDSYSTEM_TYPE "Static" "Shared")

foreach (build_type ${AM_BUILDSYSTEM_TYPE})
    string(TOLOWER ${build_type} build_type_l)

    set_target_properties(${build_type}
        PROPERTIES
            OUTPUT_NAME ${PROJECT_NAME}
            ARCHIVE_OUTPUT_DIRECTORY ${build_type_l}
    )

    target_compile_definitions(${build_type} PRIVATE AM_BUILDSYSTEM_BUILDING_AMPLITUDE)
    target_compile_definitions(${build_type} PUBLIC "$<$<CONFIG:RELEASE>:AM_NO_MEMORY_STATS>")
    target_compile_definitions(${build_type} PUBLIC "$<$<CONFIG:RELEASE>:AM_NO_ASSERTS>")

    target_link_libraries(${build_type}
        PRIVATE
        flatbuffers::flatbuffers xsimd
    )

    add_dependencies(${build_type}
        generated_includes
        amplitude_build_binary_schemas
    )

    if (UNIX)
        target_link_libraries(${build_type} PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
    endif ()

    if (APPLE)
        target_link_libraries(${build_type} PUBLIC ${ACCELERATE_FRAMEWORK})
        target_compile_definitions(${build_type} PUBLIC AM_FFT_APPLE_ACCELERATE)
    endif ()
endforeach ()

if (BUILD_TOOLS)
    add_subdirectory(tools/amac)
    add_subdirectory(tools/ampk)
    add_subdirectory(tools/amir)
endif ()

if (BUILD_SAMPLES OR UNIT_TESTS)
    add_custom_target(ss_amplitude_audio_sample_project
        COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_project.py -p ${CMAKE_SOURCE_DIR}/sample_project -b ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/samples/assets -f ${CMAKE_SOURCE_DIR}/bin/flatc -s ${CMAKE_SOURCE_DIR}/schemas
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    add_custom_command(
        TARGET ss_amplitude_audio_sample_project POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/samples/assets
    )

    add_dependencies(ss_amplitude_audio_sample_project
        amplitude_build_binary_schemas
    )
endif()

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif ()

if(UNIT_TESTS)
    if (NOT MSVC)
        setup_target_for_coverage_lcov(
            NAME ss_amplitude_audio_tests_coverage
            EXECUTABLE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ss_amplitude_audio_tests
            DEPENDENCIES ss_amplitude_audio_tests
            EXCLUDE "/usr/*" "${CMAKE_BINARY_DIR}/vcpkg_installed/*" "${PROJECT_SOURCE_DIR}/src/Utils/*" "${PROJECT_SOURCE_DIR}/samples/*" "${PROJECT_SOURCE_DIR}/tools/*"
        )
    endif ()

    enable_testing()
    add_subdirectory(tests)
    message(STATUS ${PROJECT_BINARY_DIR})
endif ()

# #####################################################
# INSTALL

install(
    TARGETS Static
    EXPORT AmplitudeAudioSDKConfig
    ARCHIVE DESTINATION ${AM_LIB_DESTINATION}/static
    INCLUDES DESTINATION ${AM_INC_DESTINATION}
)

install(
    TARGETS Shared
    EXPORT AmplitudeAudioSDKConfig
    ARCHIVE DESTINATION ${AM_LIB_DESTINATION}/shared
    LIBRARY DESTINATION ${AM_LIB_DESTINATION}/shared
    RUNTIME DESTINATION ${AM_LIB_DESTINATION}/shared
    INCLUDES DESTINATION ${AM_INC_DESTINATION}
)

install(
    FILES $<TARGET_RUNTIME_DLLS:Shared>
    DESTINATION ${AM_LIB_DESTINATION}/shared
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${AM_INC_DESTINATION}
    FILES_MATCHING PATTERN "*.h*"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/schemas/
    DESTINATION ${AM_SHM_DESTINATION}
    FILES_MATCHING PATTERN "*.bfbs"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/
    DESTINATION ${AM_PYS_DESTINATION}
    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "build_schemas.py" EXCLUDE
    PATTERN "__pycache__" EXCLUDE
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    FILES_MATCHING PATTERN "*.cmake"
)

install(
    EXPORT AmplitudeAudioSDKConfig
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    NAMESPACE SparkyStudios::Audio::Amplitude::SDK::
)

export(
    TARGETS Static Shared
    NAMESPACE SparkyStudios::Audio::Amplitude::SDK::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/AmplitudeAudioSDKConfig.cmake"
)

export(PACKAGE ${PROJECT_NAME})

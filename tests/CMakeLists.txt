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

cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME ss_amplitude_audio_tests)

project(${PROJECT_NAME})

find_package(Catch2 CONFIG REQUIRED)
add_executable(${PROJECT_NAME}
    version.cpp
    filesystem.cpp
    main.cpp
    thread.cpp
    listener.cpp
    entity.cpp
    engine.cpp
    environment.cpp
    common.cpp
    math.cpp
    room.cpp
    shape.cpp
    zone.cpp
    audio_buffer.cpp
    pipeline.cpp
    fader.cpp
    hrtf.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2 Static)

add_custom_target(ss_amplitude_audio_test_package
    COMMAND $<TARGET_FILE:ampk> -q -c 0 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/samples/assets" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/samples/assets.ampk"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_dependencies(ss_amplitude_audio_test_package
    ampk
    ss_amplitude_audio_sample_project
)

add_dependencies(${PROJECT_NAME}
    Static
    ss_amplitude_audio_test_package
    ampk
)

include(Catch)
catch_discover_tests(${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    AM_SDK_PLATFORM="${VCPKG_TARGET_TRIPLET}"
    AM_SDK_PATH="$ENV{AM_SDK_PATH}"
)

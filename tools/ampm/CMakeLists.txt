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

project(ampm)

find_package(CLI11 CONFIG REQUIRED)
find_package(CMakeRC CONFIG REQUIRED)

file(GLOB_RECURSE AMPM_RES
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.*"
)

cmrc_add_resource_library(
    ampm-resources

    ALIAS ampm::rc
    NAMESPACE ampm

    ${AMPM_RES}
)

set(AMPM_SRC
    main.cpp
)

add_executable(ampm ${AMPM_SRC})

target_link_libraries(ampm
    PUBLIC ampm::rc
    PRIVATE Static CLI11::CLI11
)

add_dependencies(ampm
    Static
)

install(
    TARGETS ampm
    RUNTIME DESTINATION ${AM_BIN_DESTINATION}
)

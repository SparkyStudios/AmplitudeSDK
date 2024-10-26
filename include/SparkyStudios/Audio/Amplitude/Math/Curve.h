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

#ifndef _AM_MATH_CURVE_H
#define _AM_MATH_CURVE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>

namespace SparkyStudios::Audio::Amplitude
{
    class CurveDefinition;
    class CurvePartDefinition;

    /**
     * @brief A single point in a Curve.
     *
     * @ingroup math
     */
    struct AM_API_PUBLIC CurvePoint
    {
        /**
         * @brief The coordinates of the point over the X axis.
         */
        AmReal64 x;

        /**
         * @brief The coordinates of the point over the Y axis.
         */
        AmReal32 y;

        bool operator==(const CurvePoint& rhs) const;
        bool operator!=(const CurvePoint& rhs) const;
    };

    /**
     * @brief A segment of a `Curve`.
     *
     * A `CurvePart` allows a curve to have different fading algorithms at the same time.
     * Each `CurvePart` has a start and end point, and the fading algorithm which moves the value
     * from the start point to the end point.
     *
     * @ingroup math
     */
    class AM_API_PUBLIC CurvePart
    {
    public:
        /**
         * @brief Creates an empty `CurvePart`.
         */
        CurvePart();

        /**
         * @brief Copy constructor.
         *
         * @param[in] other The `CurvePart` to copy.
         */
        CurvePart(const CurvePart& other);

        /**
         * @brief Destroys this `CurvePart`.
         */
        ~CurvePart();

        /**
         * @brief Initializes this `CurvePart` from a definition.
         *
         * @param[in] definition The definition of the curve part generated
         * from a flatbuffer binary.
         */
        void Initialize(const CurvePartDefinition* definition);

        /**
         * @brief Returns the start point of this `CurvePart`.
         *
         * @return The start point of this `CurvePart`.
         */
        [[nodiscard]] const CurvePoint& GetStart() const;

        /**
         * @brief Sets the start point of this `CurvePart`.
         *
         * @param[in] start The new start point.
         */
        void SetStart(const CurvePoint& start);

        /**
         * @brief Returns the end point of this `CurvePart`.
         *
         * @return The end point of this `CurvePart`.
         */
        [[nodiscard]] const CurvePoint& GetEnd() const;

        /**
         * @brief Sets the end point of this `CurvePart`.
         *
         * @param[in] end The new end point.
         */
        void SetEnd(const CurvePoint& end);

        /**
         * @brief Returns the Fader of this `CurvePart`.
         *
         * @return The `FaderInstance` of this `CurvePart`.
         */
        [[nodiscard]] FaderInstance* GetFader() const;

        /**
         * @brief Sets the fader of this `CurvePart`.
         *
         * @param[in] fader The name of the `Fader` to set.
         */
        void SetFader(const AmString& fader);

        /**
         * @brief Gets the Y coordinates of a point given its coordinates over the X axis.
         *
         * @param[in] x The coordinates of the point over the X axis.
         *
         * @return The Y coordinates of the point.
         */
        [[nodiscard]] AmReal32 Get(AmReal64 x) const;

        /**
         * @brief Copy assignment operator.
         *
         * @param[in] other The other `CurvePart` to copy.
         *
         * @return A copy of this `CurvePart`.
         */
        CurvePart& operator=(const CurvePart& other);

    private:
        friend class Curve;

        CurvePoint _start;
        CurvePoint _end;

        Fader* _faderFactory;
        FaderInstance* _fader;
    };

    /**
     * @brief A `Curve` which describe the variation of a value (on the Y-axis) according to another (on the X-axis).
     *
     * @ingroup math
     */
    class AM_API_PUBLIC Curve
    {
    public:
        /**
         * @brief Creates an empty `Curve`.
         */
        Curve();

        /**
         * @brief Initializes curve parts from the given definition.
         *
         * @param[in] definition The curve definition data.
         */
        void Initialize(const CurveDefinition* definition);

        /**
         * @brief Initializes curve parts from the given vector.
         *
         * @param[in] parts The vector of curve parts.
         */
        void Initialize(const std::vector<CurvePart>& parts);

        /**
         * @brief Get the curve value corresponding to the given X value.
         *
         * @param[in] x The X value.
         *
         * @return The curve value.
         */
        [[nodiscard]] AmReal32 Get(AmReal64 x) const;

    private:
        // Finds the curve part corresponding to the given X value.
        [[nodiscard]] const CurvePart* _findCurvePart(AmReal64 x) const;

        // The parts of the curve.
        std::vector<CurvePart> _parts;
    };
} // namespace SparkyStudios::Audio::Amplitude
#endif // _AM_MATH_CURVE_H

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

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("Barycentric Coordinate Tests", "[barycentric_coordinates][math][amplitude]")
{
    const auto point1 = AM_V3(1, 2, 1);
    const auto point2 = AM_V3(0, 0, 0);
    const auto point3 = AM_V3(2, 1, 2);

    const auto center = AM_V3(1, 1, 1);
    const auto triangle = std::array<AmVec3, 3>{ point1, point2, point3 };

    const auto i = point1 - center;
    const auto j = point2 - center;
    const auto k = AM_Cross(i, j);

    const auto rayOrigin = center;
    const auto rayDirection = k;

    SECTION("can compute the barycentric coordinates of a point inside the triangle")
    {
        BarycentricCoordinates coordinates(center, triangle);

        REQUIRE(coordinates.IsValid());

        REQUIRE(coordinates.m_U + coordinates.m_V + coordinates.m_W == 1.0f);

        REQUIRE((coordinates.m_U - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_V - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_W - 1.0f / 3.0f) < kEpsilon);
    }

    SECTION("can compute the barycentric coordinates of a ray-triangle intersection")
    {
        BarycentricCoordinates coordinates;
        REQUIRE(BarycentricCoordinates::RayTriangleIntersection(rayOrigin, rayDirection, triangle, coordinates));

        REQUIRE(coordinates.IsValid());

        REQUIRE(coordinates.m_U + coordinates.m_V + coordinates.m_W == 1.0f);

        REQUIRE((coordinates.m_U - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_V - 1.0f / 3.0f) < kEpsilon);
        REQUIRE((coordinates.m_W - 1.0f / 3.0f) < kEpsilon);
    }
}

TEST_CASE("Cartesian Coordinate System Tests", "[cartesian_coordinate_system][math][amplitude]")
{
    GIVEN("a right-handed z-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::RightHandedZUp();

        THEN("it have the correct axes")
        {
            REQUIRE(AM_EqV3(coordinateSystem.GetRightVector(), AM_V3(1, 0, 0)));
            REQUIRE(AM_EqV3(coordinateSystem.GetUpVector(), AM_V3(0, 0, 1)));
            REQUIRE(AM_EqV3(coordinateSystem.GetForwardVector(), AM_V3(0, 1, 0)));
        }
    }

    GIVEN("a right-handed y-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::RightHandedYUp();

        THEN("it have the correct axes")
        {
            REQUIRE(AM_EqV3(coordinateSystem.GetRightVector(), AM_V3(1, 0, 0)));
            REQUIRE(AM_EqV3(coordinateSystem.GetUpVector(), AM_V3(0, 1, 0)));
            REQUIRE(AM_EqV3(coordinateSystem.GetForwardVector(), AM_V3(0, 0, -1)));
        }
    }

    GIVEN("a left-handed z-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::LeftHandedZUp();

        THEN("it have the correct axes")
        {
            REQUIRE(AM_EqV3(coordinateSystem.GetRightVector(), AM_V3(1, 0, 0)));
            REQUIRE(AM_EqV3(coordinateSystem.GetUpVector(), AM_V3(0, 0, 1)));
            REQUIRE(AM_EqV3(coordinateSystem.GetForwardVector(), AM_V3(0, -1, 0)));
        }
    }

    GIVEN("a left-handed y-up Cartesian coordinate system")
    {
        const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::LeftHandedYUp();

        THEN("it have the correct axes")
        {
            REQUIRE(AM_EqV3(coordinateSystem.GetRightVector(), AM_V3(1, 0, 0)));
            REQUIRE(AM_EqV3(coordinateSystem.GetUpVector(), AM_V3(0, 1, 0)));
            REQUIRE(AM_EqV3(coordinateSystem.GetForwardVector(), AM_V3(0, 0, 1)));
        }
    }

    GIVEN("a Cartesian coordinate system with arbitrary axes")
    {
        constexpr auto rightVector = CartesianCoordinateSystem::Axis::NegativeX;
        constexpr auto upVector = CartesianCoordinateSystem::Axis::PositiveY;
        constexpr auto forwardVector = CartesianCoordinateSystem::Axis::NegativeZ;

        const CartesianCoordinateSystem coordinateSystem(rightVector, forwardVector, upVector);

        THEN("it have the correct axes")
        {
            REQUIRE(AM_EqV3(coordinateSystem.GetRightVector(), CartesianCoordinateSystem::GetVector(rightVector)));
            REQUIRE(AM_EqV3(coordinateSystem.GetUpVector(), CartesianCoordinateSystem::GetVector(upVector)));
            REQUIRE(AM_EqV3(coordinateSystem.GetForwardVector(), CartesianCoordinateSystem::GetVector(forwardVector)));
        }
    }

    SECTION("coordinate system conversion")
    {
        const auto from = CartesianCoordinateSystem::Default();
        const auto to = CartesianCoordinateSystem::AmbiX();

        GIVEN("a point in the default coordinate system")
        {
            const auto point = AM_V3(1, 2, 3);

            WHEN("converted to AmbiX coordinate system")
            {
                const auto convertedPoint = CartesianCoordinateSystem::Convert(point, from, to);

                THEN("it should have the same position in the AmbiX coordinate system")
                {
                    REQUIRE(AM_EqV3(convertedPoint, AM_V3(2, -1, 3)));
                }

                THEN("it should convert back to the original coordinate system")
                {
                    const auto convertedBackPoint = CartesianCoordinateSystem::Convert(convertedPoint, to, from);

                    REQUIRE(AM_EqV3(convertedBackPoint, point));
                }

                THEN("it should match the point using the Converter API")
                {
                    const auto converter = CartesianCoordinateSystem::Converter(from, to);
                    const auto convertedPoint2 = converter.Forward(point);

                    REQUIRE(AM_EqV3(convertedPoint2, convertedPoint));

                    AND_THEN("it should convert back to the original coordinate system using the Converter API")
                    {
                        const auto convertedBackPoint2 = converter.Backward(convertedPoint);

                        REQUIRE(AM_EqV3(convertedBackPoint2, point));
                    }
                }
            }
        }

        GIVEN("a quaternion rotation in the default coordinate system")
        {
            const auto rotation = AM_QFromAxisAngle_RH(AM_V3(0, 1, 0), AM_DegToRad * 45.0f);

            WHEN("converted to AmbiX coordinate system")
            {
                const auto convertedRotation = CartesianCoordinateSystem::Convert(rotation, from, to);

                THEN("it should have the same rotation in the AmbiX coordinate system")
                {
                    REQUIRE(AM_EqV3(convertedRotation.XYZ, AM_V3(rotation.Y, -rotation.X, rotation.Z)));
                    REQUIRE(convertedRotation.W == rotation.W);
                }

                THEN("it should convert back to the original coordinate system")
                {
                    const auto convertedBackRotation = CartesianCoordinateSystem::Convert(convertedRotation, to, from);

                    REQUIRE(AM_EqV3(convertedBackRotation.XYZ, rotation.XYZ));
                    REQUIRE(convertedBackRotation.W == rotation.W);
                }

                THEN("it should match the rotation using the Converter API")
                {
                    const auto converter = CartesianCoordinateSystem::Converter(from, to);
                    const auto convertedRotation2 = converter.Forward(rotation);

                    REQUIRE(AM_EqV3(convertedRotation2.XYZ, convertedRotation.XYZ));
                    REQUIRE(convertedRotation2.W == convertedRotation.W);

                    AND_THEN("it should convert back to the original coordinate system using the Converter API")
                    {
                        const auto convertedBackRotation2 = converter.Backward(convertedRotation);

                        REQUIRE(AM_EqV3(convertedBackRotation2.XYZ, rotation.XYZ));
                        REQUIRE(convertedBackRotation2.W == rotation.W);
                    }
                }
            }
        }

        GIVEN("a scalar in the default coordinate system")
        {
            constexpr auto scalar = 5.0f;

            WHEN("converted to AmbiX coordinate system")
            {
                const auto convertedScalar = CartesianCoordinateSystem::Convert(scalar, from, to);

                THEN("it should have the same scalar in the AmbiX coordinate system")
                {
                    REQUIRE(convertedScalar == scalar);
                }

                THEN("it should convert back to the original coordinate system")
                {
                    const auto convertedBackScalar = CartesianCoordinateSystem::Convert(convertedScalar, to, from);

                    REQUIRE(convertedBackScalar == scalar);
                }

                THEN("it should match the scalar using the Converter API")
                {
                    const auto converter = CartesianCoordinateSystem::Converter(from, to);
                    const auto convertedScalar2 = converter.Forward(scalar);

                    REQUIRE(convertedScalar2 == convertedScalar);

                    AND_THEN("it should convert back to the original coordinate system using the Converter API")
                    {
                        const auto convertedBackScalar2 = converter.Backward(convertedScalar);

                        REQUIRE(convertedBackScalar2 == scalar);
                    }
                }
            }
        }
    }
}

TEST_CASE("Spherical Position Tests", "[spherical_position][math][amplitude]")
{
    GIVEN("a spherical position")
    {
        SphericalPosition position(AM_DegToRad * 45.0f, AM_DegToRad * 30.0f, 5.0f);

        THEN("it should store the correct spherical coordinates")
        {
            REQUIRE(position.GetAzimuth() == AM_DegToRad * 45.0f);
            REQUIRE(position.GetElevation() == AM_DegToRad * 30.0f);
            REQUIRE(position.GetRadius() == 5.0f);
        }

        THEN("it can convert to Cartesian coordinates")
        {
            const auto cartesianPosition = position.ToCartesian();

            REQUIRE(cartesianPosition.X == +5.0f * std::cos(position.GetElevation()) * std::cos(position.GetAzimuth()));
            REQUIRE(cartesianPosition.Y == -5.0f * std::cos(position.GetElevation()) * std::sin(position.GetAzimuth()));
            REQUIRE(cartesianPosition.Z == +5.0f * std::sin(position.GetElevation()));
        }

        THEN("it can flip azimuth")
        {
            const auto flippedPosition = position.FlipAzimuth();

            REQUIRE(flippedPosition.GetAzimuth() == -45.0f * AM_DegToRad);
            REQUIRE(flippedPosition.GetElevation() == position.GetElevation());
            REQUIRE(flippedPosition.GetRadius() == position.GetRadius());
        }

        THEN("it can change azimuth, elevation and radius")
        {
            position.SetAzimuth(AM_DegToRad * 60.0f);
            position.SetElevation(AM_DegToRad * 45.0f);
            position.SetRadius(10.0f);

            REQUIRE(position.GetAzimuth() == AM_DegToRad * 60.0f);
            REQUIRE(position.GetElevation() == AM_DegToRad * 45.0f);
            REQUIRE(position.GetRadius() == 10.0f);
        }

        THEN("it can be rotated")
        {
            const auto rotation = AM_QFromAxisAngle_RH(AM_V3(0, 0, 1), AM_DegToRad * 90.0f);
            const auto rotatedPosition = position.Rotate(rotation);

            const auto rotatedPosition2 = SphericalPosition::FromWorldSpace(AM_RotateV3Q(position.ToCartesian(), rotation));

            REQUIRE(rotatedPosition.GetAzimuth() == rotatedPosition2.GetAzimuth());
            REQUIRE(rotatedPosition.GetElevation() == rotatedPosition2.GetElevation());
            REQUIRE(rotatedPosition.GetRadius() == rotatedPosition2.GetRadius());
        }

        SECTION("equality comparison")
        {
            const auto otherPosition = SphericalPosition(AM_DegToRad * 45.0f, AM_DegToRad * 30.0f, 5.0f);

            THEN("it should be equal to itself")
            {
                REQUIRE(position == position);
            }

            THEN("it should be equal to another spherical position with the same coordinates")
            {
                REQUIRE(position == otherPosition);
            }

            THEN("it should not be equal to another spherical position with different coordinates")
            {
                const auto differentPosition = SphericalPosition(AM_DegToRad * 60.0f, AM_DegToRad * 45.0f, 5.0f);

                REQUIRE(position != differentPosition);
            }
        }
    }

    GIVEN("a cartesian position")
    {
        const auto cartesianPosition = AM_V3(5.0f, 3.0f, 4.0f);

        THEN("it can convert to spherical coordinates in world space")
        {
            const auto sphericalPosition = SphericalPosition::FromWorldSpace(cartesianPosition);

            REQUIRE(sphericalPosition.GetAzimuth() == -std::atan2(cartesianPosition.Y, cartesianPosition.X));
            REQUIRE(sphericalPosition.GetElevation() == std::atan2(cartesianPosition.Z, AM_Len(cartesianPosition.XY)));
            REQUIRE(sphericalPosition.GetRadius() == AM_Len(cartesianPosition));
        }

        THEN("it can convert to spherical coordinates in AmbiX space")
        {
            const auto sphericalPosition = SphericalPosition::ForHRTF(cartesianPosition);

            REQUIRE(sphericalPosition.GetAzimuth() == 90.0f * AM_DegToRad - std::atan2(cartesianPosition.Y, cartesianPosition.X));
            REQUIRE(sphericalPosition.GetElevation() == std::atan2(cartesianPosition.Z, AM_Len(cartesianPosition.XY)));
            REQUIRE(sphericalPosition.GetRadius() == AM_Len(cartesianPosition));
        }
    }

    GIVEN("values in degrees")
    {
        constexpr auto azimuth = 45.0f;
        constexpr auto elevation = 30.0f;
        constexpr auto radius = 5.0f;

        THEN("it should store the correct spherical coordinates")
        {
            const auto sphericalPosition = SphericalPosition::FromDegrees(azimuth, elevation, radius);

            REQUIRE(sphericalPosition.GetAzimuth() == azimuth * AM_DegToRad);
            REQUIRE(sphericalPosition.GetElevation() == elevation * AM_DegToRad);
            REQUIRE(sphericalPosition.GetRadius() == radius);
        }
    }
}

TEST_CASE("Curve Tests", "[curve][math][amplitude]")
{
    SECTION("curve points")
    {
        CurvePoint a{ 0.5, 1.0f };
        CurvePoint b{ 1.0, 2.0f };
        CurvePoint c{ 0.5, 1.0f };

        THEN("it should be equal to itself")
        {
            REQUIRE(a == a);
        }

        THEN("it should be equal to another curve point with the same coordinates")
        {
            REQUIRE(a == c);
        }

        THEN("it should not be equal to another curve point with different coordinates")
        {
            REQUIRE(a != b);
            REQUIRE(b != c);
        }
    }

    SECTION("curve parts")
    {
        CurvePart part1;

        CurvePoint zero{ 0.0, 0.0f };
        CurvePoint one{ 1.0, 1.0f };

        THEN("it cannot be initialized with a null definition")
        {
            part1.Initialize(nullptr);
            REQUIRE(part1.GetStart() == zero);
            REQUIRE(part1.GetEnd() == zero);
        }

        THEN("it can receive a start point")
        {
            part1.SetStart(one);
            REQUIRE(part1.GetStart() == one);
        }

        THEN("it can receive an end point")
        {
            part1.SetEnd(zero);
            REQUIRE(part1.GetEnd() == zero);
        }

        THEN("it can receive a fader")
        {
            part1.SetFader("Linear");
            REQUIRE(part1.GetFader() != nullptr);

            AND_THEN("it cannot use an invalid fader")
            {
                part1.SetFader("Invalid");
                REQUIRE(part1.GetFader() == nullptr);
            }
        }

        THEN("it can get the right values")
        {
            part1.SetFader("Linear");
            part1.SetStart(zero);
            part1.SetEnd(one);

            for (AmReal64 t = 0.0; t <= 1.0; t += 0.1)
            {
                const AmReal32 value = part1.Get(t);
                REQUIRE(value - t < kEpsilon);
            }
        }
    }

    SECTION("curve")
    {
        CurvePoint zero{ 0.0, 0.0f };
        CurvePoint middle{ 0.5, 1.0f };
        CurvePoint one{ 1.0, 0.0f };

        CurvePart part1;
        part1.SetStart(zero);
        part1.SetEnd(middle);
        part1.SetFader("Linear");

        CurvePart part2;
        part2.SetStart(middle);
        part2.SetEnd(one);
        part2.SetFader("Linear");

        Curve curve;

        THEN("cannot be initialized with a null definition")
        {
            curve.Initialize(nullptr);

            REQUIRE(curve.Get(0.0) == 0.0f);
        }

        THEN("can be initialized with curve parts")
        {
            const std::vector<CurvePart> parts = { part1, part2 };
            curve.Initialize(parts);

            REQUIRE(curve.Get(0.0) - 0.0f < kEpsilon);
            REQUIRE(curve.Get(0.5) - 1.0f < kEpsilon);
            REQUIRE(curve.Get(1.0) - 0.0f < kEpsilon);
        }

        THEN("can get the right values")
        {
            curve.Initialize({ part1, part2 });

            for (AmReal64 t = 0.0; t <= 0.5; t += 0.1)
            {
                const AmReal32 value = curve.Get(t);
                REQUIRE(value - (t * 2) < kEpsilon);
            }

            for (AmReal64 t = 0.5; t <= 1.0; t += 0.1)
            {
                const AmReal32 value = curve.Get(t);
                REQUIRE(value - 2 - 2 * t < kEpsilon);
            }
        }

        THEN("cannot get values outside the range")
        {
            curve.Initialize({ part1, part2 });

            REQUIRE(curve.Get(-1.0) == 0.0f);
            REQUIRE(curve.Get(2.0) == 0.0f);
        }
    }
}

TEST_CASE("Utilities Tests", "[utilities][math][amplitude]")
{
    THEN("dithering always works")
    {
        // This is only to increase coverage, as the dithering function is a simple function that always returns a value
        AmDitherReal32(1.0f / INT16_MIN, 1.0f / INT16_MAX);
    }

    SECTION("floating-point to fixed-point conversion")
    {
        THEN("it can convert floating-point audio sample to fixed-point")
        {
            constexpr AmReal32 value = 0.5f;
            const AmInt32 fixedPointValue = AmFloatToFixedPoint(value);
            REQUIRE(std::abs(fixedPointValue - 16384) < kEpsilon);
        }

        THEN("it can convert 16-bit integer to 32-bit floating-point")
        {
            constexpr AmInt16 fxp = 16384;
            const AmReal32 value = AmInt16ToReal32(fxp);
            REQUIRE(std::abs(value - 0.5f) < kEpsilon);
        }

        THEN("it can convert 32-bit integer to 32-bit floating-point")
        {
            constexpr AmInt32 value = 16384;
            const AmReal32 fxp = AmInt32ToReal32(value);
            REQUIRE(std::abs(fxp - 0.5f) < kEpsilon);
        }

        THEN("it can convert 32-bit floating-point to 16-bit integer")
        {
            constexpr AmReal32 value = 0.5f;

            const AmInt16 fxp1 = AmReal32ToInt16(value, false);
            REQUIRE((fxp1 - 16384) < kEpsilon);

            const AmInt16 fxp2 = AmReal32ToInt16(value, true);
            REQUIRE((fxp2 - 16384) < kEpsilon);

            REQUIRE(std::abs(fxp1 - fxp2) < kEpsilon);
        }
    }

    SECTION("catmull_rom")
    {
        constexpr AmReal32 p1 = 0.0f;
        constexpr AmReal32 p2 = 1.0f;
        constexpr AmReal32 p3 = 2.0f;
        constexpr AmReal32 p4 = 3.0f;

        REQUIRE(CatmullRom(0.0f, p1, p2, p3, p4) == 1.0f);
        REQUIRE(CatmullRom(1.0f, p1, p2, p3, p4) == 2.0f);
        REQUIRE(CatmullRom(0.5f, p1, p2, p3, p4) == 1.5f);
    }

    SECTION("doppler factor")
    {
        constexpr AmReal32 soundSpeed = 343.0f;
        constexpr AmVec3 source = { 10.0f, 25.0f, 1.0f };
        constexpr AmVec3 listener = { 0.0f, 0.0f, 0.0f };

        const AmReal32 dopplerFactor = ComputeDopplerFactor(source - listener, source, listener, soundSpeed, 1.0f);

        REQUIRE(std::abs(dopplerFactor - 0.927166343f) < kEpsilon);
    }

    SECTION("next power of two")
    {
        REQUIRE(NextPowerOf2(1) == 1);
        REQUIRE(NextPowerOf2(2) == 2);
        REQUIRE(NextPowerOf2(3) == 4);
        REQUIRE(NextPowerOf2(16) == 16);
        REQUIRE(NextPowerOf2(250) == 256);
        REQUIRE(NextPowerOf2(4000) == 4096);
        REQUIRE(NextPowerOf2(1024) == 1024);
        REQUIRE(NextPowerOf2(2024) == 2048);
        REQUIRE(NextPowerOf2(4096) == 4096);
    }

    SECTION("integer pow")
    {
        REQUIRE(IntegerPow(2, 0) == 1);
        REQUIRE(IntegerPow(2.5, 1) == 2.5);
        REQUIRE(IntegerPow(3.1f, 2) == 9.61f);
        REQUIRE(IntegerPow(2, 10) == 1024);
    }

    SECTION("gcd")
    {
        REQUIRE(FindGCD(12, 18) == 6);
        REQUIRE(FindGCD(20, 30) == 10);
        REQUIRE(FindGCD(48, 144) == 48);
        REQUIRE(FindGCD(-100, 200) == 100);
        REQUIRE(FindGCD(0, 200) == 200);
    }
}
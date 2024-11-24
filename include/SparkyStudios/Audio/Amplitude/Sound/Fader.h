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

#ifndef _AM_SOUND_FADER_H
#define _AM_SOUND_FADER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Enumerates the list of states in a fader.
     *
     * @ingroup engine
     */
    enum eFaderState : AmInt8
    {
        eFaderState_Stopped = -1, ///< The fader is stopped
        eFaderState_Disabled = 0, ///< The fader is disabled
        eFaderState_Active = 1, ///< The fader is active and fading
    };

    /**
     * @brief A Fader instance. An object of this class will be created each time a `Fader` is requested.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC FaderInstance
    {
    public:
        /**
         * @brief Create an animation transition function using
         * a one-dimensional cubic Bézier curve.
         *
         * This use the exact same algorithm as in CSS. The first and last
         * control points of the cubic Bézier curve are fixed to (0,0)
         * and (1,1) respectively.
         */
        struct Transition
        {
        public:
            /**
             * @brief Constructs a new Transition curve.
             *
             * @param[in] x1 The x coordinate of the second control point.
             * @param[in] y1 The y coordinate of the second control point.
             * @param[in] x2 The x coordinate of the third control point.
             * @param[in] y2 The y coordinate of the third control point.
             */
            Transition(AmReal32 x1, AmReal32 y1, AmReal32 x2, AmReal32 y2);

            /**
             * @brief Constructs a new Transition curve.
             *
             * @param[in] controlPoints The control points of the curve.
             */
            Transition(const BezierCurveControlPoints& controlPoints);

            /**
             * @brief Given an animation duration percentage (in the range [0, 1]),
             * it calculates the animation progression percentage from the configured curve.
             *
             * @param[in] t The animation duration percentage (in the range [0, 1]).
             *
             * @return The animation progress percentage (in the range [0, 1]).
             */
            [[nodiscard]] AmTime Ease(AmTime t) const;

            /**
             * @brief The control points.
             */
            BezierCurveControlPoints m_controlPoints;

        private:
            [[nodiscard]] AmTime GetTFromX(AmReal64 x) const;

            AmReal64 _samples[11];
        };

        /**
         * @brief Constructs a new FaderInstance object.
         *
         * This will initialize the fader instance state to default values.
         */
        FaderInstance();

        /**
         * @brief Default destructor.
         */
        virtual ~FaderInstance() = default;

        /**
         * @brief Set up fader.
         *
         * @param[in] from The start value.
         * @param[in] to The target value.
         * @param[in] duration The duration of transition.
         */
        void Set(AmReal64 from, AmReal64 to, AmTime duration);

        /**
         * @brief Set up fader.
         *
         * @param[in] from The start value.
         * @param[in] to The target value.
         */
        void Set(AmReal64 from, AmReal64 to);

        /**
         * @brief Sets the duration of the transition.
         *
         * @param[in] duration The transition duration.
         */
        void SetDuration(AmTime duration);

        /**
         * @brief Gets the current fading value.
         *
         * To use this method you first need to define the fading start time using
         * @ref Start `Start()`.
         *
         * @param[in] time The time at which the value should be calculated.
         *
         * @return The current value.
         */
        virtual AmReal64 GetFromTime(AmTime time);

        /**
         * @brief Gets the current fading value.
         *
         * @param[in] percentage The percentage of time elapsed. This should be in the range [0, 1].
         *
         * @return The current value.
         */
        virtual AmReal64 GetFromPercentage(AmReal64 percentage);

        /**
         * @brief Gets the state of this Fader.
         *
         * @return The Fader state.
         */
        [[nodiscard]] AM_INLINE eFaderState GetState() const
        {
            return m_state;
        }

        /**
         * @brief Sets the state of this Fader.
         *
         * @param[in] state The state to set.
         */
        AM_INLINE void SetState(eFaderState state)
        {
            m_state = state;
        }

        /**
         * @brief Sets the fading start time.
         *
         * @param[in] time The fading start time.
         */
        void Start(AmTime time = 0.0);

    protected:
        // Value to fade from
        AmReal64 m_from;
        // Value to fade to
        AmReal64 m_to;
        // Delta between from and to
        AmReal64 m_delta;
        // Total AmTime to fade
        AmTime m_time;
        // Time fading started
        AmTime m_startTime;
        // Time fading will end
        AmTime m_endTime;
        // Active flag; 0 means disabled, 1 is active, -1 means was active, but stopped
        eFaderState m_state;
        // The transition function
        Transition m_curve;
    };

    /**
     * @brief Helper class to process faders.
     *
     * A fader is used to move a value to a specific target value
     * during an amount of time and according to a fading algorithm.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Fader
    {
    public:
        /**
         * @brief Create a new Fader instance.
         *
         * @param name The Fader name. eg. "MiniAudioLinear".
         */
        explicit Fader(AmString name);

        /**
         * @brief Default Fader constructor.
         *
         * This will not automatically register the Fader. It's meant for internal Faders only.
         */
        Fader();

        /**
         * @brief Default destructor.
         */
        virtual ~Fader();

        /**
         * @brief Creates a new instance of the Fader.
         *
         * @return A new instance of the Fader.
         */
        virtual FaderInstance* CreateInstance() = 0;

        /**
         * @brief Destroys an instance of the Fader.
         *
         * @note The instance should have been created with CreateInstance().
         *
         * @param[in] instance The Fader instance to be destroyed.
         */
        virtual void DestroyInstance(FaderInstance* instance) = 0;

        /**
         * @brief Gets the name of this Fader.
         *
         * @return The name of this Fader.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Gets the control points of the transition curve used by this Fader.
         *
         * @return The control points of the transition curve used by this Fader.
         */
        [[nodiscard]] virtual BezierCurveControlPoints GetControlPoints() const = 0;

        /**
         * @brief Registers a new fader.
         *
         * @param[in] fader The Fader to add in the registry.
         */
        static void Register(Fader* fader);

        /**
         * @brief Unregister a fader.
         *
         * @param[in] fader The Fader to remove from the registry.
         */
        static void Unregister(const Fader* fader);

        /**
         * @brief Creates a new instance of the fader with the given name and returns its pointer.
         *
         * @note The returned pointer should be deleted using @ref Destruct `Destruct()`.
         *
         * @param[in] name The name of the fader.
         *
         * @return The fader with the given name, or `nullptr` if none.
         */
        static FaderInstance* Construct(const AmString& name);

        /**
         * @brief Destroys the given fader instance.
         *
         * @param[in] name The name of the fader.
         * @param[in] instance The fader instance to destroy.
         */
        static void Destruct(const AmString& name, FaderInstance* instance);

        /**
         * @brief Locks the faders registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the `Engine` initialization, to discard the registration
         * of new fader after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the fader's registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the `Engine` deinitialization, to allow the registration
         * of new fader after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered Faders.
         *
         * @return The registry of Faders.
         */
        static const std::map<AmString, Fader*>& GetRegistry();

        /**
         * @brief Look up a Fader by name.
         *
         * @return The Fader with the given name, or NULL if none.
         *
         * @internal
         */
        static Fader* Find(const AmString& name);

    protected:
        /**
         * @brief The name of this Fader.
         */
        AmString m_name;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_SOUND_FADER_H

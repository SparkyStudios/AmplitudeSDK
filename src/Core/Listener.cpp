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

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>

#include <Core/ListenerInternalState.h>

namespace SparkyStudios::Audio::Amplitude
{
    Listener::Listener()
        : _state(nullptr)
    {}

    Listener::Listener(ListenerInternalState* state)
        : _state(state)
    {}

    void Listener::Clear()
    {
        _state = nullptr;
    }

    bool Listener::Valid() const
    {
        return _state != nullptr && _state->GetId() != kAmInvalidObjectId && _state->node.in_list();
    }

    AmListenerID Listener::GetId() const
    {
        return _state->GetId();
    }

    const AmVec3& Listener::GetVelocity() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetVelocity();
    }

    void Listener::SetOrientation(const Orientation& orientation) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetOrientation(orientation);
    }

    Orientation Listener::GetOrientation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetOrientation();
    }

    void Listener::SetDirectivity(AmReal32 directivity, AmReal32 sharpness) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetDirectivity(directivity, sharpness);
    }

    AmReal32 Listener::GetDirectivity() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetDirectivity();
    }

    AmReal32 Listener::GetDirectivitySharpness() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetDirectivitySharpness();
    }

    const AmMat4& Listener::GetInverseMatrix() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetInverseMatrix();
    }

    const AmVec3& Listener::GetLocation() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetLocation();
    }

    void Listener::SetLocation(const AmVec3& location) const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->SetLocation(location);
    }

    AmVec3 Listener::GetDirection() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetDirection();
    }

    AmVec3 Listener::GetUp() const
    {
        AMPLITUDE_ASSERT(Valid());
        return _state->GetUp();
    }

    void Listener::Update() const
    {
        AMPLITUDE_ASSERT(Valid());
        _state->Update();
    }

    ListenerInternalState* Listener::GetState() const
    {
        return _state;
    }
} // namespace SparkyStudios::Audio::Amplitude

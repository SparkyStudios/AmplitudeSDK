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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Mixer/SoundData.h>
#include <Sound/Sound.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static AmObjectID gLastSoundInstanceID = 0;

    SoundImpl::SoundImpl()
        : SoundObjectImpl()
        , _codec(nullptr)
        , _decoder(nullptr)
        , _stream(false)
        , _loop(false)
        , _loopCount(0)
        , _soundData(nullptr)
        , _format()
        , _soundDataRefCounter()
        , _settings()
    {}

    SoundImpl::~SoundImpl()
    {
        if (_decoder != nullptr)
        {
            _decoder->Close();
            _codec->DestroyDecoder(_decoder);

            _decoder = nullptr;
            _codec = nullptr;
        }

        if (_soundData != nullptr)
        {
            AMPLITUDE_ASSERT(_soundDataRefCounter.GetCount() == 0);

            SoundChunk::DestroyChunk(_soundData);
            _soundData = nullptr;
        }

        m_bus = nullptr;
        m_effect = nullptr;
        m_attenuation = nullptr;
    }

    SoundInstance* SoundImpl::CreateInstance()
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);
        return ampoolnew(eMemoryPoolKind_Engine, SoundInstance, this, _settings, m_effect);
    }

    SoundInstance* SoundImpl::CreateInstance(const CollectionImpl* collection)
    {
        if (collection == nullptr)
            return CreateInstance();

        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        auto* sound = ampoolnew(eMemoryPoolKind_Engine, SoundInstance, this, collection->_soundSettings.at(m_id), collection->m_effect);
        sound->_collection = collection;

        return sound;
    }

    SoundChunk* SoundImpl::AcquireSoundData()
    {
        if (_stream)
            return nullptr;

        if (_soundDataRefCounter.GetCount() == 0)
        {
            if (_decoder == nullptr)
            {
                amLogError("Could not load a sound instance. No decoder was initialized. Make sure the codec able to decode the audio file "
                           "at '" AM_OS_CHAR_FMT "' is registered to the engine.", m_filename.c_str());
                return nullptr;
            }

            _soundData = SoundChunk::CreateChunk(_format.GetFramesCount(), _format.GetNumChannels());

            if (_decoder->Load(_soundData->buffer) != _format.GetFramesCount())
            {
                SoundChunk::DestroyChunk(_soundData);
                amLogError("Could not load a sound instance. Unable to read data from the parent sound.");
                return nullptr;
            }
        }

        _soundDataRefCounter.Increment();
        return _soundData;
    }

    void SoundImpl::ReleaseSoundData()
    {
        if (_stream)
            return;

        _soundDataRefCounter.Decrement();

        if (_soundDataRefCounter.GetCount() == 0)
        {
            SoundChunk::DestroyChunk(_soundData);
            _soundData = nullptr;
        }
    }

    AmObjectID SoundImpl::GetId() const
    {
        return AssetImpl::GetId();
    }
    const AmString& SoundImpl::GetName() const
    {
        return AssetImpl::GetName();
    }

    bool SoundImpl::IsStream() const
    {
        return _stream;
    }

    bool SoundImpl::IsLoop() const
    {
        return _loop;
    }

    void SoundImpl::Load(const FileSystem* loader)
    {
        const AmOsString& filename = GetPath();

        if (filename.empty())
        {
            amLogError("Cannot load the sound: the filename is empty.");
            return;
        }

        if (!loader->Exists(filename))
        {
            amLogError("Cannot load the sound: the file '" AM_OS_CHAR_FMT "' does not exist.", filename.c_str());
            return;
        }

        const auto file = loader->OpenFile(filename);

        _codec = Codec::FindCodecForFile(file);
        if (_codec == nullptr)
        {
            amLogError("Cannot load the sound: unable to find codec for '" AM_OS_CHAR_FMT "'.", filename.c_str());
            return;
        }

        _decoder = _codec->CreateDecoder();
        if (!_decoder->Open(file))
        {
            amLogError("Cannot load the sound: unable to initialize a decoder for '" AM_OS_CHAR_FMT "'.", filename.c_str());
            return;
        }

        _format = _decoder->GetFormat();
    }

    const RtpcValue& SoundImpl::GetGain() const
    {
        return SoundObjectImpl::GetGain();
    }

    const RtpcValue& SoundImpl::GetNearFieldGain() const
    {
        return _nearFieldGain;
    }

    const RtpcValue& SoundImpl::GetPitch() const
    {
        return SoundObjectImpl::GetPitch();
    }

    const RtpcValue& SoundImpl::GetPriority() const
    {
        return SoundObjectImpl::GetPriority();
    }

    const Effect* SoundImpl::GetEffect() const
    {
        return SoundObjectImpl::GetEffect();
    }

    const Attenuation* SoundImpl::GetAttenuation() const
    {
        return SoundObjectImpl::GetAttenuation();
    }

    Bus SoundImpl::GetBus() const
    {
        return SoundObjectImpl::GetBus();
    }

    bool SoundImpl::LoadDefinition(const SoundDefinition* definition, EngineInternalState* state)
    {
        if (definition->id() == kAmInvalidObjectId)
        {
            amLogError("Sound definition is invalid: no ID defined.");
            return false;
        }

        if (definition->bus() == kAmInvalidObjectId)
        {
            amLogError("Sound definition is invalid: no bus ID defined.");
            return false;
        }

        m_bus = FindBusInternalState(state, definition->bus());
        if (!m_bus)
        {
            amLogError("Sound %s specifies an unknown bus ID: " AM_ID_CHAR_FMT ".", definition->name()->c_str(), definition->bus());
            return false;
        }

        if (definition->effect() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->effect_map.find(definition->effect()); findIt != state->effect_map.end())
            {
                m_effect = findIt->second.get();
            }
            else
            {
                amLogError("Sound definition is invalid: invalid effect ID '" AM_ID_CHAR_FMT "'", definition->effect());
                return false;
            }
        }

        if (definition->attenuation() != kAmInvalidObjectId)
        {
            if (const auto findIt = state->attenuation_map.find(definition->attenuation()); findIt != state->attenuation_map.end())
            {
                m_attenuation = findIt->second.get();
            }
            else
            {
                amLogError("Sound definition is invalid: invalid attenuation ID '" AM_ID_CHAR_FMT "'", definition->attenuation());
                return false;
            }
        }

        m_id = definition->id();
        m_name = definition->name()->str();

        auto* fs = amEngine->GetFileSystem();

        _stream = definition->stream();
        _loop = definition->loop() != nullptr && definition->loop()->enabled();
        _loopCount = definition->loop() ? definition->loop()->loop_count() : 0;
        m_filename = fs->ResolvePath(fs->Join({ AM_OS_STRING("data"), AM_STRING_TO_OS_STRING(definition->path()->str()) }));

        RtpcValue::Init(m_gain, definition->gain(), 1);
        RtpcValue::Init(_nearFieldGain, definition->near_field_gain(), 1);
        RtpcValue::Init(m_pitch, definition->pitch(), 1);
        RtpcValue::Init(m_priority, definition->priority(), 1);

        _settings.m_id = m_id;
        _settings.m_kind = SoundKind::Standalone;
        _settings.m_busID = definition->bus();
        _settings.m_effectID = definition->effect();
        _settings.m_attenuationID = definition->attenuation();
        _settings.m_spatialization = definition->spatialization();
        _settings.m_priority = RtpcValue(m_priority);
        _settings.m_gain = RtpcValue(m_gain);
        _settings.m_nearFieldGain = RtpcValue(_nearFieldGain);
        _settings.m_pitch = RtpcValue(m_pitch);
        _settings.m_loop = _loop;
        _settings.m_loopCount = _loopCount;

        return true;
    }

    const SoundDefinition* SoundImpl::GetDefinition() const
    {
        return GetSoundDefinition(m_source.c_str());
    }

    void SoundImpl::AcquireReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        if (m_effect)
            m_effect->GetRefCounter()->Increment();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Increment();
    }

    void SoundImpl::ReleaseReferences(EngineInternalState* state)
    {
        AMPLITUDE_ASSERT(m_id != kAmInvalidObjectId);

        if (m_effect)
            m_effect->GetRefCounter()->Decrement();

        if (m_attenuation)
            m_attenuation->GetRefCounter()->Decrement();
    }
    const AmOsString& SoundImpl::GetPath() const
    {
        return ResourceImpl::GetPath();
    }

    SoundInstance::SoundInstance(SoundImpl* parent, SoundInstanceSettings settings, const EffectImpl* effect)
        : _userData(nullptr)
        , _channel(nullptr)
        , _parent(parent)
        , _collection(nullptr)
        , _effect(effect)
        , _effectInstance(nullptr)
        , _decoder(nullptr)
        , _settings(std::move(settings))
        , _currentLoopCount(0)
        , _id(++gLastSoundInstanceID)
    {
        if (_effect != nullptr)
            _effectInstance = _effect->CreateInstance();

        if (_parent->_stream)
        {
            const auto filename = _parent->GetPath();
            const auto file = amEngine->GetFileSystem()->OpenFile(filename);

            _decoder = _parent->_codec->CreateDecoder();
            if (!_decoder->Open(file))
            {
                amLogError("Cannot load the sound: unable to initialize a decoder for '" AM_OS_CHAR_FMT "'.", filename.c_str());
                return;
            }
        }
    }

    SoundInstance::~SoundInstance()
    {
        Destroy();
    }

    void SoundInstance::Load()
    {
        AMPLITUDE_ASSERT(Valid());

        const AmUInt16 channels = _parent->_format.GetNumChannels();
        const AmUInt64 frames = _parent->_format.GetFramesCount();

        SoundData* data;
        SoundChunk* chunk;

        if (_parent->_stream)
        {
            chunk = SoundChunk::CreateChunk(amEngine->GetSamplesPerStream(), channels);
            data = SoundData::CreateMusic(_parent->_format, chunk, frames, this);
        }
        else
        {
            chunk = _parent->AcquireSoundData();
            data = SoundData::CreateSound(_parent->_format, chunk, frames, this);
        }

        if (data == nullptr)
        {
            amLogError("Could not load a sound instance. Unable to read data from the parent sound.");

            if (chunk != nullptr)
            {
                if (_parent->_stream)
                    SoundChunk::DestroyChunk(chunk);
                else
                    _parent->ReleaseSoundData();
            }

            return;
        }

        SetUserData(data);
    }

    const SoundInstanceSettings& SoundInstance::GetSettings() const
    {
        return _settings;
    }

    AmVoidPtr SoundInstance::GetUserData() const
    {
        return _userData;
    }

    void SoundInstance::SetUserData(AmVoidPtr userData)
    {
        _userData = userData;
    }

    AmUInt64 SoundInstance::GetAudio(AmUInt64 offset, AmUInt64 frames) const
    {
        AMPLITUDE_ASSERT(Valid());

        if (!_parent->_stream || _userData == nullptr)
            return 0;

        const auto* data = static_cast<SoundData*>(_userData);

        const AmUInt16 channels = _parent->_format.GetNumChannels();

        AmUInt64 l = frames, o = offset, r = 0, s = 0;
        AudioBuffer* b = data->chunk->buffer;

        bool needFill = true;
        do
        {
            const AmUInt64 n = _decoder->Stream(b, s, o, l);
            r += n;

            // If we reached the end of the file but looping is enabled, then
            // seek back to the beginning of the file and fill the remaining part of the buffer.
            if (needFill = n < l && _parent->_loop && _decoder->Seek(0); needFill)
            {
                s += n;
                l -= n;
                o = 0;
            }
        } while (needFill);

        return r;
    }

    void SoundInstance::Destroy()
    {
        AMPLITUDE_ASSERT(Valid());

        if (_userData != nullptr)
        {
            SoundData::Destroy(static_cast<SoundData*>(_userData), _parent->_stream);

            if (!_parent->_stream)
                _parent->ReleaseSoundData();
        }

        _userData = nullptr;

        _effect->DestroyInstance(_effectInstance);
        _effectInstance = nullptr;

        if (_parent->_stream)
        {
            _decoder->Close();
            _parent->_codec->DestroyDecoder(_decoder);
        }

        _decoder = nullptr;

        _parent = nullptr;
    }

    bool SoundInstance::Valid() const
    {
        return _parent != nullptr;
    }

    Channel SoundInstance::GetChannel() const
    {
        return _channel != nullptr ? Channel(_channel->GetParentChannelState()) : Channel(nullptr);
    }

    void SoundInstance::SetChannel(RealChannel* channel)
    {
        _channel = channel;
    }

    RealChannel* SoundInstance::GetRealChannel() const
    {
        return _channel;
    }

    const Sound* SoundInstance::GetSound() const
    {
        return _parent;
    }

    const CollectionImpl* SoundInstance::GetCollection() const
    {
        return _collection;
    }

    AmUInt32 SoundInstance::GetCurrentLoopCount() const
    {
        return _currentLoopCount;
    }

    const EffectInstance* SoundInstance::GetEffect() const
    {
        return _effectInstance;
    }

    AmObjectID SoundInstance::GetId() const
    {
        return _id;
    }
} // namespace SparkyStudios::Audio::Amplitude

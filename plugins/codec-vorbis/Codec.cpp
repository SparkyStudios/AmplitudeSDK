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

#include <Codec.h>

AM_API_PRIVATE void vorbis_interleave(float* dest, const float* const* src, unsigned nframes, unsigned channels)
{
    for (const float* const* src_end = src + channels; src != src_end; ++src, ++dest)
    {
        float* d = dest;
        for (const float *s = *src, *s_end = s + nframes; s != s_end; ++s, d += channels)
            *d = *s;
    }
}

AM_API_PRIVATE size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto* file = static_cast<File*>(userdata);
    return file->Read(static_cast<AmUInt8Buffer>(ptr), nmemb);
}

AM_API_PRIVATE int seek_callback(void* userdata, ogg_int64_t offset, int whence)
{
    auto* file = static_cast<File*>(userdata);
    file->Seek(offset, static_cast<FileSeekOrigin>(whence));
    return 0;
}

AM_API_PRIVATE long tell_callback(void* userdata)
{
    auto* file = static_cast<File*>(userdata);
    return file->Position();
}

AM_API_PRIVATE ov_callbacks OV_CALLBACKS = { read_callback, seek_callback, nullptr, tell_callback };

bool VorbisCodec::VorbisDecoder::Open(std::shared_ptr<File> file)
{
    if (!m_codec->CanHandleFile(file))
    {
        CallLogFunc("The Vorbis codec cannot handle the file: '" AM_OS_CHAR_FMT "'.\n", file->GetPath().c_str());
        return false;
    }

    _file = file;

    if (ov_open_callbacks(_file.get(), &_vorbis, nullptr, 0, OV_CALLBACKS) < 0)
    {
        _file.reset();
        CallLogFunc("Unable to open the file: '" AM_OS_CHAR_FMT "'.\n", file->GetPath().c_str());
        return false;
    }

    const vorbis_info* info = ov_info(&_vorbis, -1);
    const AmUInt32 framesCount = ov_pcm_total(&_vorbis, -1);

    m_format.SetAll(
        info->rate, info->channels, 16, framesCount, info->channels * sizeof(AmAudioSample),
        AM_SAMPLE_FORMAT_FLOAT
    );

    _initialized = true;

    return true;
}

bool VorbisCodec::VorbisDecoder::Close()
{
    if (_initialized)
    {
        _file.reset();

        m_format = SoundFormat();
        _initialized = false;
        ov_clear(&_vorbis);

        return true;
    }

    // true because it is already closed
    return true;
}

AmUInt64 VorbisCodec::VorbisDecoder::Load(AmVoidPtr out)
{
    return Stream(out, 0, m_format.GetFramesCount());
}

AmUInt64 VorbisCodec::VorbisDecoder::Stream(AmVoidPtr out, AmUInt64 offset, AmUInt64 length)
{
    if (!_initialized)
        return 0;

    if (!Seek(offset))
        return 0;

    const AmUInt16 channels = m_format.GetNumChannels();

    AmUInt64 size = length;
    AmUInt64 read = 0;

    float** data;

    while (size > 0)
    {
        _current_section = 0;
        const AmInt64 ret = ov_read_float(&_vorbis, &data, static_cast<AmInt32>(size), &_current_section);

        if (ret == 0)
            break;

        if (ret > 0)
        {
            vorbis_interleave(static_cast<AmAudioSampleBuffer>(out) + read * channels, data, ret, channels);

            size -= ret;
            read += ret;
        }
        else
        {
            if (ret == OV_EBADLINK)
            {
                CallLogFunc("Corrupt bitstream section!.\n");
                return 0;
            }

            if (ret == OV_EINVAL)
            {
                CallLogFunc("Invalid bitstream section!.\n");
                return 0;
            }
        }
    }

    return read;
}

bool VorbisCodec::VorbisDecoder::Seek(AmUInt64 offset)
{
    return ov_pcm_seek(&_vorbis, offset) >= 0;
}

bool VorbisCodec::VorbisEncoder::Open(std::shared_ptr<File> file)
{
    _initialized = true;
    return false;
}

bool VorbisCodec::VorbisEncoder::Close()
{
    if (_initialized)
        return true;

    return true;
}

AmUInt64 VorbisCodec::VorbisEncoder::Write(AmVoidPtr in, AmUInt64 offset, AmUInt64 length)
{
    return 0;
}

Codec::Decoder* VorbisCodec::CreateDecoder()
{
    return ampoolnew(MemoryPoolKind::Codec, VorbisDecoder, this);
}

void VorbisCodec::DestroyDecoder(Decoder* decoder)
{
    ampooldelete(MemoryPoolKind::Codec, VorbisDecoder, (VorbisDecoder*)decoder);
}

Codec::Encoder* VorbisCodec::CreateEncoder()
{
    return ampoolnew(MemoryPoolKind::Codec, VorbisEncoder, this);
}

void VorbisCodec::DestroyEncoder(Encoder* encoder)
{
    ampooldelete(MemoryPoolKind::Codec, VorbisEncoder, (VorbisEncoder*)encoder);
}

bool VorbisCodec::CanHandleFile(std::shared_ptr<File> file) const
{
    const auto& path = file->GetPath();
    return path.find(AM_OS_STRING(".ogg")) != AmOsString::npos; // OGG/Vorbis extension
}
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

#include <Core/Codecs/AMS/Codec.h>

using namespace SparkyStudios::Audio::Amplitude::Compression::ADPCM;

// This runtime macro is not strictly needed because the code is endian-safe,
// but including it improves performance on little-endian systems because we
// can avoid a couple loops through the audio.
#define IS_BIG_ENDIAN (*(uint16_t*)"\0\xff" < 0x0100)

namespace SparkyStudios::Audio::Amplitude
{
    static void little_endian_to_native(void* data, const char* format)
    {
        char* i = const_cast<char*>(format);
        auto* cp = (unsigned char*)data;
        AmInt32 temp;

        while (*i)
        {
            switch (*i)
            {
            case 'L':
                temp = cp[0] + ((AmInt32)cp[1] << 8) + ((AmInt32)cp[2] << 16) + ((AmInt32)cp[3] << 24);
                *(AmInt32*)cp = temp;
                cp += 4;
                break;

            case 'S':
                temp = cp[0] + (cp[1] << 8);
                *(short*)cp = (short)temp;
                cp += 2;
                break;

            default:
                if (isdigit((unsigned char)*i))
                    cp += *i - '0';

                break;
            }

            i += 1;
        }
    }

    static void native_to_little_endian(void* data, const char* format)
    {
        char* i = const_cast<char*>(format);
        auto* cp = (unsigned char*)data;
        AmInt32 temp;

        while (*i)
        {
            switch (*i)
            {
            case 'L':
                temp = *(AmInt32*)cp;
                *cp++ = (unsigned char)temp;
                *cp++ = (unsigned char)(temp >> 8);
                *cp++ = (unsigned char)(temp >> 16);
                *cp++ = (unsigned char)(temp >> 24);
                break;

            case 'S':
                temp = *(short*)cp;
                *cp++ = (unsigned char)temp;
                *cp++ = (unsigned char)(temp >> 8);
                break;

            default:
                if (isdigit((unsigned char)*i))
                    cp += *i - '0';

                break;
            }

            i += 1;
        }
    }

    static bool ReadHeader(std::shared_ptr<File> file, SoundFormat& format, AmUInt16& blockSize)
    {
        AmInt32 fmt = 0, res = 0, bits_per_sample, sample_rate, num_channels;
        AmUInt32 fact_samples = 0;
        AmSize num_samples = 0;

        RIFFHeader riff_chunk_header;
        FMTHeader chunk_header;
        WAVEHeaderEx wave_header;

        // read initial RIFF form header

        if (file->Read((AmUInt8Buffer)&riff_chunk_header, sizeof(RIFFHeader)) != sizeof(RIFFHeader) ||
            strncmp((char*)riff_chunk_header.chunkID, "RIFF", 4) != 0 || strncmp((char*)riff_chunk_header.chunkFormat, "WAVE", 4) != 0)
        {
            return false;
        }

        // loop through all elements of the RIFF wav header (until the data chuck)

        while (true)
        {
            if (file->Read((AmUInt8Buffer)&chunk_header, sizeof(FMTHeader)) != sizeof(FMTHeader))
            {
                return false;
            }

            little_endian_to_native(&chunk_header, HEADER_FMT_CHUNK);

            // if it's the format chunk, we want to get some info out of there and
            // make sure it's a .wav file we can handle

            if (!strncmp((char*)chunk_header.chunkID, "fmt ", 4))
            {
                bool supported = true;

                if (chunk_header.chunkSize < 16 || chunk_header.chunkSize > sizeof(WAVEHeaderEx) ||
                    file->Read((AmUInt8Buffer)&wave_header, chunk_header.chunkSize) != chunk_header.chunkSize)
                {
                    return false;
                }

                little_endian_to_native(&wave_header, HEADER_FMT_WAVE);

                fmt = (wave_header.head.audioFormat == WAVE_FORMAT_EXTENSIBLE && chunk_header.chunkSize == 40)
                    ? wave_header.subFormat
                    : wave_header.head.audioFormat;

                bits_per_sample = (chunk_header.chunkSize == 40 && wave_header.head.validBitsPerSample)
                    ? wave_header.head.validBitsPerSample
                    : wave_header.head.bitsPerSample;

                if (wave_header.head.numChannels < 1 || wave_header.head.numChannels > 2)
                {
                    // Only support up to 2 channels
                    supported = false;
                }
                else if (fmt == WAVE_FORMAT_PCM)
                {
                    // Only support ADPCM
                    supported = false;
                }
                else if (fmt == WAVE_FORMAT_IMA_ADPCM)
                {
                    if (bits_per_sample != 4)
                        supported = false; // Invalid ADPCM format

                    if (wave_header.head.validBitsPerSample !=
                        (wave_header.head.blockAlign - wave_header.head.numChannels * 4) * (wave_header.head.numChannels ^ 3) + 1)
                    {
                        return false;
                    }
                }
                else
                {
                    // Unknown format
                    supported = false;
                }

                if (!supported)
                {
                    return false;
                }
            }
            else if (!strncmp((char*)chunk_header.chunkID, "fact", 4))
            {
                if (chunk_header.chunkSize < 4 || file->Read((AmUInt8Buffer)&fact_samples, sizeof(fact_samples)) != sizeof(fact_samples))
                {
                    return false;
                }

                if (chunk_header.chunkSize > 4)
                {
                    int bytes_to_skip = chunk_header.chunkSize - 4;
                    char dummy;

                    while (bytes_to_skip--)
                    {
                        if (!file->Read((AmUInt8Buffer)&dummy, 1))
                        {
                            return false;
                        }
                    }
                }
            }
            else if (!strncmp((char*)chunk_header.chunkID, "data", 4))
            {
                // on the data chunk, get size and exit parsing loop

                if (!wave_header.head.numChannels)
                { // make sure we saw a "fmt" chunk...
                    return false;
                }

                if (!chunk_header.chunkSize)
                {
                    return false;
                }

                if (fmt == WAVE_FORMAT_PCM)
                {
                    if (chunk_header.chunkSize % wave_header.head.blockAlign)
                    {
                        return false;
                    }

                    num_samples = chunk_header.chunkSize / wave_header.head.blockAlign;
                }
                else
                {
                    int complete_blocks = chunk_header.chunkSize / wave_header.head.blockAlign;
                    int leftover_bytes = chunk_header.chunkSize % wave_header.head.blockAlign;
                    int samples_last_block;

                    num_samples = complete_blocks * wave_header.head.validBitsPerSample;

                    if (leftover_bytes)
                    {
                        if (leftover_bytes % (wave_header.head.numChannels * 4))
                        {
                            return false;
                        }

                        samples_last_block = (leftover_bytes - (wave_header.head.numChannels * 4)) * (wave_header.head.numChannels ^ 3) + 1;
                        num_samples += samples_last_block;
                    }
                    else
                    {
                        samples_last_block = wave_header.head.validBitsPerSample;
                    }

                    if (fact_samples)
                    {
                        if (fact_samples < num_samples && fact_samples > num_samples - samples_last_block)
                        {
                            num_samples = fact_samples;
                        }
                        else if (
                            wave_header.head.numChannels == 2 && (fact_samples >>= 1) < num_samples &&
                            fact_samples > num_samples - samples_last_block)
                        {
                            num_samples = fact_samples;
                        }
                    }
                }

                if (!num_samples)
                {
                    return false;
                }

                num_channels = wave_header.head.numChannels;
                sample_rate = wave_header.head.sampleRate;
                break;
            }
            else
            { // just ignore unknown chunks
                int bytes_to_eat = (chunk_header.chunkSize + 1) & ~1L;
                char dummy;

                while (bytes_to_eat--)
                {
                    if (!file->Read((AmUInt8Buffer)&dummy, 1))
                    {
                        return false;
                    }
                }
            }
        }

        format.SetAll(sample_rate, num_channels, bits_per_sample, num_samples, num_channels * sizeof(AmInt16), eAudioSampleFormat_Int16);

        blockSize = wave_header.head.blockAlign;

        return true;
    }

    static bool WriteHeader(std::shared_ptr<File> file, SoundFormat& format, AmUInt32 samplesPerBlock)
    {
        ADPCMHeader header;

        AmInt32 blockSize = (samplesPerBlock - 1) / (format.GetNumChannels() ^ 3) + (format.GetNumChannels() * 4);
        AmSize numBlocks = format.GetFramesCount() / samplesPerBlock;
        AmInt32 leftOverSamples = format.GetFramesCount() % samplesPerBlock;
        AmSize totalDataBytes = numBlocks * blockSize;

        if (leftOverSamples)
        {
            AmInt32 lastBlockSamples = ((leftOverSamples + 6) & ~7) + 1;
            AmInt32 lastBlockSize = (lastBlockSamples - 1) / (format.GetNumChannels() ^ 3) + (format.GetNumChannels() * 4);
            totalDataBytes += lastBlockSize;
        }

        memset(&header, 0, sizeof(header));

        // ========== RIFF HEADER
#if defined(AM_WINDOWS_VERSION)
        strncpy_s((char*)header.riff.chunkID, 4, "RIFF", sizeof(header.riff.chunkID));
#else
        strncpy((char*)header.riff.chunkID, "RIFF", sizeof(header.riff.chunkID));
#endif
        header.riff.chunkSize = sizeof(RIFFHeader) + sizeof(WAVEHeader) + sizeof(DATAHeader) + totalDataBytes;

#if defined(AM_WINDOWS_VERSION)
        strncpy_s((char*)header.riff.chunkFormat, 4, "WAVE", sizeof(header.riff.chunkFormat));
#else
        strncpy((char*)header.riff.chunkFormat, "WAVE", sizeof(header.riff.chunkFormat));
#endif

        // ========== FORMAT HEADER
#if defined(AM_WINDOWS_VERSION)
        strncpy_s((char*)header.fmt.chunkID, 4, "fmt ", sizeof(header.fmt.chunkID));
#else
        strncpy((char*)header.fmt.chunkID, "fmt ", sizeof(header.fmt.chunkID));
#endif

        header.fmt.chunkSize = sizeof(WAVEHeader);

        // ========== WAVE HEADER
        header.wave.audioFormat = WAVE_FORMAT_IMA_ADPCM;
        header.wave.numChannels = format.GetNumChannels();
        header.wave.sampleRate = format.GetSampleRate();
        header.wave.byteRate = format.GetSampleRate() * blockSize / samplesPerBlock;
        header.wave.blockAlign = blockSize;
        header.wave.bitsPerSample = 4; // <- 4 for ADPCM
        header.wave.extendedSize = 2;
        header.wave.validBitsPerSample = samplesPerBlock;

        // ========== FACT HEADER
#if defined(AM_WINDOWS_VERSION)
        strncpy_s((char*)header.fact.chunkID, 4, "fact", sizeof(header.fact.chunkID));
#else
        strncpy((char*)header.fact.chunkID, "fact", sizeof(header.fact.chunkID));
#endif

        header.fact.totalSamples = format.GetFramesCount();
        header.fact.chunkSize = 4;

        // ========== DATA HEADER
#if defined(AM_WINDOWS_VERSION)
        strncpy_s((char*)header.data.chunkID, 4, "data", sizeof(header.data.chunkID));
#else
        strncpy((char*)header.data.chunkID, "data", sizeof(header.data.chunkID));
#endif

        header.data.chunkSize = totalDataBytes;

        // write the RIFF chunks up to just before the data starts

        native_to_little_endian(&header.riff, HEADER_FMT_CHUNK);
        native_to_little_endian(&header.fmt, HEADER_FMT_CHUNK);
        native_to_little_endian(&header.wave, HEADER_FMT_WAVE);
        native_to_little_endian(&header.fact, HEADER_FMT_FACT);
        native_to_little_endian(&header.data, HEADER_FMT_CHUNK);

        return file->Write((AmConstUInt8Buffer)&header, sizeof(header));
    }

    static AmUInt64 Decode(
        std::shared_ptr<File> file, const SoundFormat& format, AudioBuffer* out, AmUInt64 offset, AmUInt64 length, AmUInt32 blockSize)
    {
        const AmUInt32 numChannels = format.GetNumChannels();
        const AmUInt32 frameSize = format.GetFrameSize();
        const AmUInt32 samplesPerBlock = (blockSize - numChannels * 4) * (numChannels ^ 3) + 1;

        auto pcm_block = static_cast<AmInt16Buffer>(ampoolmalloc(eMemoryPoolKind_Codec, samplesPerBlock * frameSize));
        auto adpcm_block = static_cast<AmUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_Codec, blockSize));

        if (!pcm_block || !adpcm_block)
        {
            return 0;
        }

        auto* output16 = static_cast<AmInt16Buffer>(ampoolmalloc(eMemoryPoolKind_Codec, length * numChannels * sizeof(AmInt16)));

        if (!output16)
        {
            ampoolfree(eMemoryPoolKind_Codec, pcm_block);
            ampoolfree(eMemoryPoolKind_Codec, adpcm_block);
            return 0;
        }

        if (file->Read(adpcm_block, blockSize) != blockSize)
        {
            ampoolfree(eMemoryPoolKind_Codec, pcm_block);
            ampoolfree(eMemoryPoolKind_Codec, adpcm_block);
            return 0;
        }

        if (Decompress(pcm_block, adpcm_block, blockSize, numChannels) != samplesPerBlock)
        {
            ampoolfree(eMemoryPoolKind_Codec, pcm_block);
            ampoolfree(eMemoryPoolKind_Codec, adpcm_block);
            return 0;
        }

        const AmUInt64 oo = (offset % blockSize) * numChannels;
        std::memcpy(output16, pcm_block + oo, length * frameSize);

        for (AmUInt16 c = 0; c < numChannels; c++)
        {
            auto& channel = out->GetChannel(c);
            for (AmUInt64 i = 0; i < length; i++)
                channel[i] = AmInt16ToReal32(output16[i * numChannels + c]);
        }

        ampoolfree(eMemoryPoolKind_Codec, output16);
        ampoolfree(eMemoryPoolKind_Codec, pcm_block);
        ampoolfree(eMemoryPoolKind_Codec, adpcm_block);

        return length;
    }

    static AmUInt64 Encode(
        std::shared_ptr<File> file,
        SoundFormat& format,
        AudioBuffer* in,
        AmUInt64 length,
        AmUInt32 samplesPerBlock,
        int lookAhead,
        NoiseShapingMode noiseShaping)
    {
        const AmUInt32 numChannels = format.GetNumChannels();
        AmUInt32 blockSize = (samplesPerBlock - 1) / (numChannels ^ 3) + (numChannels * 4);

        auto adpcm_block = static_cast<AmUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_Codec, blockSize));

        Context* ctx = nullptr;

        if (!adpcm_block)
            return 0;

        auto* input16 = static_cast<AmInt16Buffer>(ampoolmalloc(eMemoryPoolKind_Codec, length * numChannels * sizeof(AmInt16)));

        if (!input16)
            return 0;

        for (AmUInt16 c = 0; c < numChannels; c++)
        {
            const auto& channel = in->GetChannel(c);
            for (AmUInt64 i = 0; i < length; i++)
                input16[i * numChannels + c] = AmReal32ToInt16(channel[i], true);
        }

        AmUInt64 offset = 0;
        while (length)
        {
            AmUInt32 this_block_adpcm_samples = samplesPerBlock;
            AmUInt32 this_block_pcm_samples = samplesPerBlock;
            AmSize num_bytes;

            if (this_block_pcm_samples > length)
            {
                this_block_adpcm_samples = ((length + 6) & ~7) + 1;
                blockSize = (this_block_adpcm_samples - 1) / (numChannels ^ 3) + (numChannels * 4);
                this_block_pcm_samples = length;
            }

            AmInt16Buffer pcm_block = input16 + offset * numChannels;

            if (IS_BIG_ENDIAN)
            {
                AmUInt32 count = this_block_pcm_samples * numChannels;
                auto* cp = reinterpret_cast<unsigned char*>(pcm_block);

                while (count--)
                {
                    const int16_t temp = cp[0] + (cp[1] << 8);
                    *reinterpret_cast<int16_t*>(cp) = temp;
                    cp += 2;
                }
            }

            // if this is the last block, and it's not full, duplicate the last sample(s) so we don't
            // create problems for the lookAhead

            if (this_block_adpcm_samples > this_block_pcm_samples)
            {
                AmInt16 *dst = pcm_block + this_block_pcm_samples * numChannels, *src = dst - numChannels;
                int dups = (this_block_adpcm_samples - this_block_pcm_samples) * numChannels;

                while (dups--)
                {
                    *dst++ = *src++;
                }
            }

            // if this is the first block, compute a decaying average (in reverse) so that we can let the
            // encoder know what kind of initial deltas to expect (helps to initialize index)

            if (!ctx)
            {
                AmInt32 average_deltas[2];

                average_deltas[0] = average_deltas[1] = 0;

                for (AmUInt32 i = this_block_adpcm_samples * numChannels; i -= numChannels;)
                {
                    average_deltas[0] -= average_deltas[0] >> 3;
                    average_deltas[0] += std::abs((AmInt32)pcm_block[i] - pcm_block[i - numChannels]);

                    if (numChannels == 2)
                    {
                        average_deltas[1] -= average_deltas[1] >> 3;
                        average_deltas[1] += std::abs((AmInt32)pcm_block[i - 1] - pcm_block[i + 1]);
                    }
                }

                average_deltas[0] >>= 3;
                average_deltas[1] >>= 3;

                ctx = CreateContext(numChannels, lookAhead, noiseShaping, average_deltas);
            }

            Compress(ctx, adpcm_block, num_bytes, pcm_block, this_block_adpcm_samples);

            if (num_bytes != blockSize)
            {
                ampoolfree(eMemoryPoolKind_Codec, adpcm_block);
                return 0;
            }

            if (file->Write(adpcm_block, blockSize) != blockSize)
            {
                ampoolfree(eMemoryPoolKind_Codec, adpcm_block);
                return 0;
            }

            length -= this_block_pcm_samples;
            offset += this_block_pcm_samples;
        }

        if (ctx)
        {
            FreeContext(ctx);
        }

        ampoolfree(eMemoryPoolKind_Codec, input16);
        ampoolfree(eMemoryPoolKind_Codec, adpcm_block);

        return offset;
    }

    AMSCodec::AMSCodec()
        : Codec("ams")
    {}

    bool AMSCodec::AMSDecoder::Open(std::shared_ptr<File> file)
    {
        _file = file;

        if (!ReadHeader(_file, m_format, _blockSize))
        {
            amLogError("The AMS codec cannot handle the file: '" AM_OS_CHAR_FMT "'", file->GetPath().c_str());
            return false;
        }

        _initialized = true;

        return true;
    }

    bool AMSCodec::AMSDecoder::Close()
    {
        if (_initialized)
        {
            _file.reset();

            m_format = SoundFormat();
            _initialized = false;
        }

        // true because it is already closed
        return true;
    }

    AmUInt64 AMSCodec::AMSDecoder::Load(AudioBuffer* out)
    {
        if (!_initialized)
            return 0;

        if (!Seek(0))
            return 0;

        return Decode(_file, m_format, out, 0, m_format.GetFramesCount(), _blockSize);
    }

    AmUInt64 AMSCodec::AMSDecoder::Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length)
    {
        if (!_initialized)
            return 0;

        if (!Seek(seekOffset))
            return 0;

        return Decode(_file, m_format, out, seekOffset, length, _blockSize);
    }

    bool AMSCodec::AMSDecoder::Seek(AmUInt64 offset)
    {
        const AmUInt32 numChannels = m_format.GetNumChannels();
        const AmUInt32 samplesPerBlock = (_blockSize - numChannels * 4) * (numChannels ^ 3) + 1;
        const AmUInt32 steps = offset / samplesPerBlock;

        offset = steps * _blockSize;
        _file->Seek(sizeof(ADPCMHeader) + offset, eFileSeekOrigin_Start);

        return true;
    }

    bool AMSCodec::AMSEncoder::Open(std::shared_ptr<File> file)
    {
        _file = file;

        if (!WriteHeader(_file, m_format, _samplesPerBlock))
        {
            amLogError("The AMS codec was unable to write the file: '" AM_OS_CHAR_FMT "'", file->GetPath().c_str());
            return false;
        }

        _initialized = true;
        return true;
    }

    bool AMSCodec::AMSEncoder::Close()
    {
        if (_initialized)
        {
            _file.reset();

            m_format = SoundFormat();
            _initialized = false;
        }

        return true;
    }

    AmUInt64 AMSCodec::AMSEncoder::Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length)
    {
        _file->Seek(sizeof(ADPCMHeader) + offset, eFileSeekOrigin_Start);
        return Encode(_file, m_format, in, length, _samplesPerBlock, _lookAhead, _noiseShaping);
    }

    void AMSCodec::AMSEncoder::SetEncodingParams(
        AmUInt32 blockSize, AmUInt32 samplesPerBlock, AmUInt32 lookAhead, NoiseShapingMode noiseShaping)
    {
        _blockSize = blockSize;
        _samplesPerBlock = samplesPerBlock;
        _lookAhead = lookAhead;
        _noiseShaping = noiseShaping;
    }

    Codec::Decoder* AMSCodec::CreateDecoder()
    {
        return ampoolnew(eMemoryPoolKind_Codec, AMSDecoder, this);
    }

    void AMSCodec::DestroyDecoder(Decoder* decoder)
    {
        ampooldelete(eMemoryPoolKind_Codec, AMSDecoder, (AMSDecoder*)decoder);
    }

    Codec::Encoder* AMSCodec::CreateEncoder()
    {
        return ampoolnew(eMemoryPoolKind_Codec, AMSEncoder, this);
    }

    void AMSCodec::DestroyEncoder(Encoder* encoder)
    {
        ampooldelete(eMemoryPoolKind_Codec, AMSEncoder, (AMSEncoder*)encoder);
    }

    bool AMSCodec::CanHandleFile(std::shared_ptr<File> file) const
    {
        const auto& path = file->GetPath();
        return path.find(AM_OS_STRING(".ams")) != AmOsString::npos;
    }
} // namespace SparkyStudios::Audio::Amplitude

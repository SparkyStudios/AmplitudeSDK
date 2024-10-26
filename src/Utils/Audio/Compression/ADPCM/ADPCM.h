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
//
// Based on ADPCM-XQ, Copyright (c) 2015 David Bryant.
// https://github.com/dbry/adpcm-xq

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_COMPRESSION_ADPCM_H
#define SS_AMPLITUDE_AUDIO_COMPRESSION_ADPCM_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 0x1
#endif

#ifndef WAVE_FORMAT_IMA_ADPCM
#define WAVE_FORMAT_IMA_ADPCM 0x11
#endif

#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE 0xfffe
#endif

#define HEADER_FMT_CHUNK "4L"
#define HEADER_FMT_FACT "4LL"
#define HEADER_FMT_WAVE "SSLLSSSSLS"

namespace SparkyStudios::Audio::Amplitude::Compression::ADPCM
{
    /**
     * @brief RIFF part of the ADPCM header.
     */
    struct RIFFHeader
    {
        AmUInt8 chunkID[4] = { 'R', 'I', 'F', 'F' }; // Contains the letters "RIFF" in ASCII form
        AmUInt32 chunkSize;
        AmUInt8 chunkFormat[4] = { 'W', 'A', 'V', 'E' }; // Contains the letters "WAVE" in ASCII form
    };

    /**
     * @brief FMT part of the ADPCM header.
     */
    struct FMTHeader
    {
        AmUInt8 chunkID[4] = { 'f', 'm', 't', ' ' }; // Contains the letters "fmt " in ASCII form
        AmUInt32 chunkSize = 16;
    };

    /**
     * @brief WAVE part of the ADPCM header.
     */
    struct WAVEHeader
    {
        AmUInt16 audioFormat;
        AmUInt16 numChannels;
        AmUInt32 sampleRate;
        AmUInt32 byteRate;
        AmUInt16 blockAlign;
        AmUInt16 bitsPerSample;
        AmUInt16 extendedSize;
        AmUInt16 validBitsPerSample;
    };

    /**
     * @brief Extended WAVE header. Not used in the ADPCM header.
     * Only there for decoding.
     */
    struct WAVEHeaderEx
    {
        WAVEHeader head;
        AmUInt32 channelMask;
        AmUInt16 subFormat;
        AmUInt8 guid[14];
    };

    /**
     * @brief FACT part of the ADPCM header.
     */
    struct FACTHeader
    {
        AmUInt8 chunkID[4] = { 'f', 'a', 'c', 't' }; // Contains the letters "fact" in ASCII form
        AmUInt32 chunkSize;
        AmUInt32 totalSamples;
    };

    /**
     * @brief DATA part of the ADPCM header.
     */
    struct DATAHeader
    {
        AmUInt8 chunkID[4] = { 'd', 'a', 't', 'a' }; // Contains the letters "data" in ASCII form
        AmUInt32 chunkSize; // This is the number of bytes in the data
    };

    /**
     * @brief ADPCM header.
     */
    struct ADPCMHeader
    {
        // ========== RIFF HEADER
        RIFFHeader riff;

        // ========== FORMAT HEADER
        FMTHeader fmt;

        // ========== WAVE HEADER
        WAVEHeader wave;

        // ========== FACT HEADER
        FACTHeader fact;

        // ========== DATA HEADER
        DATAHeader data;
    };

    struct Channel
    {
        AmInt32 pcmData; // current PCM value
        AmInt32 error, weight, history[2]; // for noise shaping
        AmInt8 index; // current index into step size table
    };

    struct Context
    {
        Channel channels[2];
        int numChannels, lookAhead, noiseShaping;
    };

    enum NoiseShapingMode
    {
        eNSM_OFF = 0,
        eNSM_STATIC = 1,
        eNSM_DYNAMIC = 2,
    };

    /**
     * @brief The returned pointer is used for subsequent calls. Note that
     * even though an ADPCM encoder could be set up to encode frames
     * independently, we use a context so that we can use previous
     * data to improve quality (this encoder might not be optimal
     * for encoding independent frames).
     *
     * @param numChannels The number of channels in the audio to compress.
     * @param lookAhead Look ahead mode.
     * @param noiseShaping Noise shaping mode.
     * @param initialDeltas Initial deltas for compression.
     */
    Context* CreateContext(int numChannels, int lookAhead, NoiseShapingMode noiseShaping, AmInt32 initialDeltas[2]);

    /**
     * @brief Frees an ADPCM encoding context.
     *
     * @param context The context to free.
     */
    void FreeContext(Context* context);

    /**
     * @brief Compresses a block of 16-bit PCM data into 4-bit ADPCM.
     *
     * @param p The compression context.
     * @param out The destination buffer.
     * @param outSize Pointer to variable where the number of bytes written will be stored.
     * @param in Source PCM samples.
     * @param sampleCount Number of composite PCM samples provided.
     *
     * @return bool
     */
    bool Compress(Context* ctx, AmUInt8Buffer out, AmSize& outSize, AmConstInt16Buffer in, AmSize sampleCount);

    /**
     * @brief Decompresses the block of ADPCM data into PCM. This requires no context because ADPCM blocks
     * are independently decompressable. This assumes that a single entire block is always decoded; it must
     * be called multiple times for multiple blocks and cannot resume in the middle of a block.
     *
     * @param out Destination for interleaved PCM samples.
     * @param in Source ADPCM block.
     * @param inSize Size of source ADPCM block.
     * @param channels Number of channels in block (must be determined from other context).
     *
     * @returns The number of converted composite samples (total samples divided by number of channels).
     */
    AmInt32 Decompress(AmInt16Buffer out, AmConstUInt8Buffer in, AmSize inSize, AmUInt32 channels);
} // namespace SparkyStudios::Audio::Amplitude::Compression::ADPCM

#endif // SS_AMPLITUDE_AUDIO_COMPRESSION_ADPCM_H

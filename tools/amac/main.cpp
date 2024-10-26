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

#include <cstdarg>
#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "../src/Core/Codecs/AMS/Codec.h"
#include "../src/Core/Codecs/MP3/Codec.h"
#include "../src/Core/Codecs/WAV/Codec.h"

#define AM_FLAG_NOISE_SHAPING 0x1

using namespace SparkyStudios::Audio::Amplitude;

/**
 * @brief Defines in which mode the process should run.
 */
enum ProcessingMode
{
    ePM_UNKNOWN = 0,
    ePM_ENCODE = 1,
    ePM_DECODE = 2,
};

/**
 * @brief Stores the current process state.
 */
struct ProcessingState
{
    /**
     * @brief Defines the current processing mode, should be
     * either encode or decode.
     */
    ProcessingMode mode = ePM_UNKNOWN;

    /**
     * @brief Defines if the process is called in verbose mode.
     */
    bool verbose = false;

    /**
     * @brief The look ahead setting to use when encoding.
     */
    AmUInt32 lookAhead = 3;

    /**
     * @brief Whether to use noise shaping.
     */
    bool noiseShaping = true;

    /**
     * @brief Used to determine the encoded ADPCM block size.
     */
    AmUInt32 blockSizeShift = 0;

    /**
     * @brief Configures the resampler for the encoded ADPCM file.
     */
    struct
    {
        bool enabled = false;
        AmUInt32 targetSampleRate = 48000;
    } resampling;
};

/**
 * @brief The log function, used in verbose mode.
 *
 * @param output The output stream.
 * @param fmt The message format.
 * @param ... The arguments.
 */
static void log(FILE* output, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(output, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(output, fmt, args);
#endif
    va_end(args);
}

static int process(const AmOsString& inFileName, const AmOsString& outFileName, const ProcessingState& state)
{
    AmInt32 res;
    DiskFileSystem fs;

    const auto inputFile = fs.OpenFile(inFileName, eFileOpenMode_Read);
    const auto outputFile = fs.OpenFile(outFileName, eFileOpenMode_Write);

    auto* ams_codec = Codec::Find("ams");
    auto* wav_codec = Codec::Find("wav");

    if (state.mode == ePM_ENCODE)
    {
        auto* codec = Codec::FindCodecForFile(inputFile);
        if (!codec)
        {
            log(stderr, "Unable to load the input file: " AM_OS_CHAR_FMT ". File not found or codec unavailable.\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        auto* decoder = codec->CreateDecoder();
        if (!decoder->Open(inputFile))
        {
            log(stderr, "Unable to load the input file: " AM_OS_CHAR_FMT ". The found codec (%s) was not able to open the input file.\n",
                inFileName.c_str(), codec->GetName().c_str());
            return EXIT_FAILURE;
        }

        const SoundFormat format = decoder->GetFormat();

        AmUInt16 numChannels = format.GetNumChannels();
        AmUInt32 sampleRate = format.GetSampleRate(), blockSize;
        AmUInt64 numSamples = format.GetFramesCount();
        AmUInt64 framesSize = format.GetFrameSize();

        auto* encoder = dynamic_cast<AMSCodec::AMSEncoder*>(ams_codec->CreateEncoder());

        if (state.blockSizeShift > 0)
            blockSize = 1 << state.blockSizeShift;
        else
            blockSize = 256 * numChannels * (sampleRate < 11000 ? 1 : sampleRate / 11000);

        const AmUInt32 samplesPerBlock = (blockSize - numChannels * 4) * (numChannels ^ 3) + 1;

        if (state.verbose)
        {
            log(stdout, "Each %d byte ADPCM block will contain %d samples * %d channels.\n", blockSize, samplesPerBlock, numChannels);
            log(stdout, "Encoding PCM file \"" AM_OS_CHAR_FMT "\" to ADPCM file \"" AM_OS_CHAR_FMT "\"...\n", inFileName.c_str(),
                outFileName.c_str());
        }

        AudioBuffer pcmData(numSamples, numChannels);
        if (decoder->Load(&pcmData) != numSamples || !decoder->Close())
        {
            log(stderr, "Error while decoding PCM file \"" AM_OS_CHAR_FMT "\".\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        encoder->SetEncodingParams(
            blockSize, samplesPerBlock, state.lookAhead,
            state.noiseShaping ? (sampleRate > 64000 ? Compression::ADPCM::eNSM_STATIC : Compression::ADPCM::eNSM_DYNAMIC)
                               : Compression::ADPCM::eNSM_OFF);

        if (state.resampling.enabled)
        {
            auto* resampler = Resampler::Construct("default");
            resampler->Initialize(numChannels, sampleRate, state.resampling.targetSampleRate);

            AmUInt64 f = resampler->GetExpectedOutputFrames(numSamples);
            AudioBuffer output(f, numChannels);

            resampler->Process(pcmData, numSamples, output, f);

            sampleRate = state.resampling.targetSampleRate;
            numSamples = f;

            pcmData = output;
            Resampler::Destruct("default", resampler);
        }

        SoundFormat encodeFormat{};
        encodeFormat.SetAll(sampleRate, numChannels, format.GetBitsPerSample(), numSamples, framesSize, eAudioSampleFormat_Int16);

        encoder->SetFormat(encodeFormat);
        if (!encoder->Open(outputFile))
        {
            log(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for writing.\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (encoder->Write(&pcmData, 0, numSamples) != numSamples || !encoder->Close())
        {
            log(stderr, "Error while encoding ADPCM file \"" AM_OS_CHAR_FMT "\".\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            log(stdout, "Operation completed successfully.\n");
        }

        res = EXIT_SUCCESS;
    }
    else if (state.mode == ePM_DECODE)
    {
        auto* decoder = ams_codec->CreateDecoder();
        auto* encoder = wav_codec->CreateEncoder();

        if (!decoder->Open(inputFile))
        {
            log(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for decoding.\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        const SoundFormat& amsFormat = decoder->GetFormat();

        SoundFormat wavFormat{};
        wavFormat.SetAll(
            amsFormat.GetSampleRate(), amsFormat.GetNumChannels(),
            16, // always decode in 16 bits per sample
            amsFormat.GetFramesCount(),
            amsFormat.GetNumChannels() * sizeof(AmInt16), // Always decode in 16 bits signed integers
            eAudioSampleFormat_Int16);

        encoder->SetFormat(wavFormat);
        if (!encoder->Open(outputFile))
        {
            log(stderr, "Unable to open file \"" AM_OS_CHAR_FMT "\" for encoding.\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            log(stdout, "Decoding ADPCM file \"" AM_OS_CHAR_FMT "\" to PCM file \"" AM_OS_CHAR_FMT "\"...\n", inFileName.c_str(),
                outFileName.c_str());
        }

        AmUInt64 numSamples = amsFormat.GetFramesCount();
        AmUInt64 numChannels = amsFormat.GetNumChannels();

        AudioBuffer adpcmData(numSamples, numChannels);

        if (decoder->Load(&adpcmData) != numSamples || !decoder->Close())
        {
            log(stderr, "Error while decoding ADPCM file \"" AM_OS_CHAR_FMT "\".\n", inFileName.c_str());
            return EXIT_FAILURE;
        }

        if (encoder->Write(&adpcmData, 0, numSamples) != numSamples || !encoder->Close())
        {
            log(stderr, "Error while encoding PCM file \"" AM_OS_CHAR_FMT "\".\n", outFileName.c_str());
            return EXIT_FAILURE;
        }

        if (state.verbose)
        {
            log(stdout, "Operation completed successfully.\n");
        }

        ams_codec->DestroyDecoder(decoder);
        wav_codec->DestroyEncoder(encoder);

        res = EXIT_SUCCESS;
    }
    else
    {
        log(stderr, "No encode/decode mode selected. Either add -e (encode) or -d (decode). Use -h for help.\n");
        return EXIT_FAILURE;
    }

    return res;
}

int main(int argc, char* argv[])
{
    MemoryManager::Initialize();

    char *inFileName = nullptr, *outFileName = nullptr;
    bool noLogo = false, needHelp = false;
    ProcessingState state;

    for (int i = 1; i < argc; i++)
    {
#if defined(AM_WINDOWS_VERSION)
        if (*argv[i] == '-' || *argv[i] == '/')
#else
        if (*argv[i] == '-')
#endif // AM_WINDOWS_VERSION
        {
            switch (argv[i][1])
            {
            case 'H':
            case 'h':
                needHelp = true;
                state.verbose = true;
                break;

            case 'O':
            case 'o':
                noLogo = true;
                break;

            case 'Q':
            case 'q':
                state.verbose = false;
                noLogo = true;
                break;

            case 'V':
            case 'v':
                state.verbose = true;
                break;

            case 'C':
            case 'c':
                state.mode = ePM_ENCODE;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                state.lookAhead = argv[i][1] - '0';
                break;

            case 'B':
            case 'b':
                state.blockSizeShift = strtol(argv[++i], argv, 10);

                if (state.blockSizeShift < 8 || state.blockSizeShift > 15)
                {
                    log(stderr, "\nblock size power must be 8 to 15!\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'F':
            case 'f':
                state.noiseShaping = false;
                break;

            case 'R':
            case 'r':
                state.resampling.enabled = true;
                state.resampling.targetSampleRate = strtol(argv[++i], argv, 10);

                if (state.resampling.targetSampleRate < 8000 || state.blockSizeShift > 384000)
                {
                    log(stderr, "\nInvalid sample rate provided. Please give a value between 8000 and 384000.\n");
                    return EXIT_FAILURE;
                }
                break;

            case 'D':
            case 'd':
                state.mode = ePM_DECODE;
                break;

            default:
                log(stderr, "\nInvalid option: -%c. Use -h for help.\n", **argv);
                return EXIT_FAILURE;
            }
        }
        else if (!inFileName)
        {
            const auto len = strlen(argv[i]);
            inFileName = static_cast<char*>(ampoolmalloc(eMemoryPoolKind_Default, len + 1));

            std::memcpy(inFileName, argv[i], len);
            inFileName[len] = '\0';
        }
        else if (!outFileName)
        {
            const auto len = strlen(argv[i]);
            outFileName = static_cast<char*>(ampoolmalloc(eMemoryPoolKind_Default, len + 1));

            std::memcpy(outFileName, argv[i], len);
            outFileName[len] = '\0';
        }
        else
        {
            log(stderr, "\nUnknown extra argument: %s !\n", *argv);
            return EXIT_FAILURE;
        }
    }

    if (!inFileName || !outFileName)
    {
        needHelp = true;
    }

    if (!noLogo)
    {
        // clang-format off
        log(stdout, "\n");
        log(stdout, "Amplitude Audio Compressor (amac)\n");
        log(stdout, "Copyright (c) 2021-present Sparky Studios - Licensed under Apache 2.0\n");
        log(stdout, "=====================================================================\n");
        log(stdout, "\n");
        // clang-format on
    }

    if (needHelp)
    {
        // clang-format off
        log(stdout, "Usage: amac [OPTIONS] INPUT_FILE OUTPUT_FILE\n");
        log(stdout, "\n");
        log(stdout, "Global options:\n");
        log(stdout, "    -[hH]:        \tDisplay this help message.\n");
        log(stdout, "    -[oO]:        \tHide logo and copyright notice.\n");
        log(stdout, "    -[qQ]:        \tQuiet mode. Shutdown all messages.\n");
        log(stdout, "    -[vV]:        \tVerbose mode. Display all messages.\n");
        log(stdout, "\n");
        log(stdout, "Compression options:\n");
        log(stdout, "    -[cC]:        \tCompress the input file into the output file.\n");
        log(stdout, "    -[0-8]:       \tThe look ahead level.\n");
        log(stdout, "                  \tDefaults to 3.\n");
        log(stdout, "    -[bB] [8-15]: \tThe block size shift.\n");
        log(stdout, "                  \tIf not defined, the block size will be calculated based on the number of channels and the sample rate.\n");
        log(stdout, "    -[fF]:        \tDisable noise shaping. Only used for compression.\n");
        log(stdout, "    -[rR] freq:   \tResamples input data to the target frequency.\n");
        log(stdout, "\n");
        log(stdout, "Decompression options:\n");
        log(stdout, "    -[dD]:        \tDecompress the input file into the output file.\n");
        log(stdout, "\n");
        log(stdout, "Example: amac -c -4 -b 12 input_pcm.wav output_adpcm.ams\n");
        log(stdout, "\n");
        // clang-format on

        return EXIT_SUCCESS;
    }

    Engine::RegisterDefaultPlugins();

    const auto res = process(AM_STRING_TO_OS_STRING(inFileName), AM_STRING_TO_OS_STRING(outFileName), state);

    ampoolfree(eMemoryPoolKind_Default, inFileName);
    ampoolfree(eMemoryPoolKind_Default, outFileName);

    Engine::UnregisterDefaultPlugins();

    MemoryManager::Deinitialize();

    return res;
}

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

#include <cstdarg>
#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

struct ProcessingState
{
    bool verbose = false;

    ePackageFileCompressionAlgorithm compression = ePackageFileCompressionAlgorithm_None;
};

static constexpr AmUInt32 kCurrentVersion = 1;

static constexpr char kProjectDirAttenuators[] = "attenuators";
static constexpr char kProjectDirCollections[] = "collections";
static constexpr char kProjectDirData[] = "data";
static constexpr char kProjectDirEffects[] = "effects";
static constexpr char kProjectDirEvents[] = "events";
static constexpr char kProjectDirRTPC[] = "rtpc";
static constexpr char kProjectDirSoundbanks[] = "soundbanks";
static constexpr char kProjectDirSounds[] = "sounds";
static constexpr char kProjectDirSwitchContainers[] = "switch_containers";
static constexpr char kProjectDirSwitches[] = "switches";

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
    const std::filesystem::path projectPath(inFileName);
    const std::filesystem::path packagePath(outFileName);

    if (!exists(projectPath))
    {
        log(stderr, "The path " AM_OS_CHAR_FMT " does not exist.\n", projectPath.native().c_str());
        return EXIT_FAILURE;
    }

    const auto projectDirectories = { kProjectDirAttenuators,      kProjectDirCollections, kProjectDirData,       kProjectDirEffects,
                                      kProjectDirEvents,           kProjectDirRTPC,        kProjectDirSoundbanks, kProjectDirSounds,
                                      kProjectDirSwitchContainers, kProjectDirSwitches };

    for (const auto& directory : projectDirectories)
    {
        if (!exists(projectPath / directory) || !is_directory(projectPath / directory))
        {
            log(stderr, "Invalid project path. The \"attenuators\" directory is missing.\n");
            return EXIT_FAILURE;
        }
    }

    DiskFile packageFile(absolute(packagePath), eFileOpenMode_Write);

    packageFile.Write(reinterpret_cast<AmConstUInt8Buffer>("AMPK"), 4);
    packageFile.Write16(kCurrentVersion);
    packageFile.Write8(ePackageFileCompressionAlgorithm_None); // TODO: state.compression

    AmSize lastOffset = 0;
    std::vector<AmUInt8> buffer;
    std::vector<PackageFileItemDescription> items;

    const auto appendItem = [&](const std::filesystem::path& file)
    {
        DiskFile diskFile(absolute(file));

        PackageFileItemDescription item;
        std::string relativePath = relative(absolute(file), projectPath).string();
        std::ranges::replace(relativePath, '\\', '/');
        item.m_Name = relativePath;
        item.m_Offset = lastOffset;
        item.m_Size = diskFile.Length();

        buffer.resize(lastOffset + item.m_Size, 0);
        diskFile.Read(buffer.data() + lastOffset, item.m_Size);

        items.push_back(item);
        lastOffset += item.m_Size;
    };

    for (const auto& directory : projectDirectories)
    {
        for (const auto& file : std::filesystem::recursive_directory_iterator(projectPath / directory))
        {
            if (file.is_directory())
                continue;

            appendItem(file);
        }
    }

    for (const auto& file : std::filesystem::directory_iterator(projectPath))
    {
        if (file.is_directory())
            continue;

        appendItem(file);
    }

    packageFile.Write64(items.size());

    for (const auto& item : items)
    {
        packageFile.WriteString(item.m_Name);
        packageFile.Write64(item.m_Offset);
        packageFile.Write64(item.m_Size);
    }

    packageFile.Write(buffer.data(), buffer.size());

    return EXIT_SUCCESS;
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
                state.compression = static_cast<ePackageFileCompressionAlgorithm>(strtol(argv[++i], argv, 10));

                if (state.compression < ePackageFileCompressionAlgorithm_None ||
                    state.compression >= ePackageFileCompressionAlgorithm_Invalid)
                {
                    log(stderr, "\nInvalid compression algorithm!\n");
                    return EXIT_FAILURE;
                }
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
        log(stdout, "Amplitude Packager (ampk)\n");
        log(stdout, "Copyright (c) 2024-present Sparky Studios - Licensed under Apache 2.0\n");
        log(stdout, "=====================================================================\n");
        log(stdout, "\n");
        // clang-format on
    }

    if (needHelp)
    {
        // clang-format off
        log(stdout, "Usage: ampk [OPTIONS] PROJECT_DIR OUTPUT_FILE\n");
        log(stdout, "\n");
        log(stdout, "Options:\n");
        log(stdout, "    -[hH]:        \tDisplay this help message.\n");
        log(stdout, "    -[oO]:        \tHide logo and copyright notice.\n");
        log(stdout, "    -[qQ]:        \tQuiet mode. Shutdown all messages.\n");
        log(stdout, "    -[vV]:        \tVerbose mode. Display all messages.\n");
        log(stdout, "    -[cC]:        \tThe compression algorithm to use.\n");
        log(stdout, "                  \tIf not defined, the resulting package will not be compressed. The available values are:\n");
        log(stdout, "           0:     \tNo compression.\n");
        log(stdout, "           1:     \tZLib compression.\n");
        log(stdout, "\n");
        log(stdout, "Example: ampk -c 1 /path/to/project/ output_package.ampk\n");
        log(stdout, "\n");
        // clang-format on

        return EXIT_SUCCESS;
    }

    const int res = process(AM_STRING_TO_OS_STRING(inFileName), AM_STRING_TO_OS_STRING(outFileName), state);

    ampoolfree(eMemoryPoolKind_Default, inFileName);
    ampoolfree(eMemoryPoolKind_Default, outFileName);

    return res;
}

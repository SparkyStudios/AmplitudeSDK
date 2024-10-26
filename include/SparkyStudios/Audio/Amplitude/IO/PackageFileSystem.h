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

#pragma once

#ifndef _AM_IO_PACKAGE_FILESYSTEM_H
#define _AM_IO_PACKAGE_FILESYSTEM_H

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Defines the algorithms a package file can be compressed with.
     *
     * @ingroup io
     */
    enum ePackageFileCompressionAlgorithm : AmUInt8
    {
        /**
         * @brief No compression algorithm has been used for the package file.
         */
        ePackageFileCompressionAlgorithm_None,

        /**
         * @brief The package file has been compressed using ZLib.
         */
        ePackageFileCompressionAlgorithm_ZLib,

        /**
         * @brief Invalid compression algorithm.
         */
        ePackageFileCompressionAlgorithm_Invalid
    };

    /**
     * @brief Describes an item in the package file.
     *
     * The item description is stored in the package file's header.
     *
     * @ingroup io
     */
    struct PackageFileItemDescription
    {
        /**
         * @brief The name of the package item.
         *
         * @note It usually refers to the resource path.
         */
        AmString m_Name;

        /**
         * @brief The offset of the package item in the package file.
         */
        AmSize m_Offset = 0;

        /**
         * @brief The size of the package item in bytes.
         */
        AmSize m_Size = 0;
    };

    /**
     * @brief Provides metadata about the package file.
     *
     * @ingroup io
     */
    struct PackageFileHeaderDescription
    {
        /**
         * @brief Package file header tag.
         *
         * @note Should be equal to 'AMPK'.
         */
        AmUInt8 m_Header[4] = { 'A', 'M', 'P', 'K' };

        /**
         * @brief Package file version.
         *
         * This is used to implement new features in package
         * files and still be backward compatible with old versions.
         */
        AmUInt16 m_Version = 0;

        /**
         * @brief The compression algorithm used for this package file.
         */
        ePackageFileCompressionAlgorithm m_CompressionAlgorithm = ePackageFileCompressionAlgorithm_Invalid;

        /**
         * @brief The description of each item in the package file.
         *
         * The total number of descriptions should match the number of items.
         */
        std::vector<PackageFileItemDescription> m_Items;
    };

    /**
     * @brief A `FileSystem` implementation that provides access to an Amplitude package file.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC PackageFileSystem final : public FileSystem
    {
    public:
        /**
         * @brief Constructs a new `PackageFileSystem` instance.
         */
        PackageFileSystem();

        /**
         * @brief Destroys the `PackageFileSystem` instance.
         */
        ~PackageFileSystem() override;

        /**
         * @inherit
         */
        void SetBasePath(const AmOsString& basePath) override;

        /**
         * @inherit
         */
        [[nodiscard]] const AmOsString& GetBasePath() const override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString ResolvePath(const AmOsString& path) const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool Exists(const AmOsString& path) const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool IsDirectory(const AmOsString& path) const override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString Join(const std::vector<AmOsString>& parts) const override;

        /**
         * @inherit
         */
        [[nodiscard]] std::shared_ptr<File> OpenFile(const AmOsString& path, eFileOpenMode mode = eFileOpenMode_Read) const override;

        /**
         * @inherit
         */
        void StartOpenFileSystem() override;

        /**
         * @inherit
         */
        bool TryFinalizeOpenFileSystem() override;

        /**
         * @inherit
         */
        void StartCloseFileSystem() override;

        /**
         * @inherit
         */
        bool TryFinalizeCloseFileSystem() override;

        /**
         * @brief Returns if the package file is valid and loaded.
         *
         * @return @c true if the package file is valid and loaded, @c false otherwise.
         */
        [[nodiscard]] bool IsValid() const;

    private:
        /**
         * @brief Loads the package in a background thread.
         *
         * @param[in] pParam The `PackageFileSystem` instance to load.
         *
         * @internal
         */
        static void LoadPackage(AmVoidPtr pParam);

        std::filesystem::path _packagePath;
        AmUniquePtr<eMemoryPoolKind_IO, DiskFile> _packageFile;

        AmThreadHandle _loadingThreadHandle;
        mutable bool _initialized;
        bool _valid;

        PackageFileHeaderDescription _header;
        AmSize _headerSize;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_PACKAGE_FILESYSTEM_H

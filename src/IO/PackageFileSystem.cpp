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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageItemFile.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The last supported package file version.
     */
    static constexpr AmUInt16 kLastPackageFileVersion = 1;

    PackageFileSystem::PackageFileSystem()
        : _packageFile(nullptr)
        , _loadingThreadHandle(nullptr)
        , _initialized(false)
        , _valid(false)
        , _header()
        , _headerSize(0)
    {}

    PackageFileSystem::~PackageFileSystem()
    {
        if (_loadingThreadHandle != nullptr)
            Thread::Release(_loadingThreadHandle);

        _packageFile.reset(nullptr);
        _initialized = false;
        _valid = false;
        _header = {};
        _headerSize = 0;
    }

    void PackageFileSystem::SetBasePath(const AmOsString& basePath)
    {
        const auto& p = std::filesystem::path(basePath);
        _packagePath = p.is_relative() ? (std::filesystem::current_path() / p).lexically_normal().make_preferred() : p;
    }

    const AmOsString& PackageFileSystem::GetBasePath() const
    {
        return _packagePath.native();
    }

    AmOsString PackageFileSystem::ResolvePath(const AmOsString& path) const
    {
        return std::filesystem::path(path).lexically_normal().native();
    }

    bool PackageFileSystem::Exists(const AmOsString& path) const
    {
        if (!IsValid())
            return false;

        const auto it = std::ranges::find_if(
            _header.m_Items,
            [&path](const PackageFileItemDescription& item)
            {
                return AM_STRING_TO_OS_STRING(item.m_Name) == path;
            });

        return it != _header.m_Items.end();
    }

    bool PackageFileSystem::IsDirectory(const AmOsString& path) const
    {
        return false;
    }

    AmOsString PackageFileSystem::Join(const std::vector<AmOsString>& parts) const
    {
        if (parts.empty())
            return AM_OS_STRING("");

        std::filesystem::path joined(parts[0]);

        for (AmSize i = 1, l = parts.size(); i < l; i++)
            joined += AM_OS_STRING("/") + parts[i];

        return joined.lexically_normal().native();
    }

    std::shared_ptr<File> PackageFileSystem::OpenFile(const AmOsString& path, eFileOpenMode mode) const
    {
        if (!IsValid())
            return nullptr;

        const auto it = std::ranges::find_if(
            _header.m_Items,
            [&path](const PackageFileItemDescription& item)
            {
                return AM_STRING_TO_OS_STRING(item.m_Name) == path;
            });

        if (it == _header.m_Items.end())
            return nullptr;

        return std::shared_ptr<PackageItemFile>(
            ampoolnew(eMemoryPoolKind_IO, PackageItemFile, &*it, _packagePath, _headerSize),
            am_delete<eMemoryPoolKind_IO, PackageItemFile>{});
    }

    void PackageFileSystem::StartOpenFileSystem()
    {
        if (_loadingThreadHandle != nullptr)
            Thread::Release(_loadingThreadHandle);

        if (_packageFile != nullptr)
            StartCloseFileSystem();

        _loadingThreadHandle = Thread::CreateThread(&PackageFileSystem::LoadPackage, this);
    }

    bool PackageFileSystem::TryFinalizeOpenFileSystem()
    {
        return _initialized == true;
    }

    void PackageFileSystem::StartCloseFileSystem()
    {
        _packageFile->Close();
        _packageFile.reset(nullptr);
        _initialized = false;
    }

    bool PackageFileSystem::TryFinalizeCloseFileSystem()
    {
        return _packageFile == nullptr;
    }

    bool PackageFileSystem::IsValid() const
    {
        return _valid;
    }

    void PackageFileSystem::LoadPackage(AmVoidPtr pParam)
    {
        auto* pFileSystem = static_cast<PackageFileSystem*>(pParam);

        pFileSystem->_packageFile.reset(ampoolnew(eMemoryPoolKind_IO, DiskFile, pFileSystem->_packagePath));

        if (!pFileSystem->_packageFile->IsValid())
        {
            pFileSystem->_initialized = true;
            return;
        }

        // Read package file header
        {
            // Tag
            pFileSystem->_packageFile->Read(pFileSystem->_header.m_Header, 4);
            if (pFileSystem->_header.m_Header[0] != 'A' || pFileSystem->_header.m_Header[1] != 'M' ||
                pFileSystem->_header.m_Header[2] != 'P' || pFileSystem->_header.m_Header[3] != 'K')
            {
                pFileSystem->_initialized = true;
                return;
            }

            // Version
            pFileSystem->_header.m_Version = pFileSystem->_packageFile->Read16();
            if (pFileSystem->_header.m_Version > kLastPackageFileVersion)
            {
                pFileSystem->_initialized = true;
                return;
            }

            // Compression Algorithm
            pFileSystem->_header.m_CompressionAlgorithm = static_cast<ePackageFileCompressionAlgorithm>(pFileSystem->_packageFile->Read8());

            // Item Descriptions
            if (const AmSize itemsCount = pFileSystem->_packageFile->Read64(); itemsCount > 0)
            {
                pFileSystem->_header.m_Items.resize(itemsCount);
                for (AmSize i = 0; i < itemsCount; i++)
                {
                    auto& item = pFileSystem->_header.m_Items[i];

                    // Item Name/Path
                    item.m_Name = pFileSystem->_packageFile->ReadString();

                    // Item Offset
                    item.m_Offset = pFileSystem->_packageFile->Read64();

                    // Item Size
                    item.m_Size = pFileSystem->_packageFile->Read64();
                }
            }
        }

        pFileSystem->_valid = true;
        pFileSystem->_initialized = true;

        pFileSystem->_headerSize = pFileSystem->_packageFile->Position();
    }
} // namespace SparkyStudios::Audio::Amplitude

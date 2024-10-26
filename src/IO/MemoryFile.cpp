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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/MemoryFile.h>

namespace SparkyStudios::Audio::Amplitude
{

    MemoryFile::MemoryFile()
        : m_dataPtr(nullptr)
        , m_dataSize(0)
        , m_offset(0)
        , m_dataOwned(false)
    {}

    MemoryFile::MemoryFile(AmUInt8Buffer buffer, AmSize size, bool copy, bool takeOwnership)
        : MemoryFile()
    {
        OpenMem(buffer, size, copy, takeOwnership);
    }

    MemoryFile::~MemoryFile()
    {
        Close();
    }

    AmOsString MemoryFile::GetPath() const
    {
        return AM_OS_STRING("");
    }

    bool MemoryFile::Eof()
    {
        return m_offset >= m_dataSize;
    }

    AmSize MemoryFile::Read(AmUInt8Buffer dst, AmSize bytes)
    {
        if (m_offset + bytes >= m_dataSize)
            bytes = m_dataSize - m_offset;

        std::memcpy(dst, m_dataPtr + m_offset, bytes);
        m_offset += bytes;

        return bytes;
    }

    AmSize MemoryFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        const auto bytesToWrite = std::min(bytes, m_dataSize - m_offset);

        std::memcpy(m_dataPtr + m_offset, src, bytesToWrite);
        m_offset += bytesToWrite;

        return bytesToWrite;
    }

    AmSize MemoryFile::Length()
    {
        return m_dataSize;
    }

    void MemoryFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        if (origin == eFileSeekOrigin_Start)
            m_offset = offset;
        else if (origin == eFileSeekOrigin_Current)
            m_offset += offset;
        else if (origin == eFileSeekOrigin_End)
            m_offset = m_dataSize + offset;

        if (m_offset > m_dataSize - 1)
            m_offset = m_dataSize - 1;
    }

    AmSize MemoryFile::Position()
    {
        return m_offset;
    }

    AmVoidPtr MemoryFile::GetPtr()
    {
        return m_dataPtr;
    }

    bool MemoryFile::IsValid() const
    {
        return m_dataPtr != nullptr;
    }

    AmResult MemoryFile::Open(AmSize size)
    {
        return OpenMem(static_cast<AmConstUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_IO, size)), size, false, true);
    }

    AmResult MemoryFile::OpenMem(AmConstUInt8Buffer buffer, AmSize size, bool copy, bool takeOwnership)
    {
        if (buffer == nullptr || size == 0)
            return eErrorCode_InvalidParameter;

        Close();

        m_dataSize = size;

        if (copy)
        {
            m_dataOwned = true;
            m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_IO, size));

            if (m_dataPtr == nullptr)
                return eErrorCode_OutOfMemory;

            std::memcpy(m_dataPtr, buffer, size);
            return eErrorCode_Success;
        }

        m_dataPtr = const_cast<AmUInt8Buffer>(buffer);
        m_dataOwned = takeOwnership;

        return eErrorCode_Success;
    }

    AmResult MemoryFile::OpenToMem(const std::filesystem::path& fileName)
    {
        if (fileName.empty())
            return eErrorCode_InvalidParameter;

        Close();

        DiskFile df;
        if (const AmResult res = df.Open(fileName); res != eErrorCode_Success)
            return res;

        m_dataSize = df.Length();
        m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_IO, m_dataSize));

        if (m_dataPtr == nullptr)
            return eErrorCode_OutOfMemory;

        df.Read(m_dataPtr, m_dataSize);
        m_dataOwned = true;

        df.Close();

        return eErrorCode_Success;
    }

    AmResult MemoryFile::OpenFileToMem(File* file)
    {
        if (!file)
            return eErrorCode_InvalidParameter;

        Close();
        m_offset = file->Position();

        m_dataSize = file->Length();
        m_dataPtr = static_cast<AmUInt8Buffer>(ampoolmalloc(eMemoryPoolKind_IO, m_dataSize));

        if (m_dataPtr == nullptr)
            return eErrorCode_OutOfMemory;

        file->Seek(0, eFileSeekOrigin_Start);
        file->Read(m_dataPtr, m_dataSize);
        file->Seek(m_offset, eFileSeekOrigin_Start);

        m_dataOwned = true;

        return eErrorCode_Success;
    }

    void MemoryFile::Close()
    {
        if (m_dataOwned && m_dataPtr != nullptr)
            ampoolfree(eMemoryPoolKind_IO, m_dataPtr);

        m_dataPtr = nullptr;
        m_dataSize = 0;
        m_offset = 0;
        m_dataOwned = false;
    }
} // namespace SparkyStudios::Audio::Amplitude
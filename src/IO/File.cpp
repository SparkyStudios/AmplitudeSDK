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

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmUInt8 File::Read8()
    {
        AmUInt8 d = 0;
        Read(&d, 1);
        return d;
    }

    AmUInt16 File::Read16()
    {
        AmUInt16 d = 0;
        Read(reinterpret_cast<AmUInt8Buffer>(&d), 2);
        return d;
    }

    AmUInt32 File::Read32()
    {
        AmUInt32 d = 0;
        Read(reinterpret_cast<AmUInt8Buffer>(&d), 4);
        return d;
    }

    AmUInt64 File::Read64()
    {
        AmUInt64 d = 0;
        Read(reinterpret_cast<AmUInt8Buffer>(&d), 8);
        return d;
    }

    AmString File::ReadString()
    {
        const AmUInt32 len = Read32();

        AmString s;
        s.resize(len);
        Read(reinterpret_cast<AmUInt8Buffer>(s.data()), len);

        return s;
    }

    AmSize File::Write8(AmUInt8 value)
    {
        return Write(&value, 1);
    }

    AmSize File::Write16(AmUInt16 value)
    {
        return Write(reinterpret_cast<AmUInt8Buffer>(&value), 2);
    }

    AmSize File::Write32(AmUInt32 value)
    {
        return Write(reinterpret_cast<AmUInt8Buffer>(&value), 4);
    }

    AmSize File::Write64(AmUInt64 value)
    {
        return Write(reinterpret_cast<AmUInt8Buffer>(&value), 8);
    }

    AmSize File::WriteString(const AmString& value)
    {
        AmSize written = Write32(value.length());
        written += Write(reinterpret_cast<AmConstUInt8Buffer>(value.data()), value.length());
        return written;
    }

    void File::Seek(AmSize offset)
    {
        Seek(offset, eFileSeekOrigin_Start);
    }

    AmVoidPtr File::GetPtr()
    {
        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude

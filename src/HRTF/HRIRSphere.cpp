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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Math/BarycentricCoordinates.h>

#include <HRTF/HRIRSphere.h>
#include <Math/FaceBSPTree.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    HRIRSphereImpl::HRIRSphereImpl()
        : ResourceImpl()
        , _loaded(false)
    {}

    HRIRSphereImpl::~HRIRSphereImpl()
    {
        _vertices.clear();
        _faces.clear();
    }

    void HRIRSphereImpl::SetResource(const AmOsString& filePath)
    {
        m_filename = filePath;
    }

    void HRIRSphereImpl::Load(const FileSystem* loader)
    {
        if (_loaded)
            return;

        if (!loader->Exists(m_filename))
        {
            amLogError("Failed to load HRIRSphere: " AM_OS_CHAR_FMT " does not exist.", m_filename.c_str());
            return;
        }

        const auto file = loader->OpenFile(m_filename);

        // Read the header magic
        file->Read(_header.m_Header, 4);

        if (std::strncmp(reinterpret_cast<char*>(_header.m_Header), "AMIR", 4) != 0)
        {
            amLogError("Failed to load HRIRSphere: " AM_OS_CHAR_FMT " is not a valid HRIRSphere file.", m_filename.c_str());
            return;
        }

        _header.m_Version = file->Read16();
        _header.m_SampleRate = file->Read32();
        _header.m_IRLength = file->Read32();
        _header.m_VertexCount = file->Read32();
        _header.m_IndexCount = file->Read32();

        std::vector<AmUInt32> indices(_header.m_IndexCount);
        file->Read(reinterpret_cast<AmUInt8Buffer>(indices.data()), _header.m_IndexCount * sizeof(AmUInt32));

        _vertices.resize(_header.m_VertexCount);

        AmUInt32 i = 0;
        std::vector<AmVec3> vertices(_header.m_VertexCount);

        for (auto& vertex : _vertices)
        {
            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_Position), sizeof(AmVec3));

            vertex.m_LeftIR.resize(_header.m_IRLength);
            file->Read(reinterpret_cast<AmUInt8Buffer>(vertex.m_LeftIR.data()), _header.m_IRLength * sizeof(AmReal32));

            vertex.m_RightIR.resize(_header.m_IRLength);
            file->Read(reinterpret_cast<AmUInt8Buffer>(vertex.m_RightIR.data()), _header.m_IRLength * sizeof(AmReal32));

            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_LeftDelay), sizeof(AmReal32));

            file->Read(reinterpret_cast<AmUInt8Buffer>(&vertex.m_RightDelay), sizeof(AmReal32));

            vertices[i] = vertex.m_Position;
            ++i;
        }

        const AmUInt32 faceCount = indices.size() / 3;
        _faces.resize(faceCount);

        for (i = 0; i < faceCount; ++i)
        {
            _faces[i].m_A = indices[i * 3 + 0];
            _faces[i].m_B = indices[i * 3 + 1];
            _faces[i].m_C = indices[i * 3 + 2];
        }

        _tree.Build(vertices, _faces);

        _loaded = true;
    }

    const std::vector<HRIRSphereVertex>& HRIRSphereImpl::GetVertices() const
    {
        return _vertices;
    }

    const std::vector<Face>& HRIRSphereImpl::GetFaces() const
    {
        return _faces;
    }

    const HRIRSphereVertex& HRIRSphereImpl::GetVertex(AmUInt32 index) const
    {
        return _vertices[index];
    }

    AmUInt32 HRIRSphereImpl::GetVertexCount() const
    {
        return _vertices.size();
    }

    AmUInt32 HRIRSphereImpl::GetFaceCount() const
    {
        return _faces.size();
    }

    AmUInt32 HRIRSphereImpl::GetSampleRate() const
    {
        return _header.m_SampleRate;
    }

    AmUInt32 HRIRSphereImpl::GetIRLength() const
    {
        return _header.m_IRLength;
    }

    eHRIRSphereSamplingMode HRIRSphereImpl::GetSamplingMode() const
    {
        return _samplingMode;
    }

    void HRIRSphereImpl::SetSamplingMode(eHRIRSphereSamplingMode mode)
    {
        _samplingMode = mode;
    }

    void HRIRSphereImpl::Sample(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const
    {
        switch (_samplingMode)
        {
        case eHRIRSphereSamplingMode_Bilinear:
            return SampleBilinear(direction, leftHRIR, rightHRIR);
        case eHRIRSphereSamplingMode_NearestNeighbor:
            return SampleNearestNeighbor(direction, leftHRIR, rightHRIR);
        }
    }

    void HRIRSphereImpl::Transform(const AmMat4& matrix)
    {
        for (auto& vertex : _vertices)
            vertex.m_Position = AM_Mul(matrix, AM_V4V(vertex.m_Position, 1.0f)).XYZ;
    }

    bool HRIRSphereImpl::IsLoaded() const
    {
        return _loaded;
    }

    void HRIRSphereImpl::SampleBilinear(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const
    {
        const auto& dir = AM_Mul(direction, 10.0f);
        const auto* face = _tree.Query(dir);

        if (face == nullptr)
            return;

        // If we are very close to any vertex, just return the HRIR of that vertex
        {
            const auto* vertex = GetClosestVertex(direction, face);

            if (vertex != nullptr)
            {
                const AmSize length = vertex->m_LeftIR.size();
                std::memcpy(leftHRIR, vertex->m_LeftIR.data(), length * sizeof(AmReal32));
                std::memcpy(rightHRIR, vertex->m_RightIR.data(), length * sizeof(AmReal32));
                return;
            }
        }

        const auto& vertexA = _vertices[face->m_A];
        const auto& vertexB = _vertices[face->m_B];
        const auto& vertexC = _vertices[face->m_C];

        // Otherwise, perform bilinear interpolation
        {
            BarycentricCoordinates barycenter;
            if (!BarycentricCoordinates::RayTriangleIntersection(
                    AM_V3(0.0f, 0.0f, 0.0f), dir, { vertexA.m_Position, vertexB.m_Position, vertexC.m_Position }, barycenter))
            {
                return;
            }

            const AmSize length = vertexA.m_LeftIR.size();

            for (AmSize i = 0; i < length; ++i)
            {
                leftHRIR[i] =
                    vertexA.m_LeftIR[i] * barycenter.m_U + vertexB.m_LeftIR[i] * barycenter.m_V + vertexC.m_LeftIR[i] * barycenter.m_W;

                rightHRIR[i] =
                    vertexA.m_RightIR[i] * barycenter.m_U + vertexB.m_RightIR[i] * barycenter.m_V + vertexC.m_RightIR[i] * barycenter.m_W;
            }
        }
    }

    void HRIRSphereImpl::SampleNearestNeighbor(const AmVec3& direction, AmReal32* leftHRIR, AmReal32* rightHRIR) const
    {
        const auto& dir = AM_Mul(direction, 10.0f);
        const auto* face = _tree.Query(dir);

        if (face == nullptr)
            return;

        // If we are very close to any vertex, just return the HRIR of that vertex
        {
            const auto* vertex = GetClosestVertex(direction, face);

            if (vertex != nullptr)
            {
                const AmSize length = vertex->m_LeftIR.size();
                std::memcpy(leftHRIR, vertex->m_LeftIR.data(), length * sizeof(AmReal32));
                std::memcpy(rightHRIR, vertex->m_RightIR.data(), length * sizeof(AmReal32));
                return;
            }
        }

        const auto& vertexA = _vertices[face->m_A];
        const auto& vertexB = _vertices[face->m_B];
        const auto& vertexC = _vertices[face->m_C];

        // Otherwise, perform nearest neighbor interpolation
        {
            BarycentricCoordinates barycenter;
            if (!BarycentricCoordinates::RayTriangleIntersection(
                    AM_V3(0.0f, 0.0f, 0.0f), dir, { vertexA.m_Position, vertexB.m_Position, vertexC.m_Position }, barycenter))
            {
                return;
            }

            const AmSize length = vertexA.m_LeftIR.size();
            const AmReal32 min = std::min({ barycenter.m_U, barycenter.m_V, barycenter.m_W });

            if (min == barycenter.m_V)
            {
                std::memcpy(leftHRIR, vertexA.m_LeftIR.data(), length * sizeof(AmReal32));
                std::memcpy(rightHRIR, vertexA.m_RightIR.data(), length * sizeof(AmReal32));
                return;
            }

            if (min == barycenter.m_U)
            {
                std::memcpy(leftHRIR, vertexB.m_LeftIR.data(), length * sizeof(AmReal32));
                std::memcpy(rightHRIR, vertexB.m_RightIR.data(), length * sizeof(AmReal32));
                return;
            }

            if (min == barycenter.m_W)
            {
                std::memcpy(leftHRIR, vertexC.m_LeftIR.data(), length * sizeof(AmReal32));
                std::memcpy(rightHRIR, vertexC.m_RightIR.data(), length * sizeof(AmReal32));
                return;
            }
        }
    }

    const HRIRSphereVertex* HRIRSphereImpl::GetClosestVertex(const AmVec3& position, const Face* face) const
    {
        const auto& vertexA = _vertices[face->m_A];
        const auto& vertexB = _vertices[face->m_B];
        const auto& vertexC = _vertices[face->m_C];

        constexpr AmReal32 k2 = kEpsilon * kEpsilon;

        if (AM_LenSqr(vertexA.m_Position - position) < k2)
            return &vertexA;

        if (AM_LenSqr(vertexB.m_Position - position) < k2)
            return &vertexB;

        if (AM_LenSqr(vertexC.m_Position - position) < k2)
            return &vertexC;

        return nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude

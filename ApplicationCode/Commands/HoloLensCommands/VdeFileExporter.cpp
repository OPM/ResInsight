/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "VdeFileExporter.h"
#include "VdeArrayDataPacket.h"

#include "RicHoloLensExportImpl.h"

#include "RifJsonEncodeDecode.h"

#include "RiaLogging.h"

#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSet.h"
#include "cvfTransform.h"
#include "cvfTrace.h"

#include <QString>
#include <QDir>
#include <QFile>



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeFileExporter::VdeFileExporter(QString absOutputFolder)
:   m_absOutputFolder(absOutputFolder)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::exportViewContents(const RimGridView& view)
{
    cvf::Collection<cvf::Part> allPartsColl;
    RicHoloLensExportImpl::partsForExport(&view, &allPartsColl);

    std::vector<VdeMesh> meshArr;

    for (size_t i = 0; i < allPartsColl.size(); i++)
    {
        const cvf::Part* part = allPartsColl.at(i);
        if (part)
        {
            VdeMesh mesh;
            if (extractMeshFromPart(view, *part, &mesh))
            {
                meshArr.push_back(mesh);
            }
        }
    }


    const QDir outputDir(m_absOutputFolder);
    const QString arrayDataFileNameTrunk = outputDir.absoluteFilePath("arrayData_");

    struct MeshIds
    {
        int vertexArrId;
        int connArrId;

        MeshIds()
        :   vertexArrId(-1),
            connArrId(-1)
        {}
    };

    std::vector<MeshIds> meshIdsArr;

    int nextArrayId = 0;
    for (size_t i = 0; i < meshArr.size(); i++)
    {
        const VdeMesh& mesh = meshArr[i];
        const size_t primCount = mesh.connArr.size()/3;
        cvf::Trace::show("%d:  primCount=%d  meshSourceObjName='%s'", i, primCount, mesh.meshSourceObjName.toLatin1().constData());

        MeshIds meshIds;

        {
            meshIds.vertexArrId = nextArrayId++;

            QString fileName = arrayDataFileNameTrunk + QString::number(meshIds.vertexArrId) + ".bin";

            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly))
            {
                return false;
            }

            const float* floatArr = reinterpret_cast<const float*>(mesh.vertexArr->ptr());
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromFloat32Arr(meshIds.vertexArrId, floatArr, 3*mesh.vertexArr->size());
            file.write(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize());

            // Testing decoding
            {
                VdeArrayDataPacket testPacket = VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize());
                CVF_ASSERT(dataPacket.elementCount() == testPacket.elementCount());
                CVF_ASSERT(dataPacket.elementSize() == testPacket.elementSize());
                CVF_ASSERT(dataPacket.elementType() == testPacket.elementType());
                const float* testArr = reinterpret_cast<const float*>(testPacket.arrayData());
                for (size_t j = 0; j < testPacket.elementCount(); j++)
                {
                    CVF_ASSERT(testArr[j] == floatArr[j]);
                }
            }
        }

        {
            meshIds.connArrId = nextArrayId++;

            QString fileName = arrayDataFileNameTrunk + QString::number(meshIds.connArrId) + ".bin";

            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly))
            {
                return false;
            }

            const unsigned int* uintArr = mesh.connArr.data();
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromUint32Arr(meshIds.connArrId, uintArr, mesh.connArr.size());
            file.write(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize());

            // Testing decoding
            {
                VdeArrayDataPacket testPacket = VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize());
                CVF_ASSERT(dataPacket.elementCount() == testPacket.elementCount());
                CVF_ASSERT(dataPacket.elementSize() == testPacket.elementSize());
                CVF_ASSERT(dataPacket.elementType() == testPacket.elementType());
                const unsigned int* testArr = reinterpret_cast<const unsigned int*>(testPacket.arrayData());
                for (size_t j = 0; j < testPacket.elementCount(); j++)
                {
                    CVF_ASSERT(testArr[j] == uintArr[j]);
                }
            }
        }

        meshIdsArr.push_back(meshIds);
    }


    {
        QVariantList meshVariantList;

        for (size_t i = 0; i < meshArr.size(); i++)
        {
            const VdeMesh& mesh = meshArr[i];
            const MeshIds& meshIds = meshIdsArr[i];

            QMap<QString, QVariant> meshMeta;
            meshMeta["meshSourceObjType"] = mesh.meshSourceObjTypeStr;
            meshMeta["meshSourceObjName"] = mesh.meshSourceObjName;
            meshMeta["verticesPerPrimitive"] = mesh.verticesPerPrimitive;
            meshMeta["vertexArrId"] = meshIds.vertexArrId;
            meshMeta["connArrId"] = meshIds.connArrId;

            meshVariantList.push_back(meshMeta);
        }

        QMap<QString, QVariant> modelMetaJson;
        modelMetaJson["meshArr"] = meshVariantList;

        ResInsightInternalJson::Json jsonCodec;
        const bool prettifyJson = true;
        QByteArray jsonStr = jsonCodec.encode(modelMetaJson, prettifyJson).toLatin1();

        QString jsonFileName = outputDir.absoluteFilePath("modelMeta.json");
        QFile file(jsonFileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        if (file.write(jsonStr) == -1)
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::extractMeshFromPart(const RimGridView& view, const cvf::Part& part, VdeMesh* mesh)
{
    const cvf::DrawableGeo* geo = dynamic_cast<const cvf::DrawableGeo*>(part.drawable());
    if (!geo)
    {
        return false;
    }

    if (geo->primitiveSetCount() != 1)
    {
        RiaLogging::debug("Only geometries with exactly one primitive set is supported");
        return false;
    }

    const cvf::Vec3fArray* vertexArr = geo->vertexArray();
    const cvf::PrimitiveSet* primSet = geo->primitiveSet(0);
    if (!vertexArr || !primSet || primSet->faceCount() == 0)
    {
        return false;
    }


    if (primSet->primitiveType() != cvf::PT_TRIANGLES)
    {
        RiaLogging::debug("Currently only triangle primitive sets are supported");
        return false;
    }

    mesh->verticesPerPrimitive = 3;

    // Possibly transform the vertices
    if (part.transform())
    {
        const size_t vertexCount = vertexArr->size();
        cvf::ref<cvf::Vec3fArray> transVertexArr = new cvf::Vec3fArray(vertexArr->size());

        cvf::Mat4f m = cvf::Mat4f(part.transform()->worldTransform());
        for (size_t i = 0; i < vertexCount; i++)
        {
            transVertexArr->set(i, vertexArr->get(i).getTransformedPoint(m));
        }

        mesh->vertexArr = transVertexArr.p();
    }
    else
    {
        mesh->vertexArr = vertexArr;
    }

    cvf::UIntArray faceConn;
    const size_t faceCount = primSet->faceCount();
    for (size_t iface = 0; iface < faceCount; iface++)
    {
        primSet->getFaceIndices(iface, &faceConn);
        //mesh->connArr.insert(mesh->connArr.end(), faceConn.begin(), faceConn.end());

        // Reverse the winding
        const size_t numConn = faceConn.size();
        for (size_t i = 0; i < numConn; i++)
        {
            mesh->connArr.push_back(faceConn[numConn - i - 1]);
        }
    }

    const QString nameOfObject = RicHoloLensExportImpl::nameFromPart(&part);

    QString srcObjType = "unknown";
    if      (RicHoloLensExportImpl::isGrid(&part)) srcObjType = "grid";
    else if (RicHoloLensExportImpl::isPipe(&part)) srcObjType = "pipe";

    mesh->meshSourceObjTypeStr = srcObjType;
    mesh->meshSourceObjName = nameOfObject;

    return true;
}



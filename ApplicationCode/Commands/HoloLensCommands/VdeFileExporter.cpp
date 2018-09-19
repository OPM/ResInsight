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

    std::vector<cvf::Color3f> colorArr;
    colorArr.push_back(cvf::Color3f::fromByteColor( 81, 134, 148));
    colorArr.push_back(cvf::Color3f::fromByteColor(212, 158,  97));
    colorArr.push_back(cvf::Color3f::fromByteColor(217,  82,  28));
    colorArr.push_back(cvf::Color3f::fromByteColor(212, 148, 138));
    colorArr.push_back(cvf::Color3f::fromByteColor(115, 173, 181));
    colorArr.push_back(cvf::Color3f::fromByteColor(125,  84,  56));
    colorArr.push_back(cvf::Color3f::fromByteColor(206, 193,  55));
    colorArr.push_back(cvf::Color3f::fromByteColor(252, 209, 158));

    for (size_t i = 0; i < allPartsColl.size(); i++)
    {
        const cvf::Part* part = allPartsColl.at(i);
        if (part)
        {
            VdeMesh mesh;
            if (extractMeshFromPart(view, *part, &mesh))
            {
                mesh.color = colorArr[i % colorArr.size()];
                meshArr.push_back(mesh);
            }
        }
    }


    std::vector<VdeMeshContentIds> meshContentIdsArr;

    int nextArrayId = 0;
    for (size_t i = 0; i < meshArr.size(); i++)
    {
        const VdeMesh& mesh = meshArr[i];
        const size_t primCount = mesh.connArr.size()/3;
        cvf::Trace::show("%d:  primCount=%d  meshSourceObjName='%s'", i, primCount, mesh.meshSourceObjName.toLatin1().constData());

        VdeMeshContentIds meshContentIds;

        {
            meshContentIds.vertexArrId = nextArrayId++;
            const float* floatArr = reinterpret_cast<const float*>(mesh.vertexArr->ptr());
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromFloat32Arr(meshContentIds.vertexArrId, floatArr, 3*mesh.vertexArr->size());
            writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

            // Debug testing of decoding
            debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
        }
        {
            meshContentIds.connArrId = nextArrayId++;
            const unsigned int* uintArr = mesh.connArr.data();
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromUint32Arr(meshContentIds.connArrId, uintArr, mesh.connArr.size());
            writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

            // Debug testing of decoding
            debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
        }

        if (mesh.texCoordArr.notNull() && mesh.texImage.notNull())
        {
            {
                meshContentIds.texCoordsArrId = nextArrayId++;
                const float* floatArr = reinterpret_cast<const float*>(mesh.texCoordArr->ptr());
                VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromFloat32Arr(meshContentIds.texCoordsArrId, floatArr, 3*mesh.vertexArr->size());
                writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

                // Debug testing of decoding
                debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
            }
            {
                meshContentIds.texImageArrId = nextArrayId++;
                cvf::ref<cvf::UByteArray> byteArr = mesh.texImage->toRgb();
                VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromUint8ImageRGBArr(meshContentIds.texImageArrId, mesh.texImage->width(), mesh.texImage->height(), byteArr->ptr(), byteArr->size());
                writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

                // Debug testing of decoding
                debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
            }
        }

        meshContentIdsArr.push_back(meshContentIds);
    }

    QString jsonFileName = m_absOutputFolder + "/modelMeta.json";
    if (!writeModelMetaJsonFile(meshArr, meshContentIdsArr, jsonFileName))
    {
        return false;
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


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::writeDataPacketToFile(int arrayId, const VdeArrayDataPacket& packet) const
{
    const QString fileName = m_absOutputFolder + QString("/arrayData_%1.bin").arg(arrayId);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    if (file.write(packet.fullPacketRawPtr(), packet.fullPacketSize()) == -1)
    {
        return false;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::writeModelMetaJsonFile(const std::vector<VdeMesh>& meshArr, const std::vector<VdeMeshContentIds>& meshContentIdsArr, QString fileName)
{
    QVariantList jsonMeshMetaList;

    for (size_t i = 0; i < meshArr.size(); i++)
    {
        const VdeMesh& mesh = meshArr[i];
        const VdeMeshContentIds& meshIds = meshContentIdsArr[i];

        QMap<QString, QVariant> jsonMeshMeta;
        jsonMeshMeta["meshSourceObjType"] = mesh.meshSourceObjTypeStr;
        jsonMeshMeta["meshSourceObjName"] = mesh.meshSourceObjName;

        jsonMeshMeta["verticesPerPrimitive"] = mesh.verticesPerPrimitive;
        jsonMeshMeta["vertexArrId"] = meshIds.vertexArrId;
        jsonMeshMeta["connArrId"] = meshIds.connArrId;

        if (meshIds.texCoordsArrId >= 0 && meshIds.texImageArrId >= 0)
        {
            jsonMeshMeta["texCoordsArrId"] = meshIds.texCoordsArrId;
            jsonMeshMeta["texImageArrId"] = meshIds.texImageArrId;
        }
        else
        {
            QMap<QString, QVariant> jsonColor;
            jsonColor["r"] = mesh.color.r();
            jsonColor["g"] = mesh.color.g();
            jsonColor["b"] = mesh.color.b();

            jsonMeshMeta["color"] = jsonColor;
        }

        jsonMeshMeta["opacity"] = 1.0;

        jsonMeshMetaList.push_back(jsonMeshMeta);
    }

    QMap<QString, QVariant> jsonModelMeta;
    jsonModelMeta["modelName"] = "ResInsightExport";
    jsonModelMeta["meshArr"] = jsonMeshMetaList;

    ResInsightInternalJson::Json jsonCodec;
    const bool prettifyJson = true;
    QByteArray jsonStr = jsonCodec.encode(jsonModelMeta, prettifyJson).toLatin1();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    if (file.write(jsonStr) == -1)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeFileExporter::debugComparePackets(const VdeArrayDataPacket& packetA, const VdeArrayDataPacket& packetB)
{
    CVF_ASSERT(packetA.elementCount() == packetB.elementCount());
    CVF_ASSERT(packetA.elementSize() == packetB.elementSize());
    CVF_ASSERT(packetA.elementType() == packetB.elementType());

    const char* arrA = packetA.arrayData();
    const char* arrB = packetB.arrayData();
    for (size_t i = 0; i < packetA.elementCount(); i++)
    {
        CVF_ASSERT(arrA[i] == arrB[i]);
    }
}


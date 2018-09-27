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
#include "cvfRenderStateTextureBindings.h"
#include "cvfTexture.h"
#include "cvfEffect.h"
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
    std::vector<VdeExportPart> exportPartsArr = RicHoloLensExportImpl::partsForExport(view);
    std::vector<VdeMesh> meshArr;
    for (const auto& exportPart : exportPartsArr)
    {
        VdeMesh mesh;
        if (extractMeshFromExportPart(exportPart, &mesh))
        {
            meshArr.push_back(mesh);
        }
    }

    std::vector<VdeMeshArrayIds> meshArrayIdsArr;

    const size_t meshCount = meshArr.size();
    cvf::Trace::show("Exporting %d meshes", meshCount);
    size_t totNumPrimitives = 0;
    int nextArrayId = 0;
    for (size_t i = 0; i < meshCount; i++)
    {
        const VdeMesh& mesh = meshArr[i];

        const size_t primCount = mesh.connArr.size()/mesh.verticesPerPrimitive;
        totNumPrimitives += primCount;
        cvf::Trace::show("  %2d:  primCount=%d  meshSourceObjName='%s'", i, primCount, mesh.meshSourceObjName.toLatin1().constData());

        VdeMeshArrayIds meshArrayIds;

        {
            meshArrayIds.vertexArrId = nextArrayId++;
            const float* floatArr = reinterpret_cast<const float*>(mesh.vertexArr->ptr());
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromFloat32Arr(meshArrayIds.vertexArrId, floatArr, 3*mesh.vertexArr->size());
            writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

            // Debug testing of decoding
            //debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
        }
        {
            meshArrayIds.connArrId = nextArrayId++;
            const unsigned int* uintArr = mesh.connArr.data();
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromUint32Arr(meshArrayIds.connArrId, uintArr, mesh.connArr.size());
            writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

            // Debug testing of decoding
            //debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
        }

        if (mesh.texCoordArr.notNull() && mesh.texImage.notNull())
        {
            {
                meshArrayIds.texCoordsArrId = nextArrayId++;
                const float* floatArr = reinterpret_cast<const float*>(mesh.texCoordArr->ptr());
                VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromFloat32Arr(meshArrayIds.texCoordsArrId, floatArr, 3*mesh.vertexArr->size());
                writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

                // Debug testing of decoding
                //debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
            }
            {
                meshArrayIds.texImageArrId = nextArrayId++;
                cvf::ref<cvf::UByteArray> byteArr = mesh.texImage->toRgb();
                VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromUint8ImageRGBArr(meshArrayIds.texImageArrId, mesh.texImage->width(), mesh.texImage->height(), byteArr->ptr(), byteArr->size());
                writeDataPacketToFile(dataPacket.arrayId(), dataPacket);

                // Debug testing of decoding
                //debugComparePackets(dataPacket, VdeArrayDataPacket::fromRawPacketBuffer(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize(), nullptr));
            }
        }

        meshArrayIdsArr.push_back(meshArrayIds);
    }

    QString jsonFileName = m_absOutputFolder + "/modelMeta.json";
    if (!writeModelMetaJsonFile(meshArr, meshArrayIdsArr, jsonFileName))
    {
        return false;
    }

    cvf::Trace::show("Total number of primitives exported: %d", totNumPrimitives);

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::extractMeshFromExportPart(const VdeExportPart& exportPart, VdeMesh* mesh)
{
    const cvf::Part* cvfPart = exportPart.part();
    const cvf::DrawableGeo* geo = dynamic_cast<const cvf::DrawableGeo*>(cvfPart ? cvfPart->drawable() : nullptr);
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


    // Support 2 or 3 vertices per primitive
    const cvf::PrimitiveType primType = primSet->primitiveType();
    if (primType != cvf::PT_TRIANGLES && primType != cvf::PT_LINES)
    {
        RiaLogging::debug(QString("Currently only triangle and line primitive sets are supported (saw primitive type: %1)").arg(primType));
        return false;
    }

    const int vertsPerPrimitive = (primType == cvf::PT_TRIANGLES) ? 3 : 2;
    
    mesh->verticesPerPrimitive = vertsPerPrimitive;

    // Possibly transform the vertices
    if (cvfPart->transform())
    {
        const size_t vertexCount = vertexArr->size();
        cvf::ref<cvf::Vec3fArray> transVertexArr = new cvf::Vec3fArray(vertexArr->size());

        cvf::Mat4f m = cvf::Mat4f(cvfPart->transform()->worldTransform());
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

    // Fetch connectivities
    // Using getFaceIndices() allows us to access strips and fans in the same way as triangles
    // Note that HoloLens visualization wants triangles in clockwise order so we try and fix the winding
    // This point might be moot if the HoloLens visualization always has to use two-sideded lighting to get good results
    cvf::UIntArray faceConn;
    const size_t faceCount = primSet->faceCount();
    for (size_t iface = 0; iface < faceCount; iface++)
    {
        primSet->getFaceIndices(iface, &faceConn);

        if (vertsPerPrimitive == 3 && exportPart.winding() == VdeExportPart::COUNTERCLOCKWISE)
        {
            // Reverse the winding
            const size_t numConn = faceConn.size();
            for (size_t i = 0; i < numConn; i++)
            {
                mesh->connArr.push_back(faceConn[numConn - i - 1]);
            }
        }
        else
        {
            mesh->connArr.insert(mesh->connArr.end(), faceConn.begin(), faceConn.end());
        }
    }


    if (exportPart.textureImage() && geo->textureCoordArray())
    {
        mesh->texCoordArr = geo->textureCoordArray();
        mesh->texImage = exportPart.textureImage();
    }


    QString srcObjType = "unknown";
    if      (exportPart.sourceObjectType() == VdeExportPart::OBJ_TYPE_GRID) srcObjType = "grid";
    else if (exportPart.sourceObjectType() == VdeExportPart::OBJ_TYPE_PIPE) srcObjType = "pipe";
    mesh->meshSourceObjTypeStr = srcObjType;

    mesh->meshSourceObjName = exportPart.sourceObjectName();

    mesh->color = exportPart.color();
    mesh->opacity = exportPart.opacity();

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
bool VdeFileExporter::writeModelMetaJsonFile(const std::vector<VdeMesh>& meshArr, const std::vector<VdeMeshArrayIds>& meshContentIdsArr, QString fileName)
{
    QVariantList jsonMeshMetaList;

    for (size_t i = 0; i < meshArr.size(); i++)
    {
        const VdeMesh& mesh = meshArr[i];
        const VdeMeshArrayIds& meshIds = meshContentIdsArr[i];

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

        jsonMeshMeta["opacity"] = mesh.opacity;

        jsonMeshMetaList.push_back(jsonMeshMeta);
    }

    QMap<QString, QVariant> jsonModelMeta;
    jsonModelMeta["modelName"] = "ResInsightExport";
    jsonModelMeta["meshArr"] = jsonMeshMetaList;

    ResInsightInternalJson::Json jsonCodec;
    const bool prettifyJson = true;
    QByteArray jsonStr = jsonCodec.encode(jsonModelMeta, prettifyJson).toLatin1();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
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


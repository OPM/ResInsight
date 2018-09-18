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

#include "RimGridView.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"

#include "RivSimWellPipeSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "RiuViewer.h"

#include "RifJsonEncodeDecode.h"

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


            cvf::cref<cvf::Vec3fArray> vertexArrToExport;

            vertexArrToExport = mesh.vertexArr;


            const float* floatArr = reinterpret_cast<const float*>(vertexArrToExport->ptr());
            VdeArrayDataPacket dataPacket = VdeArrayDataPacket::fromFloat32Arr(meshIds.vertexArrId, floatArr, 3*vertexArrToExport->size());
            file.write(dataPacket.fullPacketRawPtr(), dataPacket.fullPacketSize());

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
            meshMeta["meshSourceObjType"] = "grid";
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


    /*
    QDir outputDir(m_absOutputFolder);

    //QByteArray jsonStr = "Dette er en test\nlinje2";

    QMap<QString, QVariant> json;
    json["sigurd"] = "er kul";
    json["testDouble"] = 1.23;
    json["testInt"] = 1;

    QMap<QString, QVariant> sub;
    sub["keyA"] = 123;

    QVariantList l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);
    sub["keyB"] = l;


    json["theSub"] = sub;


    ResInsightInternalJson::Json jsonCodec;
    QByteArray jsonStr = jsonCodec.encode(json).toLatin1();


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
    */


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

    const cvf::Vec3fArray* vertexArr = geo->vertexArray();
    const cvf::PrimitiveSet* primSet = geo->primitiveSetCount() > 0 ? geo->primitiveSet(0) : nullptr;
    if (!vertexArr || !primSet)
    {
        return false;
    }

    if (primSet->primitiveType() != cvf::PT_TRIANGLES || primSet->faceCount() == 0)
    {
        return false;
    }

    mesh->verticesPerPrimitive = 3;

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
    for (size_t i = 0; i < faceCount; i++)
    {
        primSet->getFaceIndices(i, &faceConn);
        mesh->connArr.insert(mesh->connArr.end(), faceConn.begin(), faceConn.end());
    }

    const QString nameOfObject = RicHoloLensExportImpl::nameFromPart(&part);
    mesh->meshSourceObjName = nameOfObject;

    return true;
}



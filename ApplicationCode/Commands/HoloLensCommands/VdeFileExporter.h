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

#pragma once

#include "cvfBase.h"
#include "cvfCollection.h"
#include "cvfArray.h"
#include "cvfTextureImage.h"

#include <QString>

class VdeArrayDataPacket;
class VdeExportPart;

class RimGridView;

namespace cvf
{
class Part;
}


  
//==================================================================================================
//
//
//
//==================================================================================================
struct VdeMesh
{
    QString                         meshSourceObjTypeStr;
    QString                         meshSourceObjName;

    cvf::Color3f                    color;
    float                           opacity;

    int                             verticesPerPrimitive;
    cvf::cref<cvf::Vec3fArray>      vertexArr;
    cvf::cref<cvf::Vec2fArray>      texCoordArr;
    std::vector<cvf::uint>          connArr;
    cvf::cref<cvf::TextureImage>    texImage;

    VdeMesh()
    :   color(1,1,1),
        opacity(1),
        verticesPerPrimitive(-1)
    {}
};


//==================================================================================================
//
//
//
//==================================================================================================
struct VdeMeshArrayIds
{
    int vertexArrId;
    int connArrId;
    int texImageArrId;
    int texCoordsArrId;

    VdeMeshArrayIds()
    :   vertexArrId(-1),
        connArrId(-1),
        texImageArrId(-1),
        texCoordsArrId(-1)
    {}
};



//==================================================================================================
//
//
//
//==================================================================================================
class VdeFileExporter
{
public:
    VdeFileExporter(QString absOutputFolder);

    bool exportViewContents(const RimGridView& view);

private:
    static bool extractMeshFromExportPart(const VdeExportPart& exportPart, VdeMesh* mesh);
    static bool writeModelMetaJsonFile(const std::vector<VdeMesh>& meshArr, const std::vector<VdeMeshArrayIds>& meshContentIdsArr, QString fileName);
    static void debugComparePackets(const VdeArrayDataPacket& packetA, const VdeArrayDataPacket& packetB);

    bool        writeDataPacketToFile(int arrayId, const VdeArrayDataPacket& packet) const;

private:
    QString  m_absOutputFolder;

};

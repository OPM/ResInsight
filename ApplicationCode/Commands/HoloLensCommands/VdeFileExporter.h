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

#include <QString>

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
    QString                     meshSourceObjName;

    int                         verticesPerPrimitive;
    cvf::cref<cvf::Vec3fArray>  vertexArr;
    std::vector<cvf::uint>      connArr;

    VdeMesh()
    :   verticesPerPrimitive(-1)
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
    static bool extractMeshFromPart(const RimGridView& view, const cvf::Part& part, VdeMesh* mesh);

private:
    QString     m_absOutputFolder;

};

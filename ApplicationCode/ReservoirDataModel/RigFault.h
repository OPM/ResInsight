/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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
#include "cvfObject.h"
#include "cvfVector3.h"
#include "cvfBoundingBox.h"

#include <vector>

#include <QString>
#include "cvfStructGrid.h"
#include "cvfCellRange.h"
#include "cafFixedArray.h"

class RigMainGrid;


class RigFault : public cvf::Object
{
public:
   
    struct FaultFace
    {
        FaultFace(size_t nativeGlobalCellIndex, cvf::StructGridInterface::FaceType nativeFace, size_t oppositeGlobalCellIndex, cvf::StructGridInterface::FaceType oppositeFace) :
            m_nativeGlobalCellIndex(nativeGlobalCellIndex),
            m_nativeFace(nativeFace),
            m_oppositeGlobalCellIndex(oppositeGlobalCellIndex),
            m_oppositeFace(oppositeFace)
            { }

        size_t                              m_nativeGlobalCellIndex;
        cvf::StructGridInterface::FaceType  m_nativeFace;
        size_t                              m_oppositeGlobalCellIndex;
        cvf::StructGridInterface::FaceType  m_oppositeFace;
    };

public:
    RigFault();

    void setName(const QString& name);
    QString name() const;

    void addCellRangeForFace(cvf::StructGridInterface::FaceType face, const cvf::CellRange& cellRange);
    void computeFaultFacesFromCellRanges(const RigMainGrid* grid);

    std::vector<FaultFace>&         faultFaces();
    const std::vector<FaultFace>&   faultFaces() const;

private:
    QString m_name;

    caf::FixedArray<std::vector<cvf::CellRange>, 6> m_cellRangesForFaces;
    
    std::vector<FaultFace> m_faultFaces;
};

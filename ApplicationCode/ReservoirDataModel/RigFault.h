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



class RigFault : public cvf::Object
{
public:
    RigFault();

    void setName(const QString& name);
    QString name() const;

    void addCellRangeForFace(cvf::StructGridInterface::FaceType face, const cvf::CellRange& cellRange);

    const std::vector<cvf::CellRange>& cellRangeForFace(cvf::StructGridInterface::FaceType face) const;

private:
    QString m_name;

    std::vector< std::vector<cvf::CellRange> > m_cellRangesForFaces;
};

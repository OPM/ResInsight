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
#include "cvfObject.h"
#include "cvfPart.h"

#include <QString>

//==================================================================================================
///
//==================================================================================================
class VdeExportPart
{
public:
    enum SourceObjectType
    {
        OBJ_TYPE_GRID,
        OBJ_TYPE_PIPE,
        OBJ_TYPE_GRID_MESH,
        OBJ_TYPE_UNKNOWN
    };

    enum Winding
    {
        CLOCKWISE,
        COUNTERCLOCKWISE
    };

public:
    VdeExportPart(cvf::Part* part);

    void setSourceObjectType(SourceObjectType sourceObjectType);
    void setSourceObjectName(const QString& sourceObjectName);
    void setColor(cvf::Color3ub color);
    void setWinding(Winding winding);

    QString          sourceObjectName() const;
    SourceObjectType sourceObjectType() const;
    cvf::Part*       part();
    cvf::Color3ub    color() const;
    Winding          winding() const;

private:
    QString             m_sourceObjectName;
    SourceObjectType    m_sourceObjectType;
    cvf::Color3ub       m_color;
    cvf::ref<cvf::Part> m_part;
    Winding             m_winding;
};

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
    void setSourceObjectCellSetType(const QString& sourceObjectCellSetType);
    void setColor(const cvf::Color3f& color);
    void setWinding(Winding winding);

    cvf::Part*       part();
    QString          sourceObjectName() const;
    QString          sourceObjectCellSetType() const;
    SourceObjectType sourceObjectType() const;
    cvf::Color3f     color() const;
    Winding          winding() const;

private:
    cvf::ref<cvf::Part> m_part;

    QString          m_sourceObjectName;
    QString          m_sourceObjectCellSetType;
    SourceObjectType m_sourceObjectType;
    cvf::Color3f     m_color;
    Winding          m_winding;
};

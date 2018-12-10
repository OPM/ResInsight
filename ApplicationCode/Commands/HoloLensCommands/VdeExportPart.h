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
#include "cvfTextureImage.h"

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
        OBJ_TYPE_UNKNOWN
    };

    enum Winding
    {
        CLOCKWISE,
        COUNTERCLOCKWISE
    };

    enum Role
    {
        GEOMETRY,
        MESH_LINES
    };

public:
    VdeExportPart(cvf::Part* part);

    void setTextureImage(const cvf::TextureImage* textureImage);
    void setSourceObjectType(SourceObjectType sourceObjectType);
    void setSourceObjectName(const QString& sourceObjectName);
    void setSourceObjectCellSetType(const QString& sourceObjectCellSetType);
    void setColor(const cvf::Color3f& color);
    void setOpacity(float opacity);
    void setWinding(Winding winding);
    void setRole(Role role);

    const cvf::Part*         part() const;
    const cvf::TextureImage* textureImage() const;

    QString          sourceObjectName() const;
    QString          sourceObjectCellSetType() const;
    SourceObjectType sourceObjectType() const;
    cvf::Color3f     color() const;
    float            opacity() const;
    Winding          winding() const;
    Role             role() const;

private:
    cvf::cref<cvf::Part>         m_part;
    cvf::cref<cvf::TextureImage> m_textureImage;

    QString          m_sourceObjectName;
    QString          m_sourceObjectCellSetType;
    SourceObjectType m_sourceObjectType;
    cvf::Color3f     m_color;
    float            m_opacity;
    Winding          m_winding;
    Role             m_role;
};

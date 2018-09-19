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

#include "VdeExportPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeExportPart::VdeExportPart(cvf::Part* part)
    : m_part(part)
    , m_sourceObjectName("Unnamed Object")
    , m_sourceObjectType(OBJ_TYPE_UNKNOWN)
    , m_color(cvf::Color3f::MAGENTA)
    , m_winding(COUNTERCLOCKWISE)
    , m_opacity(1.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeExportPart::setSourceObjectType(SourceObjectType sourceObjectType)
{
    m_sourceObjectType = sourceObjectType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeExportPart::setSourceObjectName(const QString& sourceObjectName)
{
    m_sourceObjectName = sourceObjectName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeExportPart::setSourceObjectCellSetType(const QString& sourceObjectCellSetType)
{
    m_sourceObjectCellSetType = sourceObjectCellSetType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeExportPart::setColor(const cvf::Color3f& color)
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeExportPart::setOpacity(float opacity)
{
    m_opacity = opacity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void VdeExportPart::setWinding(Winding winding)
{
    m_winding = winding;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VdeExportPart::sourceObjectName() const
{
    return m_sourceObjectName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString VdeExportPart::sourceObjectCellSetType() const
{
    return m_sourceObjectCellSetType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeExportPart::SourceObjectType VdeExportPart::sourceObjectType() const
{
    return m_sourceObjectType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Part* VdeExportPart::part() const
{
    return m_part.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f VdeExportPart::color() const
{
    return m_color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float VdeExportPart::opacity() const
{
    return m_opacity;
}

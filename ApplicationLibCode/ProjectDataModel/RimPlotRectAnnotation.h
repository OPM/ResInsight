/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimCheckableNamedObject.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimPlotRectAnnotation : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPlotRectAnnotation();

    void setRangeX( double minX, double maxX );
    void setRangeY( double minY, double maxY );

    std::pair<double, double> rangeX() const;
    std::pair<double, double> rangeY() const;

    void         setColor( const cvf::Color3f& color );
    cvf::Color3f color() const;

    void    setText( const QString& text );
    QString text() const;

    void   setTransparency( double transparency );
    double transparency() const;

protected:
    caf::PdmField<double>       m_minX;
    caf::PdmField<double>       m_maxX;
    caf::PdmField<double>       m_minY;
    caf::PdmField<double>       m_maxY;
    caf::PdmField<cvf::Color3f> m_color;
    caf::PdmField<double>       m_transparency;
    caf::PdmField<QString>      m_text;
};

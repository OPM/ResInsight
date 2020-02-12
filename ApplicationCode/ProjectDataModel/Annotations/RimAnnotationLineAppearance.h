/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafPdmObject.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

#include "cafPdmFieldCvfColor.h"

//==================================================================================================
///
///
//==================================================================================================
class RimAnnotationLineAppearance : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum LineStyleEnum
    {
        STYLE_SOLID,
        STYLE_DASH
    };
    using LineStyle = caf::AppEnum<LineStyleEnum>;

public:
    RimAnnotationLineAppearance();

    void setLineFieldsHidden( bool hidden );

    void         setColor( const cvf::Color3f& newColor );
    cvf::Color3f color() const;
    bool         isDashed() const;
    int          thickness() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<bool>         m_lineFieldsHidden;
    caf::PdmField<cvf::Color3f> m_color;
    caf::PdmField<LineStyle>    m_style;
    caf::PdmField<int>          m_thickness;
};

//==================================================================================================
///
///
//==================================================================================================
class RimReachCircleLineAppearance : public RimAnnotationLineAppearance
{
};

//==================================================================================================
///
///
//==================================================================================================
class RimPolylineAppearance : public RimAnnotationLineAppearance
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolylineAppearance();

    void setSphereFieldsHidden( bool hidden );

    void         setSphereColor( const cvf::Color3f& color );
    cvf::Color3f sphereColor() const;
    double       sphereRadiusFactor() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<bool>         m_sphereFieldsHidden;
    caf::PdmField<cvf::Color3f> m_sphereColor;
    caf::PdmField<double>       m_sphereRadiusFactor;
};

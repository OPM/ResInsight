/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cafPdmFieldCvfColor.h"
#include "cvfVector3.h"

class RigPolyLinesData;

class RimPolygonAppearance : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

    void applyAppearanceSettings( RigPolyLinesData* polyLinesData );

    void setIsClosed( bool isClosed );
    bool isClosed() const;

public:
    RimPolygonAppearance();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<bool>         m_isClosed;
    caf::PdmField<bool>         m_showLines;
    caf::PdmField<int>          m_lineThickness;
    caf::PdmField<cvf::Color3f> m_lineColor;

    caf::PdmField<bool>         m_showSpheres;
    caf::PdmField<double>       m_sphereRadiusFactor;
    caf::PdmField<cvf::Color3f> m_sphereColor;

    caf::PdmField<bool>   m_lockPolygonToPlane;
    caf::PdmField<double> m_polygonPlaneDepth;
};

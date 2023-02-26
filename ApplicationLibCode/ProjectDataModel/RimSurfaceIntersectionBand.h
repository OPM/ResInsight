////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimCheckableObject.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h" // Include to make Pdm work for cvf::Color
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrArrayField.h"

class RimSurface;
class RimAnnotationLineAppearance;

//==================================================================================================
//
//
//
//==================================================================================================
class RimSurfaceIntersectionBand : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

public:
    RimSurfaceIntersectionBand();

    void setSurfaces( RimSurface* surface1, RimSurface* surface2 );
    void setBandColor( const cvf::Color3f& color );
    void setBandOpacity( double opacity );
    void setPolygonOffsetUnit( double offset );

    RimAnnotationLineAppearance* lineAppearance() const;
    cvf::Color3f                 bandColor() const;
    float                        bandOpacity() const;

    double polygonOffsetUnit() const;

    RimSurface* surface1() const;
    RimSurface* surface2() const;

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onObjectChanged( const caf::SignalEmitter* emitter );

    QString objectName() const;

private:
    caf::PdmChildField<RimAnnotationLineAppearance*> m_lineAppearance;
    caf::PdmField<cvf::Color3f>                      m_bandColor;
    caf::PdmField<double>                            m_bandOpacity;
    caf::PdmField<double>                            m_bandPolygonOffsetUnit;
    caf::PdmPtrArrayField<RimSurface*>               m_surfaces;
    caf::PdmProxyValueField<QString>                 m_nameProxy;
};

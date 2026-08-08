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
class RimSurfaceCollection;

//==================================================================================================
//
//
//
//==================================================================================================
class RimSurfaceIntersectionCurve : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> objectChanged;

public:
    RimSurfaceIntersectionCurve();

    std::vector<RimSurface*>     surfaces() const;
    RimAnnotationLineAppearance* lineAppearance() const;

    /// The color of the curve for the given surface. The color defined in the Surfaces collection is used unless the
    /// user has specified a custom color.
    cvf::Color3f colorForSurface( const RimSurface* surface ) const;

    static void appendOptionItemsForSources( int                            currentLevel,
                                             RimSurfaceCollection*          currentCollection,
                                             bool                           showEnsembleSurfaces,
                                             QList<caf::PdmOptionItemInfo>& options );

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

    void    onObjectChanged( const caf::SignalEmitter* emitter );
    QString objectName() const;

    void updateColorFromSurface();

private:
    caf::PdmPtrArrayField<RimSurface*>               m_surfaces;
    caf::PdmField<bool>                              m_useCustomColor;
    caf::PdmChildField<RimAnnotationLineAppearance*> m_lineAppearance;
    caf::PdmProxyValueField<QString>                 m_nameProxy;
};

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
#include "cafPdmPtrField.h"

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

    RimSurface*                  surface() const;
    RimAnnotationLineAppearance* lineAppearance() const;

    static void appendOptionItemsForSources( int                            currentLevel,
                                             RimSurfaceCollection*          currentCollection,
                                             bool                           showEnsembleSurfaces,
                                             QList<caf::PdmOptionItemInfo>& options );

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void onObjectChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmPtrField<RimSurface*>                    m_surface1;
    caf::PdmChildField<RimAnnotationLineAppearance*> m_lineAppearance;
};

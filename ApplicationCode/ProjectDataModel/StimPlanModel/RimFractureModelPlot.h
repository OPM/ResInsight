/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimDepthTrackPlot.h"

#include "RiaFractureModelDefines.h"

#include "cafPdmPtrField.h"

class RimFractureModel;
class RimWellLogExtractionCurve;
class RimEclipseCase;

class RimFractureModelPlot : public RimDepthTrackPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureModelPlot();

    void              setFractureModel( RimFractureModel* fractureModel );
    RimFractureModel* fractureModel();

protected:
    RimWellLogExtractionCurve* findCurveByProperty( RiaDefines::CurveProperty curveProperty ) const;

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onLoadDataAndUpdate() override;

private:
    void applyDataSource();

    caf::PdmPtrField<RimFractureModel*> m_fractureModel;
    caf::PdmField<bool>                 m_editFractureModel;
    caf::PdmPtrField<RimEclipseCase*>   m_eclipseCase;
    caf::PdmField<int>                  m_timeStep;
};

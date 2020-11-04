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

#include "RiaStimPlanModelDefines.h"

#include "cafPdmPtrField.h"

class RimStimPlanModel;
class RimWellLogExtractionCurve;
class RimEclipseCase;

class RimStimPlanModelPlot : public RimDepthTrackPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimStimPlanModelPlot();

    void              setStimPlanModel( RimStimPlanModel* stimPlanModel );
    RimStimPlanModel* stimPlanModel();

protected:
    RimWellLogExtractionCurve* findCurveByProperty( RiaDefines::CurveProperty curveProperty ) const;

    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void onLoadDataAndUpdate() override;

private:
    void applyDataSource();

    caf::PdmPtrField<RimStimPlanModel*> m_stimPlanModel;
    caf::PdmField<bool>                 m_editStimPlanModel;
    caf::PdmPtrField<RimEclipseCase*>   m_eclipseCase;
    caf::PdmField<int>                  m_timeStep;
};

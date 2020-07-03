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

#include "cafCmdFeature.h"

#include "RiaDefines.h"

#include "RimElasticPropertiesCurve.h"
#include "RimFractureModelCurve.h"
#include "RimFractureModelStressCurve.h"

class RimEclipseCase;
class RimFractureModelPlot;
class RimFractureModelPlotCollection;
class RimFractureModel;

typedef std::tuple<QString, RiaDefines::ResultCatType, RimFractureModelCurve::MissingValueStrategy, bool, RiaDefines::CurveProperty> PlotDef;
typedef std::vector<PlotDef> PlotDefVector;

//==================================================================================================
///
//==================================================================================================
class RicNewFractureModelPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static RimFractureModelPlot* createPlot( RimEclipseCase* eclipseCase, RimFractureModel* fractureModel, int timeStep );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static void
        createFormationTrack( RimFractureModelPlot* plot, RimFractureModel* fractureModel, RimEclipseCase* eclipseCase );
    static void
        createFaciesTrack( RimFractureModelPlot* plot, RimFractureModel* fractureModel, RimEclipseCase* eclipseCase );
    static void
        createLayersTrack( RimFractureModelPlot* plot, RimFractureModel* fractureModel, RimEclipseCase* eclipseCase );

    static void createParametersTrack( RimFractureModelPlot* plot,
                                       RimFractureModel*     fractureModel,
                                       RimEclipseCase*       eclipseCase,
                                       int                   timeStep,
                                       const QString&        trackTitle,
                                       const PlotDefVector&  curveConfiguration,
                                       bool                  isPlotLogarithmic );

    static void createElasticPropertiesTrack( RimFractureModelPlot*     plot,
                                              RimFractureModel*         fractureModel,
                                              RimEclipseCase*           eclipseCase,
                                              int                       timeStep,
                                              RiaDefines::CurveProperty propertyType );

    static void createStressTrack( RimFractureModelPlot*     plot,
                                   RimFractureModel*         fractureModel,
                                   RimEclipseCase*           eclipseCase,
                                   int                       timeStep,
                                   RiaDefines::CurveProperty propertyType );

    static RimFractureModelPlot* createFractureModelPlot( bool showAfterCreation, const QString& plotDescription );

    static RimFractureModelPlotCollection* fractureModelPlotCollection();
};

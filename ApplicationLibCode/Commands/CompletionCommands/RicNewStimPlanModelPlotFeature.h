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
#include "RiaStimPlanModelDefines.h"
#include "RimPlot.h"
#include "RimPlotCurveAppearance.h"

#include "cvfColor3.h"

class RimEclipseCase;
class RimStimPlanModelPlot;
class RimStimPlanModelPlotCollection;
class RimStimPlanModel;

//==================================================================================================
///
//==================================================================================================
class RicNewStimPlanModelPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static RimStimPlanModelPlot* createPlot( RimStimPlanModel* stimPlanModel );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static void
        createFormationTrack( RimStimPlanModelPlot* plot, RimStimPlanModel* stimPlanModel, RimEclipseCase* eclipseCase );
    static void
        createFaciesTrack( RimStimPlanModelPlot* plot, RimStimPlanModel* stimPlanModel, RimEclipseCase* eclipseCase );
    static void
        createLayersTrack( RimStimPlanModelPlot* plot, RimStimPlanModel* stimPlanModel, RimEclipseCase* eclipseCase );

    static void createParametersTrack( RimStimPlanModelPlot*                         plot,
                                       RimStimPlanModel*                             stimPlanModel,
                                       RimEclipseCase*                               eclipseCase,
                                       int                                           timeStep,
                                       const QString&                                trackTitle,
                                       const std::vector<RiaDefines::CurveProperty>& propertyTypes,
                                       bool                                          isPlotLogarithmic = false );

    static bool shouldShowByDefault( const std::vector<RiaDefines::CurveProperty>& propertyTypes,
                                     bool                                          useDetailedFluidLoss );

    static RimStimPlanModelPlot* createStimPlanModelPlot( bool showAfterCreation, const QString& plotDescription );

    static RimStimPlanModelPlotCollection* stimPlanModelPlotCollection();

    static cvf::Color3f                          defaultColor( RiaDefines::CurveProperty property, int colorIndex );
    static cvf::Color3f                          defaultFillColor( RiaDefines::CurveProperty property );
    static RimPlotCurveAppearance::FillStyle     defaultFillStyle( RiaDefines::CurveProperty property );
    static RiuQwtPlotCurveDefines::LineStyleEnum defaultLineStyle( RiaDefines::CurveProperty property );
    static RimPlot::RowOrColSpan                 defaultColSpan( RiaDefines::CurveProperty property );
    static bool                                  useMinMaxTicksOnly( RiaDefines::CurveProperty property );
};

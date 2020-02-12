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

#include "cafCmdFeature.h"

class RimGeoMechCase;
class RimGeoMechView;
class RimWbsParameters;
class RimWellBoreStabilityPlot;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RicNewWellBoreStabilityPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static RimWellBoreStabilityPlot* createPlot( RimGeoMechCase*         geoMechCase,
                                                 RimWellPath*            wellPath,
                                                 int                     timeStep,
                                                 const RimWbsParameters* parameters = nullptr );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static void createFormationTrack( RimWellBoreStabilityPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase );
    static void createCasingShoeTrack( RimWellBoreStabilityPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase );
    static void createParametersTrack( RimWellBoreStabilityPlot* plot,
                                       RimWellPath*              wellPath,
                                       RimGeoMechCase*           geoMechCase,
                                       int                       timeStep );
    static void createStabilityCurvesTrack( RimWellBoreStabilityPlot* plot,
                                            RimWellPath*              wellPath,
                                            RimGeoMechCase*           geoMechCase,
                                            int                       timeStep );
    static void
        createAnglesTrack( RimWellBoreStabilityPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase, int timeStep );
};

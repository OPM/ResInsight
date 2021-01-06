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

class RimStimPlanModel;
class RimStimPlanModelCollection;
class RimWellPath;
class RimWellPathCollection;
class RimEclipseCase;

//==================================================================================================
///
//==================================================================================================
class RicNewStimPlanModelFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static RimStimPlanModel* addStimPlanModel( RimWellPath*           wellPath,
                                               RimWellPathCollection* wellPathCollection,
                                               RimEclipseCase*        eclipseCase   = nullptr,
                                               int                    timeStep      = 0,
                                               double                 measuredDepth = -1.0 );

protected:
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
    bool isCommandEnabled() override;

private:
    static RimStimPlanModelCollection* selectedStimPlanModelCollection();
};

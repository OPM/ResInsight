/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <QString>

class RimEnsembleWellLogCurveSet;
class RimEnsembleWellLogs;
class RimWellLogTrack;

//==================================================================================================
///
//==================================================================================================
class RicNewEnsembleWellLogPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    // Add a curve set for the given channel to the track. The color is picked from the default palette by colorIndex.
    // The curve set is left unconfigured and not loaded if channelName is empty.
    static RimEnsembleWellLogCurveSet*
        addEnsembleCurveSetToTrack( RimWellLogTrack* track, RimEnsembleWellLogs* ensembleWellLogs, const QString& channelName, int colorIndex );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

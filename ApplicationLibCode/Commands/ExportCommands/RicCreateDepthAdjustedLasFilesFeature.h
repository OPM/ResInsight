/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

class RimEclipseCase;
class RimGeoMechCase;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RicCreateDepthAdjustedLasFilesFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    RicCreateDepthAdjustedLasFilesFeature() = default;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    void createDepthAdjustedWellLogFileFromEclipseCase( RimEclipseCase*                 eclipseCase,
                                                        RimWellPath*                    sourceWell,
                                                        const std::vector<RimWellPath*> destinationWells,
                                                        const std::vector<QString>&     selectedResultProperties,
                                                        const QString&                  exportFolder );
    void createDepthAdjustedWellLogFileFromGeoMechCase( RimGeoMechCase*                 geoMechCase,
                                                        RimWellPath*                    sourceWell,
                                                        const std::vector<RimWellPath*> destinationWells,
                                                        const std::vector<QString>&     selectedResultProperties,
                                                        const QString&                  exportFolder );
};

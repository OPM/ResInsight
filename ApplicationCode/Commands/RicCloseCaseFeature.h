/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include <vector>

class RimEclipseCase;
class RimGeoMechCase;
class RimCase;
class RimIdenticalGridCaseGroup;

//==================================================================================================
/// 
//==================================================================================================
class RicCloseCaseFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void deleteEclipseCase(RimEclipseCase* eclipseCase);
    static bool userConfirmedGridCaseGroupChange(const std::vector<RimEclipseCase*>& casesToBeDeleted);

protected:
    // Overrides
    virtual bool isCommandEnabled();
    virtual void onActionTriggered( bool isChecked );
    virtual void setupActionLook( QAction* actionToSetup );

private:
    std::vector<RimCase*> selectedCases() const;

    void deleteGeoMechCase(RimGeoMechCase* geoMechCase);
    
    static bool hasAnyStatisticsResults(RimIdenticalGridCaseGroup* gridCaseGroup);
    static void removeCaseFromAllGroups(RimEclipseCase* eclipseCase);
};



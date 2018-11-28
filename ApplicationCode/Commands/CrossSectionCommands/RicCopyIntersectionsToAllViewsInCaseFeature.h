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
#include "cafCmdExecuteCommand.h"
#include "cafPdmPointer.h"

class RimCase;
class RimIntersection;
class RimIntersectionBox;
class RimIntersectionCollection;


//==================================================================================================
/// 
//==================================================================================================
class RicCopyIntersectionsToAllViewsInCaseFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    void         copyIntersectionsToOtherViews(RimCase& gridCase, std::vector<RimIntersection*> intersections);
    void         copyIntersectionBoxesToOtherViews(RimCase& gridCase, std::vector<RimIntersectionBox*> intersectionBoxes);
};



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

class RimIntersectionCollection;
class RimSimWellInView;


//==================================================================================================
/// 
//==================================================================================================
class RicNewSimWellIntersectionCmd : public caf::CmdExecuteCommand
{
public:
    RicNewSimWellIntersectionCmd(RimIntersectionCollection* intersectionCollection, RimSimWellInView* simWell);
    virtual ~RicNewSimWellIntersectionCmd();

    virtual QString name();
    virtual void redo();
    virtual void undo();

private:
    caf::PdmPointer<RimIntersectionCollection> m_intersectionCollection;
    caf::PdmPointer<RimSimWellInView> m_simWell;
};



//==================================================================================================
/// 
//==================================================================================================
class RicNewSimWellIntersectionFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    virtual bool isCommandEnabled();
    virtual void onActionTriggered( bool isChecked );
    virtual void setupActionLook( QAction* actionToSetup );
};



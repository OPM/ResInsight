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

#include "RicPickEventHandler.h"

#include "cafCmdExecuteCommand.h"
#include "cafPdmPointer.h"

class RimIntersectionCollection;


//==================================================================================================
/// 
//==================================================================================================
class RicNewPolylineIntersectionFeatureCmd : public caf::CmdExecuteCommand
{
public:
    explicit RicNewPolylineIntersectionFeatureCmd(RimIntersectionCollection* intersectionCollection);
    virtual ~RicNewPolylineIntersectionFeatureCmd();

    virtual QString name() override;
    virtual void redo() override;
    virtual void undo() override;

private:
    caf::PdmPointer<RimIntersectionCollection> m_intersectionCollection;
};



//==================================================================================================
/// 
//==================================================================================================
class RicNewPolylineIntersectionFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    RicNewPolylineIntersectionFeature();

protected:
    virtual bool isCommandEnabled() override;
    virtual void onActionTriggered( bool isChecked ) override;
    virtual void setupActionLook( QAction* actionToSetup ) override;
};



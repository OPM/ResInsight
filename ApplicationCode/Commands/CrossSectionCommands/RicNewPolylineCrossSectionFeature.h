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

#include "RicCommandFeature.h"

#include "cafCmdExecuteCommand.h"
#include "cafPdmPointer.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

class RimCrossSectionCollection;


//==================================================================================================
/// 
//==================================================================================================
class RicNewPolylineCrossSectionFeatureCmd : public caf::CmdExecuteCommand
{
public:
    RicNewPolylineCrossSectionFeatureCmd(RimCrossSectionCollection* crossSectionCollection);
    virtual ~RicNewPolylineCrossSectionFeatureCmd();

    virtual QString name();
    virtual void redo();
    virtual void undo();

private:
    caf::PdmPointer<RimCrossSectionCollection> m_crossSectionCollection;
};



//==================================================================================================
/// 
//==================================================================================================
class RicNewPolylineCrossSectionFeature : public RicCommandFeature
{
    CAF_CMD_HEADER_INIT;

public:
    RicNewPolylineCrossSectionFeature();

protected:
    // Overrides
    virtual bool isCommandEnabled();
    virtual void onActionTriggered( bool isChecked );
    virtual void setupActionLook( QAction* actionToSetup );

    virtual bool handleUiEvent(cvf::Object* uiEventObject);
};



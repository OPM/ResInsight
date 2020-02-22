/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafCmdExecuteCommand.h"
#include "cafCmdFeature.h"
#include "cafPdmPointer.h"

class RimIntersectionResultsDefinitionCollection;

//==================================================================================================
///
//==================================================================================================
class RicAppendSeparateIntersectionResultFeatureCmd : public caf::CmdExecuteCommand
{
public:
    explicit RicAppendSeparateIntersectionResultFeatureCmd( RimIntersectionResultsDefinitionCollection* intersectionCollection );
    ~RicAppendSeparateIntersectionResultFeatureCmd() override;

    QString name() override;
    void    redo() override;
    void    undo() override;

private:
    caf::PdmPointer<RimIntersectionResultsDefinitionCollection> m_intersectionCollection;
};

//==================================================================================================
///
//==================================================================================================
class RicAppendSeparateIntersectionResultFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

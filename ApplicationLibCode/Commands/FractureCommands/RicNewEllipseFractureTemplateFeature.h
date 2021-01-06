/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
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

#include <vector>

class RimEllipseFractureTemplate;
class RimFracture;
class RimFractureTemplate;
class RimFractureTemplateCollection;

//==================================================================================================
///
//==================================================================================================
class RicNewEllipseFractureTemplateFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void createNewTemplateForFractureAndUpdate( RimFracture* fracture );
    static void selectFractureTemplateAndUpdate( RimFractureTemplate* fractureTemplate );

protected:
    static RimEllipseFractureTemplate* createNewTemplate();
    void                               onActionTriggered( bool isChecked ) override;
    void                               setupActionLook( QAction* actionToSetup ) override;
    bool                               isCommandEnabled() override;
};

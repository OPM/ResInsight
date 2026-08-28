/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

//==================================================================================================
///
//==================================================================================================
class RicCreateSumoEnsembleFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // A summary ensemble belongs to the plot window, so the command is only offered there. Returning false
    // removes the entry from the context menu rather than disabling it, see
    // caf::CmdFeatureMenuBuilder::appendToMenu.
    bool isCommandEnabled() const override;

    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

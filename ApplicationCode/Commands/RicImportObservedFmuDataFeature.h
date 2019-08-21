/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "cafPdmField.h"

//==================================================================================================
//
//
//
//==================================================================================================
class RicImportObservedFmuDataFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void selectObservedDataPathInDialog();

private:
    bool isCommandEnabled() override;
    void onActionTriggered(bool isChecked) override;
    void setupActionLook(QAction* actionToSetup) override;
};

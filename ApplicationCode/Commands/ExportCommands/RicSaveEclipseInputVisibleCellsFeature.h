/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

class RimEclipseView;
class RicSaveEclipseInputVisibleCellsUi;

//==================================================================================================
///
//==================================================================================================
class RicSaveEclipseInputVisibleCellsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void openDialogAndExecuteCommand( RimEclipseView* view );
    static void executeCommand( RimEclipseView*                          view,
                                const RicSaveEclipseInputVisibleCellsUi& exportSettings,
                                const QString&                           logPrefix );

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    RimEclipseView* selectedView() const;
};

//==================================================================================================
///
//==================================================================================================
class RicSaveEclipseInputActiveVisibleCellsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static RimEclipseView* selectedView();
};

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

#include "cvfArray.h"
#include "cvfVector3.h"

class RimEclipseView;
class RicExportEclipseSectorModelUi;

//==================================================================================================
///
//==================================================================================================
class RicExportEclipseSectorModelFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void openDialogAndExecuteCommand( RimEclipseView* view );
    static void executeCommand( RimEclipseView*                      view,
                                const RicExportEclipseSectorModelUi& exportSettings,
                                const QString&                       logPrefix );

    static std::pair<cvf::Vec3i, cvf::Vec3i> getVisibleCellRange( RimEclipseView*        view,
                                                                  const cvf::UByteArray& cellVisibility );

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    RimEclipseView* selectedView() const;
};

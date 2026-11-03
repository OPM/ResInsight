/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include <expected>

class RimViewWindow;

//==================================================================================================
///
//==================================================================================================
class RicSnapshotAllPlotsToFileFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static void saveAllPlots();

    // Returns an error description if one or more plots could not be exported
    static std::expected<void, QString> exportSnapshotOfPlotsIntoFolder( const QString& snapshotFolderName,
                                                                         int            width               = -1,
                                                                         int            height              = -1,
                                                                         bool           activateWidget      = false,
                                                                         const QString& prefix              = "",
                                                                         int            viewId              = -1,
                                                                         const QString& preferredFileSuffix = ".png" );

    // Returns an error description if the snapshot could not be exported
    static std::expected<void, QString> exportSnapshotOfPlot( RimViewWindow* viewWindow,
                                                              const QString& snapshotFolderName,
                                                              int            width               = -1,
                                                              int            height              = -1,
                                                              bool           activateWidget      = false,
                                                              const QString& prefix              = "",
                                                              const QString& preferredFileSuffix = ".png" );

protected:
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static void layoutHiddenPlotWindow();

    static std::expected<void, QString> exportSnapshotOfPlotWithoutLayout( RimViewWindow* viewWindow,
                                                                           const QString& snapshotFolderName,
                                                                           int            width,
                                                                           int            height,
                                                                           bool           activateWidget,
                                                                           const QString& prefix,
                                                                           const QString& preferredFileSuffix );
};

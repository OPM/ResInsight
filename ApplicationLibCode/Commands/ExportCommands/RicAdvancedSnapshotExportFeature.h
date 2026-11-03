/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

class RimAdvancedSnapshotExportDefinition;
class RimProject;
class Rim3dView;
class RimGridView;
class RimEclipseView;

//==================================================================================================
///
//==================================================================================================
class RicAdvancedSnapshotExportFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

public:
    static std::expected<void, QString> exportMultipleSnapshots( const QString& folder, RimProject* project );

private:
    // The functions below return the number of snapshots that failed to export
    static size_t  exportViewVariations( Rim3dView* rimView, RimAdvancedSnapshotExportDefinition* msd, const QString& folder );
    static size_t  exportViewVariationsToFolder( RimGridView* rimView, RimAdvancedSnapshotExportDefinition* msd, const QString& folder );
    static QString resultName( Rim3dView* rimView );
    static void    removeViewFromViewCollection( RimEclipseView* view );
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifSurfio.h"

#include "cafCmdFeature.h"

#include <optional>
#include <vector>

#include <QString>

class RimSurface;

//==================================================================================================
///
//==================================================================================================
class RicExportSurfaceToGriFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    // Resamples one surface onto the given regular grid and returns depth values
    // in row-major order (index = j * nx + i). Returns an empty vector on failure.
    static std::vector<float> resampleToGrid( RimSurface* surf, const RigRegularSurfaceData& gridParams );

    // Shows the export dialog, resamples and writes each surface. Format is chosen in the dialog.
    static void exportSurfaces( const std::vector<RimSurface*>& surfaces );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

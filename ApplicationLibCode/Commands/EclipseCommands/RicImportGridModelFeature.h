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

#include "cafCmdFeature.h"

//==================================================================================================
/// Umbrella feature for importing grid models of any supported type
/// (binary GRID/EGRID, text GRDECL, Roff). Presents a single file dialog with
/// filter groups for each format plus a combined group, and routes the selected
/// files to the matching importer based on extension.
//==================================================================================================
class RicImportGridModelFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimDockWindowController.h"

//==================================================================================================
/// Compatibility object used to make project files readable by ResInsight 2026.06 and older.
///
/// These versions create the window controller for a view window from the class keyword
/// "MdiWindowController". Without a section using this keyword, plot windows are never made visible
/// when the project file is opened by an older version.
///
/// The object adds no behavior, it is only used to write the main window id using the old class
/// keyword. It is stored in RimViewWindow::m_legacyWindowController, and is kept in sync with the
/// window controller in use. Can be removed after 2-3 major releases.
//==================================================================================================
class RimMdiWindowController_OBSOLETE : public RimDockWindowController
{
    CAF_PDM_HEADER_INIT;
};

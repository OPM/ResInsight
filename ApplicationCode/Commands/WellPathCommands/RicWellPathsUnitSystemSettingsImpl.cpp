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

#include "RicWellPathsUnitSystemSettingsImpl.h"

#include "RicWellPathsUnitSystemSettingsUi.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem(RimWellPath * wellPath)
{
    if (wellPath->unitSystem() != RimUnitSystem::UNITS_UNKNOWN)
    {
        return true;
    }

    RicWellPathsUnitSystemSettingsUi settings;
    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &settings, "Select Unit System for Well Path", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        wellPath->setUnitSystem(settings.unitSystem());
        return true;
    }
    return false;
}

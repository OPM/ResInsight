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

#include "RicWellPathsUnitSystemSettingsUi.h"

CAF_PDM_SOURCE_INIT( RicWellPathsUnitSystemSettingsUi, "RicWellPathsUnitSystemSettingsUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathsUnitSystemSettingsUi::RicWellPathsUnitSystemSettingsUi()
{
    CAF_PDM_InitObject( "RimWellPathsUnitSystemSettings" );

    CAF_PDM_InitField( &unitSystem, "UnitSystem", RiaDefines::EclipseUnitSystem::UNITS_METRIC, "Unit System" );
    caf::AppEnum<RiaDefines::EclipseUnitSystem>::setEnumSubset( unitSystem.keyword(),
                                                                { RiaDefines::EclipseUnitSystem::UNITS_METRIC,
                                                                  RiaDefines::EclipseUnitSystem::UNITS_FIELD } );
}


/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimcIdenticalGridCaseGroup.h"

#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimIdenticalGridCaseGroup, RimcIdenticalGridCaseGroup_createStatisticsCase, "create_statistics_case" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcIdenticalGridCaseGroup_createStatisticsCase::RimcIdenticalGridCaseGroup_createStatisticsCase( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcIdenticalGridCaseGroup_createStatisticsCase::execute()
{
    auto gridCaseGroup = self<RimIdenticalGridCaseGroup>();
    auto statCase      = gridCaseGroup->createAndAppendEmptyStatisticsCase();

    gridCaseGroup->updateConnectedEditors();

    return statCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcIdenticalGridCaseGroup_createStatisticsCase::resultIsPersistent_obsolete() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcIdenticalGridCaseGroup_createStatisticsCase::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimEclipseStatisticsCase );
}

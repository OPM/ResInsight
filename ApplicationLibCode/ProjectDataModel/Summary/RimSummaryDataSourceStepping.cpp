////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSummaryDataSourceStepping.h"

#include "Summary/RiaSummaryDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void AppEnum<RimSummaryDataSourceStepping::SourceSteppingDimension>::setUp()
{
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR, "VECTOR", "Vector" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::WELL, "WELL", RiaDefines::summaryWell() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::WELL_COMPLETION_NUMBER,
             "WELL_COMPLETION",
             RiaDefines::summaryWellCompletion() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP, "GROUP", RiaDefines::summaryWellGroup() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK, "NETWORK", RiaDefines::summaryNetwork() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::REGION, "REGION", RiaDefines::summaryRegion() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK, "BLOCK", RiaDefines::summaryBlock() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER, "AQUIFER", RiaDefines::summaryAquifer() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::WELL_SEGMENT, "WELL_SEGMENT", RiaDefines::summaryWellSegment() );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::WELL_CONNECTION, "WELL_CONNECTION", RiaDefines::summaryConnection() );
    setDefault( RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR );
}
} // namespace caf

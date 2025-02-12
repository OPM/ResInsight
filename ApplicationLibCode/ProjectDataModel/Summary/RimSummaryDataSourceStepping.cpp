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
    using ssd = RimSummaryDataSourceStepping::SourceSteppingDimension;

    addItem( ssd::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( ssd::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( ssd::VECTOR, "VECTOR", "Vector" );
    addItem( ssd::WELL, "WELL", RiaDefines::summaryWell() );
    addItem( ssd::WELL_COMPLETION_NUMBER, "WELL_COMPLETION", RiaDefines::summaryWellCompletion() );
    addItem( ssd::GROUP, "GROUP", RiaDefines::summaryWellGroup() );
    addItem( ssd::NETWORK, "NETWORK", RiaDefines::summaryNetwork() );
    addItem( ssd::REGION, "REGION", RiaDefines::summaryRegion() );
    addItem( ssd::BLOCK, "BLOCK", RiaDefines::summaryBlock() );
    addItem( ssd::AQUIFER, "AQUIFER", RiaDefines::summaryAquifer() );
    addItem( ssd::WELL_SEGMENT, "WELL_SEGMENT", RiaDefines::summaryWellSegment() );
    addItem( ssd::WELL_CONNECTION, "WELL_CONNECTION", RiaDefines::summaryWellConnection() );
    setDefault( ssd::VECTOR );
}
} // namespace caf

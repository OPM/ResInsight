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

#include "cafAppEnum.h"

namespace caf
{
template <>
void AppEnum<RimSummaryDataSourceStepping::SourceSteppingDimension>::setUp()
{
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR, "VECTOR", "Vector" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::WELL, "WELL", "Well" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP, "GROUP", "Group" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::REGION, "REGION", "Region" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK, "BLOCK", "Block" );
    addItem( RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER, "AQUIFER", "Aquifer" );
    setDefault( RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR );
}
} // namespace caf

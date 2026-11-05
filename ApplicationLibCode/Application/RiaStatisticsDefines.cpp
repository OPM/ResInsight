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

#include "RiaStatisticsDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::StatisticsType>::setUp()
{
    addItem( RiaDefines::StatisticsType::MIN, "MIN", "Minimum" );
    addItem( RiaDefines::StatisticsType::P90, "P90", "P90" );
    addItem( RiaDefines::StatisticsType::P50, "P50", "P50" );
    addItem( RiaDefines::StatisticsType::MEAN, "MEAN", "Mean" );
    addItem( RiaDefines::StatisticsType::P10, "P10", "P10" );
    addItem( RiaDefines::StatisticsType::MAX, "MAX", "Maximum" );
    setDefault( RiaDefines::StatisticsType::MEAN );
}
} // namespace caf

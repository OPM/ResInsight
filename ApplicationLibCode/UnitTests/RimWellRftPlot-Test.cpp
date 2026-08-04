/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimWellRftPlot.h"

#include <variant>

//--------------------------------------------------------------------------------------------------
/// A plot with no selected sources must not report a summary case or ensemble data source
//--------------------------------------------------------------------------------------------------
TEST( RimWellRftPlotTest, DataSourceWithoutSelectedSources )
{
    RimWellRftPlot plot;

    auto dataSource = plot.dataSource();

    EXPECT_TRUE( std::holds_alternative<std::monostate>( dataSource ) );
    EXPECT_EQ( nullptr, std::get_if<RimSummaryCase*>( &dataSource ) );
    EXPECT_EQ( nullptr, std::get_if<RimSummaryEnsemble*>( &dataSource ) );
}

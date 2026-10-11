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

#include "gtest/gtest.h"

#include "ContourMap/RimContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"

//--------------------------------------------------------------------------------------------------
/// A contour map view without an eclipse case must not crash when results are generated.
//--------------------------------------------------------------------------------------------------
TEST( RimEclipseContourMapProjection, GenerateResultsWithoutEclipseCase )
{
    RimEclipseContourMapView view;
    ASSERT_NE( nullptr, view.contourMapProjection() );

    view.contourMapProjection()->generateResultsIfNecessary( 0 );
}

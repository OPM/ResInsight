/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimRftTopologyCurve.h"

CAF_PDM_SOURCE_INIT( RimRftTopologyCurve, "RimRftTopologyCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTopologyCurve::RimRftTopologyCurve()
{
    CAF_PDM_InitObject( "RFT Topology Curve" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellName() const
{
    return "Topology curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellLogChannelUiName() const
{
    return "Topology curve channel";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellLogChannelUnits() const
{
    return "Topology curve units";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::createCurveAutoName()
{
    return "Topology curve auto name";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
}

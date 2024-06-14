/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RiaOsduDefines.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::osduFieldKind()
{
    return "osdu:wks:master-data--Field:1.0.0";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::osduWellKind()
{
    return "osdu:wks:master-data--Well:1.*.*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::osduWellboreKind()
{
    return "osdu:wks:master-data--Wellbore:1.*.*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::osduWellboreTrajectoryKind()
{
    return "osdu:wks:work-product-component--WellboreTrajectory:1.*.*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::osduWellLogKind()
{
    return "osdu:wks:work-product-component--WellLog:1.*.*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::contentTypeJson()
{
    return "application/json";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::contentTypeParquet()
{
    return "application/x-parquet";
}

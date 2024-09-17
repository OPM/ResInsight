/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RiaRftDefines.h"
#include "cafAppEnum.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isSegmentResult( const QString& resultName )
{
    return resultName.startsWith( "SEG" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentStartDepthResultName()
{
    return "SEGLENST";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentEndDepthResultName()
{
    return "SEGLENEN";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentTvdDepthResultName()
{
    return "SEGDEPTH";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentNumberResultName()
{
    return "SEGMENTNUMBER";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaDefines::isSegmentConnectionResult( const QString& resultName )
{
    return resultName.startsWith( "CON" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentConnectionTvdDepthResultName()
{
    return "CONDEPTH";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDefines::segmentConnectionEndDepthResultName()
{
    return "CONLENEN";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDefines::segmentConnectionStartDepthResultName()
{
    return "CONLENST";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDefines::segmentConnectionBranchNoResultName()
{
    return "CONBRNO";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentConnectionMeasuredDepthResultName()
{
    return "SegmentConnectionMeasuredDepth";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDefines::segmentConnectionIPos()
{
    return "CONIPOS";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDefines::segmentConnectionJPos()
{
    return "CONJPOS";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaDefines::segmentConnectionKPos()
{
    return "CONKPOS";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::allBranches()
{
    return "All";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::segmentBranchNumberResultName()
{
    return "SegmentBranchNumber";
}

namespace caf
{
template <>
void caf::AppEnum<RiaDefines::RftBranchType>::setUp()
{
    addItem( RiaDefines::RftBranchType::RFT_TUBING, "RFT_TUBING", "Tubing" );
    addItem( RiaDefines::RftBranchType::RFT_DEVICE, "RFT_DEVICE", "Device" );
    addItem( RiaDefines::RftBranchType::RFT_ANNULUS, "RFT_ANNULUS", "Annulus" );
    addItem( RiaDefines::RftBranchType::RFT_UNKNOWN, "RFT_UNKNOWN", "Unknown" );
    setDefault( RiaDefines::RftBranchType::RFT_TUBING );
}
} // namespace caf

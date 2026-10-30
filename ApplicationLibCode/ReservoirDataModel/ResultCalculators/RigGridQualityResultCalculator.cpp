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

#include "RigGridQualityResultCalculator.h"

#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridQualityResultCalculator::RigGridQualityResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridQualityResultCalculator::~RigGridQualityResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridQualityResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::riOilVolumeResultName() &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::QUALITY_CONTROL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
}

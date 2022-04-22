/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicSummaryCurveCalculatorUi.h"

#include "RiaSummaryTools.h"

#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"
#include "RimUserDefinedCalculation.h"
#include "RimUserDefinedCalculationCollection.h"

CAF_PDM_SOURCE_INIT( RicSummaryCurveCalculatorUi, "RicSummaryCurveCalculator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorUi::RicSummaryCurveCalculatorUi()
{
    CAF_PDM_InitObject( "RicSummaryCurveCalculator" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCalculatorUi::calculationsGroupName() const
{
    return "CalculatedSummariesGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCalculatorUi::calulationGroupName() const
{
    return "CalulationGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::notifyCalculatedNameChanged( int id, const QString& newName ) const
{
    RiaSummaryTools::notifyCalculatedCurveNameHasChanged( id, newName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationCollection* RicSummaryCurveCalculatorUi::calculationCollection() const
{
    return RimProject::current()->calculationCollection();
}

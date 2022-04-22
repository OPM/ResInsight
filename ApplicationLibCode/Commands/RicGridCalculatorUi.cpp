/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RicGridCalculatorUi.h"

#include "RimGridCalculationCollection.h"
#include "RimProject.h"
#include "RimUserDefinedCalculationCollection.h"

CAF_PDM_SOURCE_INIT( RicGridCalculatorUi, "RicGridCalculator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicGridCalculatorUi::RicGridCalculatorUi()
{
    CAF_PDM_InitObject( "RicGridCalculator" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicGridCalculatorUi::calculationsGroupName() const
{
    return "CalculationsGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicGridCalculatorUi::calulationGroupName() const
{
    return "CalulationGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicGridCalculatorUi::notifyCalculatedNameChanged( int id, const QString& newName ) const
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculationCollection* RicGridCalculatorUi::calculationCollection() const
{
    return RimProject::current()->gridCalculationCollection();
}

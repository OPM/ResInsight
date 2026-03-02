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

#include "RimReservoirGridEnsembleBase.h"

#include "RimCaseCollection.h"
#include "RimEclipseStatisticsCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimReservoirGridEnsembleBase::mainGrid()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimReservoirGridEnsembleBase::unionOfActiveCells( RiaDefines::PorosityModelType porosityType )
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReservoirGridEnsembleBase::computeUnionOfActiveCells()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimReservoirGridEnsembleBase::statisticsCaseCollection() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimEclipseCase*> RimReservoirGridEnsembleBase::casesInViews() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimReservoirGridEnsembleBase::activeFormationNames() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimReservoirGridEnsembleBase::createAndAppendStatisticsCase()
{
    auto* statsColl = statisticsCaseCollection();
    if ( !statsColl ) return nullptr;

    RimEclipseStatisticsCase* newStatisticsCase = new RimEclipseStatisticsCase;

    newStatisticsCase->setCaseUserDescription( QString( "Statistics " ) + QString::number( statsColl->reservoirs.size() + 1 ) );
    statsColl->reservoirs.push_back( newStatisticsCase );

    newStatisticsCase->populateResultSelectionAfterLoadingGrid();

    auto cases = sourceCases();
    if ( !cases.empty() )
    {
        newStatisticsCase->setWellDataSourceCase( cases.front() );
    }

    newStatisticsCase->openEclipseGridFile();
    newStatisticsCase->computeActiveCellsBoundingBox();
    newStatisticsCase->selectAllTimeSteps();

    return newStatisticsCase;
}

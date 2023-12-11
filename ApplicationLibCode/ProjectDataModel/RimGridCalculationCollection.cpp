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

#include "RimGridCalculationCollection.h"

#include "RiaLogging.h"
#include "RigEclipseResultAddress.h"
#include "RimGridCalculation.h"

#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimGridCalculationCollection, "RimGridCalculationCollection" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationCollection::RimGridCalculationCollection()
{
    CAF_PDM_InitObject( "Calculation Collection", ":/chain.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation* RimGridCalculationCollection::createCalculation() const
{
    return new RimGridCalculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridCalculation*> RimGridCalculationCollection::sortedGridCalculations() const
{
    std::vector<RimGridCalculation*> sortedCalculations;
    for ( auto userCalculation : calculations() )
    {
        auto gridCalculation = dynamic_cast<RimGridCalculation*>( userCalculation );
        if ( gridCalculation ) sortedCalculations.emplace_back( gridCalculation );
    }

    // Check if source calculation is depending on other. Will check one level dependency.
    auto isSourceDependingOnOther = []( const RimGridCalculation* source, const RimGridCalculation* other ) -> bool
    {
        auto outputCases = source->outputEclipseCases();
        auto outputAdr   = source->outputAddress();

        for ( auto v : other->allVariables() )
        {
            auto gridVariable = dynamic_cast<RimGridCalculationVariable*>( v );
            if ( std::find( outputCases.begin(), outputCases.end(), gridVariable->eclipseCase() ) != outputCases.end() &&
                 outputAdr.resultCatType() == gridVariable->resultCategoryType() && outputAdr.resultName() == gridVariable->resultVariable() )
            {
                return true;
            }
        }

        return false;
    };

    for ( auto source : sortedCalculations )
    {
        for ( auto other : sortedCalculations )
        {
            if ( source == other ) continue;

            if ( isSourceDependingOnOther( source, other ) && isSourceDependingOnOther( other, source ) )
            {
                QString txt = "Detected circular dependency between " + source->description() + " and " + other->description();
                RiaLogging::error( txt );

                return sortedCalculations;
            }
        }
    }

    std::sort( sortedCalculations.begin(), sortedCalculations.end(), isSourceDependingOnOther );

    return sortedCalculations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridCalculation*> RimGridCalculationCollection::dependentCalculations( RimGridCalculation* sourceCalculation ) const
{
    // Find all dependent grid calculations recursively. The ordering of calculations is least dependent first.

    std::vector<RimGridCalculation*> calculations;

    if ( !dependentCalculationsRecursively( sourceCalculation, calculations ) ) return {};

    std::reverse( calculations.begin(), calculations.end() );

    return calculations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationCollection::rebuildCaseMetaData()
{
    ensureValidCalculationIds();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculation* RimGridCalculationCollection::findCalculation( const QString& calculationName ) const
{
    for ( auto userCalculation : calculations() )
    {
        auto gridCalculation = dynamic_cast<RimGridCalculation*>( userCalculation );
        if ( gridCalculation && gridCalculation->shortName() == calculationName ) return gridCalculation;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCalculationCollection::dependentCalculationsRecursively( RimGridCalculation*               sourceCalculation,
                                                                     std::vector<RimGridCalculation*>& calculations ) const
{
    if ( std::find( calculations.begin(), calculations.end(), sourceCalculation ) != calculations.end() )
    {
        RiaLogging::error( "Detected circular dependency for " + sourceCalculation->description() );
        return false;
    }

    calculations.push_back( sourceCalculation );

    for ( auto v : sourceCalculation->allVariables() )
    {
        auto gridVariable = dynamic_cast<RimGridCalculationVariable*>( v );
        if ( gridVariable->resultCategoryType() == RiaDefines::ResultCatType::GENERATED )
        {
            if ( auto other = findCalculation( gridVariable->resultVariable() ) )
            {
                if ( !dependentCalculationsRecursively( other, calculations ) ) return false;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationCollection::initAfterRead()
{
    rebuildCaseMetaData();
}

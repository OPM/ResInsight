/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimSaturationPressurePlotCollection.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"

CAF_PDM_SOURCE_INIT( RimSaturationPressurePlotCollection, "RimSaturationPressurePlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlotCollection::RimSaturationPressurePlotCollection()
{
    CAF_PDM_InitObject( "Saturation Pressure Plots", ":/SummaryXPlotsLight16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_saturationPressurePlots, "SaturationPressurePlots", "Saturation Pressure Plots", "", "", "" );
    m_saturationPressurePlots.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSaturationPressurePlotCollection::~RimSaturationPressurePlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSaturationPressurePlot*>
    RimSaturationPressurePlotCollection::createSaturationPressurePlots( RimEclipseResultCase* eclipseResultCase,
                                                                        int                   timeStep )
{
    std::vector<RimSaturationPressurePlot*> generatedPlots;

    if ( !eclipseResultCase ) return generatedPlots;

    RigEclipseCaseData* eclipseCaseData = eclipseResultCase->eclipseCaseData();
    if ( !eclipseCaseData ) return generatedPlots;

    auto results = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    std::set<int> eqlnumRegionIdsFound;
    {
        RigEclipseResultAddress resAdr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::eqlnumResultName() );
        if ( results->hasResultEntry( resAdr ) )
        {
            results->ensureKnownResultLoaded( resAdr );

            auto vals = results->uniqueCellScalarValues( resAdr );
            for ( auto v : vals )
            {
                eqlnumRegionIdsFound.insert( v );
            }
        }
    }

    eclipseResultCase->ensureDeckIsParsedForEquilData();
    std::vector<RigEquil> equilData = eclipseCaseData->equilData();
    for ( size_t i = 0; i < equilData.size(); i++ )
    {
        int zeroBasedEquilibriumRegion = static_cast<int>( i );

        if ( eqlnumRegionIdsFound.find( zeroBasedEquilibriumRegion + 1 ) != eqlnumRegionIdsFound.end() )
        {
            RimSaturationPressurePlot* plot = new RimSaturationPressurePlot();
            plot->setAsPlotMdiWindow();

            // As discussed with Liv Merete, it is not any use for creation of different plots for matrix/fracture. For
            // now, use hardcoded value for MATRIX
            plot->assignCaseAndEquilibriumRegion( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                  eclipseResultCase,
                                                  zeroBasedEquilibriumRegion,
                                                  timeStep );

            m_saturationPressurePlots.push_back( plot );

            generatedPlots.push_back( plot );
        }
    }

    return generatedPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSaturationPressurePlot*> RimSaturationPressurePlotCollection::plots() const
{
    return m_saturationPressurePlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSaturationPressurePlotCollection::plotCount() const
{
    return m_saturationPressurePlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlotCollection::deleteAllPlots()
{
    m_saturationPressurePlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSaturationPressurePlotCollection::loadDataAndUpdateAllPlots()
{
    for ( const auto& p : m_saturationPressurePlots )
        p->loadDataAndUpdate();
}

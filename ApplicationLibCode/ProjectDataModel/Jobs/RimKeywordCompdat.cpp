/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimKeywordCompdat.h"

#include "Commands/CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "RifOpmDeckTools.h"

#include "RimEclipseCase.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"

CAF_PDM_SOURCE_INIT( RimKeywordCompdat, "RimKeywordCompdat" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordCompdat::RimKeywordCompdat()
{
    CAF_PDM_InitObject( "COMPDAT Keyword" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordCompdat::~RimKeywordCompdat()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordCompdat::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordCompdat::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword RimKeywordCompdat::keyword()
{
    if ( m_eclipseCase() == nullptr || m_wellPath() == nullptr )
    {
        return Opm::DeckKeyword();
    }

    auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( m_wellPath(), m_eclipseCase() );

    using C = Opm::ParserKeywords::COMPDAT;

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::COMPDAT() ) );

    for ( auto& cd : compdata )
    {
        std::vector<Opm::DeckItem> items;

        // items.push_back( RifOpmDeckTools::item( W::WELL::itemName, wellName ) );
        // items.push_back( RifOpmDeckTools::item( W::GROUP::itemName, groupName ) );
        // items.push_back( RifOpmDeckTools::item( W::HEAD_I::itemName, ijPos.second.x() + 1 ) );
        // items.push_back( RifOpmDeckTools::item( W::HEAD_J::itemName, ijPos.second.y() + 1 ) );

        // auto refDepth = compSettings->referenceDepth();
        // items.push_back( refDepth.has_value() ? RifOpmDeckTools::item( W::REF_DEPTH::itemName, refDepth.value() )
        //                                       : RifOpmDeckTools::defaultItem( W::REF_DEPTH::itemName ) );

        // items.push_back( RifOpmDeckTools::item( W::PHASE::itemName, compSettings->wellTypeNameForExport().toStdString() ) );

        // auto dRadius = compSettings->drainageRadius();
        // items.push_back( dRadius.has_value() ? RifOpmDeckTools::item( W::D_RADIUS::itemName, dRadius.value() )
        //                                      : RifOpmDeckTools::defaultItem( W::D_RADIUS::itemName ) );

        // items.push_back( RifOpmDeckTools::item( W::INFLOW_EQ::itemName, compSettings->gasInflowEquationForExport().toStdString() ) );
        // items.push_back( RifOpmDeckTools::item( W::AUTO_SHUTIN::itemName, compSettings->automaticWellShutInForExport().toStdString() ) );
        // items.push_back( RifOpmDeckTools::item( W::CROSSFLOW::itemName, compSettings->allowWellCrossFlowForExport().toStdString() ) );
        // items.push_back( RifOpmDeckTools::item( W::P_TABLE::itemName, compSettings->wellBoreFluidPVT() ) );
        // items.push_back( RifOpmDeckTools::item( W::DENSITY_CALC::itemName, compSettings->hydrostaticDensityForExport().toStdString() ) );
        // items.push_back( RifOpmDeckTools::item( W::FIP_REGION::itemName, compSettings->fluidInPlaceRegion() ) );

        // compDat->set_well_name( inputData.wellName().toStdString() );

        //// Convert to 1-based indexing
        // compDat->set_grid_i( inputData.completionDataGridCell().localCellIndexI() + 1 );
        // compDat->set_grid_j( inputData.completionDataGridCell().localCellIndexJ() + 1 );
        // compDat->set_upper_k( inputData.completionDataGridCell().localCellIndexK() + 1 );
        // compDat->set_lower_k( inputData.completionDataGridCell().localCellIndexK() + 1 );

        // compDat->set_open_shut_flag( "OPEN" );
        // if ( inputData.saturation() != inputData.defaultValue() )
        //{
        //     compDat->set_saturation( inputData.saturation() );
        // }
        // if ( inputData.transmissibility() != inputData.defaultValue() )
        //{
        //     compDat->set_transmissibility( inputData.transmissibility() );
        // }
        // if ( inputData.diameter() != inputData.defaultValue() )
        //{
        //     compDat->set_diameter( inputData.diameter() );
        // }
        // if ( inputData.kh() != inputData.defaultValue() )
        //{
        //     compDat->set_kh( inputData.kh() );
        // }
        // if ( inputData.skinFactor() != inputData.defaultValue() )
        //{
        //     compDat->set_skin_factor( inputData.skinFactor() );
        // }
        // if ( inputData.dFactor() != inputData.defaultValue() )
        //{
        //     compDat->set_d_factor( inputData.dFactor() );
        // }
        // compDat->set_direction( inputData.directionStringXYZ().toStdString() );
        // if ( inputData.startMD().has_value() )
        //{
        //     compDat->set_start_md( inputData.startMD().value() );
        //     compDat->set_end_md( inputData.endMD().value() );
        // }

        kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return kw;
}

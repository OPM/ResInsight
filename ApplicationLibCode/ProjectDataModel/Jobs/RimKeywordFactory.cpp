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

#include "RimKeywordFactory.h"

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
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

//==================================================================================================
///
///
//==================================================================================================
namespace RimKeywordFactory
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword welspecsKeyword( const std::string wellGrpName, RimEclipseCase* eCase, RimWellPath* wellPath )
{
    if ( eCase == nullptr || wellPath == nullptr || wellPath->completionSettings() == nullptr )
    {
        return Opm::DeckKeyword();
    }

    auto ijPos        = RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( eCase, wellPath );
    auto compSettings = wellPath->completionSettings();

    auto wellName = compSettings->wellNameForExport().toStdString();

    using W = Opm::ParserKeywords::WELSPECS;

    std::vector<Opm::DeckItem> items;

    items.push_back( RifOpmDeckTools::item( W::WELL::itemName, wellName ) );
    items.push_back( RifOpmDeckTools::item( W::GROUP::itemName, wellGrpName ) );
    items.push_back( RifOpmDeckTools::item( W::HEAD_I::itemName, ijPos.second.x() + 1 ) );
    items.push_back( RifOpmDeckTools::item( W::HEAD_J::itemName, ijPos.second.y() + 1 ) );

    auto refDepth = compSettings->referenceDepth();
    items.push_back( refDepth.has_value() ? RifOpmDeckTools::item( W::REF_DEPTH::itemName, refDepth.value() )
                                          : RifOpmDeckTools::defaultItem( W::REF_DEPTH::itemName ) );

    items.push_back( RifOpmDeckTools::item( W::PHASE::itemName, compSettings->wellTypeNameForExport().toStdString() ) );

    auto dRadius = compSettings->drainageRadius();
    items.push_back( dRadius.has_value() ? RifOpmDeckTools::item( W::D_RADIUS::itemName, dRadius.value() )
                                         : RifOpmDeckTools::defaultItem( W::D_RADIUS::itemName ) );

    items.push_back( RifOpmDeckTools::item( W::INFLOW_EQ::itemName, compSettings->gasInflowEquationForExport().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::AUTO_SHUTIN::itemName, compSettings->automaticWellShutInForExport().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::CROSSFLOW::itemName, compSettings->allowWellCrossFlowForExport().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::P_TABLE::itemName, compSettings->wellBoreFluidPVT() ) );
    items.push_back( RifOpmDeckTools::item( W::DENSITY_CALC::itemName, compSettings->hydrostaticDensityForExport().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::FIP_REGION::itemName, compSettings->fluidInPlaceRegion() ) );

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::WELSPECS() ) );
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    return kw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword compdatKeyword( RimEclipseCase* eCase, RimWellPath* wellPath )
{
    if ( eCase == nullptr || wellPath == nullptr || wellPath->completionSettings() == nullptr )
    {
        return Opm::DeckKeyword();
    }

    auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( wellPath, eCase );

    using C = Opm::ParserKeywords::COMPDAT;

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::COMPDAT() ) );

    auto wellName = wellPath->completionSettings()->wellNameForExport().toStdString();

    for ( auto& cd : compdata )
    {
        std::vector<Opm::DeckItem> items;

        items.push_back( RifOpmDeckTools::item( C::WELL::itemName, wellName ) );
        items.push_back( RifOpmDeckTools::item( C::I::itemName, cd.completionDataGridCell().localCellIndexI() + 1 ) );
        items.push_back( RifOpmDeckTools::item( C::J::itemName, cd.completionDataGridCell().localCellIndexJ() + 1 ) );
        items.push_back( RifOpmDeckTools::item( C::K1::itemName, cd.completionDataGridCell().localCellIndexK() + 1 ) );
        items.push_back( RifOpmDeckTools::item( C::K2::itemName, cd.completionDataGridCell().localCellIndexK() + 1 ) );
        items.push_back( RifOpmDeckTools::item( C::STATE::itemName, "OPEN" ) );

        auto satTable = cd.saturation();
        items.push_back( satTable != cd.defaultValue() ? RifOpmDeckTools::item( C::SAT_TABLE::itemName, satTable )
                                                       : RifOpmDeckTools::defaultItem( C::SAT_TABLE::itemName ) );

        auto transmissibility = cd.transmissibility();
        items.push_back( transmissibility != cd.defaultValue()
                             ? RifOpmDeckTools::item( C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName, transmissibility )
                             : RifOpmDeckTools::defaultItem( C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName ) );

        auto diameter = cd.diameter();
        items.push_back( diameter != cd.defaultValue() ? RifOpmDeckTools::item( C::DIAMETER::itemName, diameter )
                                                       : RifOpmDeckTools::defaultItem( C::DIAMETER::itemName ) );
        auto kh = cd.kh();
        items.push_back( kh != cd.defaultValue() ? RifOpmDeckTools::item( C::Kh::itemName, kh )
                                                 : RifOpmDeckTools::defaultItem( C::Kh::itemName ) );
        auto skinFactor = cd.skinFactor();
        items.push_back( skinFactor != cd.defaultValue() ? RifOpmDeckTools::item( C::SKIN::itemName, skinFactor )
                                                         : RifOpmDeckTools::defaultItem( C::SKIN::itemName ) );
        auto dFactor = cd.dFactor();
        items.push_back( dFactor != cd.defaultValue() ? RifOpmDeckTools::item( C::D_FACTOR::itemName, dFactor )
                                                      : RifOpmDeckTools::defaultItem( C::D_FACTOR::itemName ) );

        items.push_back( RifOpmDeckTools::item( C::DIR::itemName, cd.directionStringXYZ().toStdString() ) );

        kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return kw;
}

} // namespace RimKeywordFactory

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

#include "RiaResultNames.h"

#include "RifEclipseInputFileTools.h"
#include "RifOpmDeckTools.h"

#include "RigFault.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include "cvfStructGrid.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/F.hpp"
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword faultsKeyword( const RigMainGrid* mainGrid, const cvf::Vec3st& min, const cvf::Vec3st& max, const cvf::Vec3st& refinement )
{
    if ( mainGrid == nullptr )
    {
        return Opm::DeckKeyword();
    }

    // Helper lambda to convert FaceType to Eclipse face string
    auto faceTypeToString = []( cvf::StructGridInterface::FaceType faceType ) -> std::string
    {
        switch ( faceType )
        {
            case cvf::StructGridInterface::POS_I:
                return "I";
            case cvf::StructGridInterface::NEG_I:
                return "I-";
            case cvf::StructGridInterface::POS_J:
                return "J";
            case cvf::StructGridInterface::NEG_J:
                return "J-";
            case cvf::StructGridInterface::POS_K:
                return "K";
            case cvf::StructGridInterface::NEG_K:
                return "K-";
            default:
                return "";
        }
    };

    using F = Opm::ParserKeywords::FAULTS;

    Opm::DeckKeyword kw{ Opm::ParserKeywords::FAULTS() };

    // Process all faults in the grid
    const cvf::Collection<RigFault>& faults = mainGrid->faults();
    for ( const auto& fault : faults )
    {
        // Skip undefined faults
        if ( fault->name() == RiaResultNames::undefinedGridFaultName() || fault->name() == RiaResultNames::undefinedGridFaultWithInactiveName() )
        {
            continue;
        }

        // Extract fault cell and face data for this fault
        std::vector<RigFault::CellAndFace> faultCellAndFaces =
            RifEclipseInputFileTools::extractFaults( mainGrid, fault->faultFaces(), min, max, refinement );

        // Group consecutive cells in K direction and create fault records
        size_t                             lastI        = std::numeric_limits<size_t>::max();
        size_t                             lastJ        = std::numeric_limits<size_t>::max();
        size_t                             lastK        = std::numeric_limits<size_t>::max();
        size_t                             startK       = std::numeric_limits<size_t>::max();
        cvf::StructGridInterface::FaceType lastFaceType = cvf::StructGridInterface::FaceType::NO_FACE;

        for ( const RigFault::CellAndFace& faultCellAndFace : faultCellAndFaces )
        {
            size_t                             i, j, k;
            cvf::StructGridInterface::FaceType faceType;
            std::tie( i, j, k, faceType ) = faultCellAndFace;

            // Check if we need to write out the previous range
            if ( i != lastI || j != lastJ || lastFaceType != faceType || k != lastK + 1 )
            {
                // Write out previous fault line if valid
                if ( lastFaceType != cvf::StructGridInterface::FaceType::NO_FACE )
                {
                    // Convert from 0-based to 1-based Eclipse indexing
                    int i1 = static_cast<int>( lastI ) + 1;
                    int j1 = static_cast<int>( lastJ ) + 1;
                    int k1 = static_cast<int>( startK ) + 1;
                    int k2 = static_cast<int>( lastK ) + 1;

                    std::string faceStr = faceTypeToString( lastFaceType );

                    // Create a record for this fault line
                    std::vector<Opm::DeckItem> items;
                    items.push_back( RifOpmDeckTools::item( F::NAME::itemName, fault->name().toStdString() ) );
                    items.push_back( RifOpmDeckTools::item( F::IX1::itemName, i1 ) );
                    items.push_back( RifOpmDeckTools::item( F::IX2::itemName, i1 ) );
                    items.push_back( RifOpmDeckTools::item( F::IY1::itemName, j1 ) );
                    items.push_back( RifOpmDeckTools::item( F::IY2::itemName, j1 ) );
                    items.push_back( RifOpmDeckTools::item( F::IZ1::itemName, k1 ) );
                    items.push_back( RifOpmDeckTools::item( F::IZ2::itemName, k2 ) );
                    items.push_back( RifOpmDeckTools::item( F::FACE::itemName, faceStr ) );

                    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
                }

                // Start new range
                lastI        = i;
                lastJ        = j;
                lastK        = k;
                lastFaceType = faceType;
                startK       = k;
            }
            else
            {
                // Continue current range
                lastK = k;
            }
        }

        // Write out final fault line for this fault if valid
        if ( lastFaceType != cvf::StructGridInterface::FaceType::NO_FACE )
        {
            // Convert from 0-based to 1-based Eclipse indexing
            int i1 = static_cast<int>( lastI ) + 1;
            int j1 = static_cast<int>( lastJ ) + 1;
            int k1 = static_cast<int>( startK ) + 1;
            int k2 = static_cast<int>( lastK ) + 1;

            std::string faceStr = faceTypeToString( lastFaceType );

            // Create a record for this fault line
            std::vector<Opm::DeckItem> items;
            items.push_back( RifOpmDeckTools::item( F::NAME::itemName, fault->name().toStdString() ) );
            items.push_back( RifOpmDeckTools::item( F::IX1::itemName, i1 ) );
            items.push_back( RifOpmDeckTools::item( F::IX2::itemName, i1 ) );
            items.push_back( RifOpmDeckTools::item( F::IY1::itemName, j1 ) );
            items.push_back( RifOpmDeckTools::item( F::IY2::itemName, j1 ) );
            items.push_back( RifOpmDeckTools::item( F::IZ1::itemName, k1 ) );
            items.push_back( RifOpmDeckTools::item( F::IZ2::itemName, k2 ) );
            items.push_back( RifOpmDeckTools::item( F::FACE::itemName, faceStr ) );

            kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
        }
    }

    return kw;
}

} // namespace RimKeywordFactory

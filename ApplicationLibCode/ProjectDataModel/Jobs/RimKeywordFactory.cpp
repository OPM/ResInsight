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

#include "CompletionsMsw/RigMswTableData.h"
#include "RigEclipseResultTools.h"
#include "RigFault.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

#include "cvfStructGrid.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/F.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/O.hpp"
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
    if ( eCase == nullptr || wellPath == nullptr || wellPath->completionSettings() == nullptr || eCase->eclipseCaseData() == nullptr )
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
    if ( eCase == nullptr || wellPath == nullptr || wellPath->completionSettings() == nullptr || eCase->eclipseCaseData() == nullptr )
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
        items.push_back( satTable != RigCompletionData::defaultValue() ? RifOpmDeckTools::item( C::SAT_TABLE::itemName, satTable )
                                                                       : RifOpmDeckTools::defaultItem( C::SAT_TABLE::itemName ) );

        auto transmissibility = cd.transmissibility();
        items.push_back( transmissibility != RigCompletionData::defaultValue()
                             ? RifOpmDeckTools::item( C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName, transmissibility )
                             : RifOpmDeckTools::defaultItem( C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName ) );

        auto diameter = cd.diameter();
        items.push_back( diameter != RigCompletionData::defaultValue() ? RifOpmDeckTools::item( C::DIAMETER::itemName, diameter )
                                                                       : RifOpmDeckTools::defaultItem( C::DIAMETER::itemName ) );
        auto kh = cd.kh();
        items.push_back( kh != RigCompletionData::defaultValue() ? RifOpmDeckTools::item( C::Kh::itemName, kh )
                                                                 : RifOpmDeckTools::defaultItem( C::Kh::itemName ) );
        auto skinFactor = cd.skinFactor();
        items.push_back( skinFactor != RigCompletionData::defaultValue() ? RifOpmDeckTools::item( C::SKIN::itemName, skinFactor )
                                                                         : RifOpmDeckTools::defaultItem( C::SKIN::itemName ) );
        auto dFactor = cd.dFactor();
        items.push_back( dFactor != RigCompletionData::defaultValue() ? RifOpmDeckTools::item( C::D_FACTOR::itemName, dFactor )
                                                                      : RifOpmDeckTools::defaultItem( C::D_FACTOR::itemName ) );

        items.push_back( RifOpmDeckTools::item( C::DIR::itemName, cd.directionStringXYZ().toStdString() ) );

        kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return kw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword welsegsKeyword( const RigMswTableData& mswData )
{
    if ( !mswData.hasWelsegsData() )
    {
        return Opm::DeckKeyword();
    }

    Opm::DeckKeyword newKw( ( Opm::ParserKeywords::WELSEGS() ) );

    // welsegs header row
    auto&                      header = mswData.welsegsHeader();
    std::vector<Opm::DeckItem> headerItems;
    headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::WELL::itemName, header.well ) );
    headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::TOP_DEPTH::itemName, header.topDepth ) );
    headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::TOP_LENGTH::itemName, header.topLength ) );

    if ( header.wellboreVolume.has_value() )
        headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::WELLBORE_VOLUME::itemName, header.wellboreVolume.value() ) );
    else
        headerItems.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WELSEGS::WELLBORE_VOLUME::itemName ) );

    headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::INFO_TYPE::itemName, header.infoType ) );

    if ( header.pressureComponents.has_value() )
        headerItems.push_back(
            RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::PRESSURE_COMPONENTS::itemName, header.pressureComponents.value() ) );
    else
        headerItems.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WELSEGS::PRESSURE_COMPONENTS::itemName ) );
    if ( header.flowModel.has_value() )
        headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::FLOW_MODEL::itemName, header.flowModel.value() ) );
    else
        headerItems.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WELSEGS::FLOW_MODEL::itemName ) );

    newKw.addRecord( Opm::DeckRecord{ std::move( headerItems ) } );

    // welsegs data rows
    for ( auto& wsRow : mswData.welsegsData() )
    {
        std::vector<Opm::DeckItem> items;
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::SEGMENT1::itemName, wsRow.segment1 ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::SEGMENT2::itemName, wsRow.segment2 ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::BRANCH::itemName, wsRow.branch ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::JOIN_SEGMENT::itemName, wsRow.joinSegment ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::LENGTH::itemName, wsRow.length ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::DEPTH::itemName, wsRow.depth ) );
        if ( wsRow.diameter.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::DIAMETER::itemName, wsRow.diameter.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WELSEGS::DIAMETER::itemName ) );
        if ( wsRow.roughness.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WELSEGS::ROUGHNESS::itemName, wsRow.roughness.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WELSEGS::ROUGHNESS::itemName ) );

        newKw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return newKw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword compsegsKeyword( const RigMswTableData& mswData )
{
    if ( !mswData.hasCompsegsData() )
    {
        return Opm::DeckKeyword();
    }

    Opm::DeckKeyword newKw( ( Opm::ParserKeywords::COMPSEGS() ) );

    // header row
    std::vector<Opm::DeckItem> headerItems;
    headerItems.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::WELL::itemName, mswData.wellName() ) );
    newKw.addRecord( Opm::DeckRecord{ std::move( headerItems ) } );

    // data rows
    for ( auto& csRow : mswData.compsegsData() )
    {
        std::vector<Opm::DeckItem> items;
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::I::itemName, csRow.i ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::J::itemName, csRow.j ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::K::itemName, csRow.k ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::BRANCH::itemName, csRow.branch ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::DISTANCE_START::itemName, csRow.distanceStart ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::COMPSEGS::DISTANCE_END::itemName, csRow.distanceEnd ) );

        newKw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return newKw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword wsegvalvKeyword( const RigMswTableData& mswData )
{
    if ( !mswData.hasWsegvalvData() )
    {
        return Opm::DeckKeyword();
    }

    Opm::DeckKeyword newKw( ( Opm::ParserKeywords::WSEGVALV() ) );

    for ( auto& wvRow : mswData.wsegvalvData() )
    {
        std::vector<Opm::DeckItem> items;

        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::WELL::itemName, wvRow.well ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::SEGMENT_NUMBER::itemName, wvRow.segmentNumber ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::CV::itemName, wvRow.cv ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::AREA::itemName, wvRow.area ) );
        if ( wvRow.extraLength.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::EXTRA_LENGTH::itemName, wvRow.extraLength.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGVALV::EXTRA_LENGTH::itemName ) );
        if ( wvRow.pipeD.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::PIPE_D::itemName, wvRow.pipeD.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGVALV::PIPE_D::itemName ) );
        if ( wvRow.roughness.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::ROUGHNESS::itemName, wvRow.roughness.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGVALV::ROUGHNESS::itemName ) );
        if ( wvRow.pipeA.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::PIPE_A::itemName, wvRow.pipeA.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGVALV::PIPE_A::itemName ) );
        if ( wvRow.status.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::STATUS::itemName, wvRow.status.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGVALV::STATUS::itemName ) );
        if ( wvRow.maxA.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGVALV::MAX_A::itemName, wvRow.maxA.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGVALV::MAX_A::itemName ) );

        newKw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return newKw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword wsegaicdKeyword( const RigMswTableData& mswData )
{
    if ( !mswData.hasWsegaicdData() )
    {
        return Opm::DeckKeyword();
    }

    Opm::DeckKeyword newKw( ( Opm::ParserKeywords::WSEGAICD() ) );

    for ( auto& waRow : mswData.wsegaicdData() )
    {
        std::vector<Opm::DeckItem> items;

        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::WELL::itemName, waRow.well ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::SEGMENT1::itemName, waRow.segment1 ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::SEGMENT2::itemName, waRow.segment2 ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::STRENGTH::itemName, waRow.strength ) );

        if ( waRow.length.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::LENGTH::itemName, waRow.length.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::LENGTH::itemName ) );
        if ( waRow.densityCali.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::DENSITY_CALI::itemName, waRow.densityCali.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::DENSITY_CALI::itemName ) );
        if ( waRow.viscosityCali.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::VISCOSITY_CALI::itemName, waRow.viscosityCali.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::VISCOSITY_CALI::itemName ) );
        if ( waRow.criticalValue.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::CRITICAL_VALUE::itemName, waRow.criticalValue.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::CRITICAL_VALUE::itemName ) );
        if ( waRow.widthTrans.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::WIDTH_TRANS::itemName, waRow.widthTrans.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::WIDTH_TRANS::itemName ) );
        if ( waRow.maxViscRatio.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::MAX_VISC_RATIO::itemName, waRow.maxViscRatio.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::MAX_VISC_RATIO::itemName ) );
        if ( waRow.methodScalingFactor.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::METHOD_SCALING_FACTOR::itemName,
                                                    waRow.methodScalingFactor.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::METHOD_SCALING_FACTOR::itemName ) );

        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::MAX_ABS_RATE::itemName, waRow.maxAbsRate ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::FLOW_RATE_EXPONENT::itemName, waRow.flowRateExponent ) );
        items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::VISC_EXPONENT::itemName, waRow.viscExponent ) );

        if ( waRow.status.has_value() )
            items.push_back( RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::STATUS::itemName, waRow.status.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::STATUS::itemName ) );
        if ( waRow.oilFlowFraction.has_value() )
            items.push_back(
                RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::OIL_FLOW_FRACTION::itemName, waRow.oilFlowFraction.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::OIL_FLOW_FRACTION::itemName ) );
        if ( waRow.waterFlowFraction.has_value() )
            items.push_back(
                RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::WATER_FLOW_FRACTION::itemName, waRow.waterFlowFraction.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::WATER_FLOW_FRACTION::itemName ) );
        if ( waRow.gasFlowFraction.has_value() )
            items.push_back(
                RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::GAS_FLOW_FRACTION::itemName, waRow.gasFlowFraction.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::GAS_FLOW_FRACTION::itemName ) );
        if ( waRow.oilViscFraction.has_value() )
            items.push_back(
                RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::OIL_VISC_FRACTION::itemName, waRow.oilViscFraction.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::OIL_VISC_FRACTION::itemName ) );
        if ( waRow.waterViscFraction.has_value() )
            items.push_back(
                RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::WATER_VISC_FRACTION::itemName, waRow.waterViscFraction.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::WATER_VISC_FRACTION::itemName ) );
        if ( waRow.gasViscFraction.has_value() )
            items.push_back(
                RifOpmDeckTools::item( Opm::ParserKeywords::WSEGAICD::GAS_VISC_FRACTION::itemName, waRow.gasViscFraction.value() ) );
        else
            items.push_back( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::WSEGAICD::GAS_VISC_FRACTION::itemName ) );

        newKw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return newKw;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword bcconKeyword( const std::vector<RigEclipseResultTools::BorderCellFace>& borderCellFaces )
{
    if ( borderCellFaces.empty() )
    {
        return Opm::DeckKeyword();
    }

    // Helper lambda to convert FaceType to Eclipse face string
    auto faceTypeToString = []( cvf::StructGridInterface::FaceType faceType ) -> std::string
    {
        switch ( faceType )
        {
            case cvf::StructGridInterface::POS_I:
                return "X";
            case cvf::StructGridInterface::NEG_I:
                return "X-";
            case cvf::StructGridInterface::POS_J:
                return "Y";
            case cvf::StructGridInterface::NEG_J:
                return "Y-";
            case cvf::StructGridInterface::POS_K:
                return "Z";
            case cvf::StructGridInterface::NEG_K:
                return "Z-";
            default:
                return "";
        }
    };

    using B = Opm::ParserKeywords::BCCON;

    Opm::DeckKeyword kw{ Opm::ParserKeywords::BCCON() };

    for ( const auto& borderFace : borderCellFaces )
    {
        // Convert from 0-based to 1-based Eclipse indexing
        int i1 = static_cast<int>( borderFace.ijk[0] ) + 1;
        int j1 = static_cast<int>( borderFace.ijk[1] ) + 1;
        int k1 = static_cast<int>( borderFace.ijk[2] ) + 1;

        std::string faceStr = faceTypeToString( borderFace.faceType );

        // Create items for the record
        std::vector<Opm::DeckItem> items;

        items.push_back( RifOpmDeckTools::item( B::INDEX::itemName, borderFace.boundaryCondition ) );
        items.push_back( RifOpmDeckTools::item( B::I1::itemName, i1 ) );
        items.push_back( RifOpmDeckTools::item( B::I2::itemName, i1 ) );
        items.push_back( RifOpmDeckTools::item( B::J1::itemName, j1 ) );
        items.push_back( RifOpmDeckTools::item( B::J2::itemName, j1 ) );
        items.push_back( RifOpmDeckTools::item( B::K1::itemName, k1 ) );
        items.push_back( RifOpmDeckTools::item( B::K2::itemName, k1 ) );
        items.push_back( RifOpmDeckTools::item( B::DIRECTION::itemName, faceStr ) );

        kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
    }

    return kw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword bcpropKeyword( const std::vector<RigEclipseResultTools::BorderCellFace>& boundaryConditions,
                                const std::vector<Opm::DeckRecord>&                       boundaryConditionProperties )
{
    if ( boundaryConditions.empty() )
    {
        return Opm::DeckKeyword();
    }

    using B = Opm::ParserKeywords::BCPROP;

    Opm::DeckKeyword kw{ Opm::ParserKeywords::BCPROP() };

    // Add one entry per boundary condition
    for ( const auto& bc : boundaryConditions )
    {
        if ( bc.boundaryCondition <= 0 ) continue; // Skip entries without a valid boundary condition

        // Find the corresponding property record
        // The properties vector should be indexed by boundaryCondition - 1
        size_t propIndex = static_cast<size_t>( bc.boundaryCondition - 1 );
        if ( propIndex < boundaryConditionProperties.size() )
        {
            const auto& propRecord = boundaryConditionProperties[propIndex];

            // Create a new record with the boundary condition INDEX
            std::vector<Opm::DeckItem> items;

            // Add INDEX field
            items.push_back( RifOpmDeckTools::item( B::INDEX::itemName, bc.boundaryCondition ) );

            // Copy all items from the property record (which doesn't include INDEX)
            for ( size_t i = 0; i < propRecord.size(); ++i )
            {
                items.push_back( propRecord.getItem( i ) );
            }

            kw.addRecord( Opm::DeckRecord{ std::move( items ) } );
        }
    }

    return kw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword operaterKeyword( std::string          targetProperty,
                                  int                  regionId,
                                  std::string          equation,
                                  std::string          inputProperty,
                                  std::optional<float> alpha,
                                  std::optional<float> beta )
{
    using O = Opm::ParserKeywords::OPERATER;

    // Create the OPERATER keyword
    Opm::DeckKeyword operaterKw( ( Opm::ParserKeywords::OPERATER() ) );

    std::vector<Opm::DeckItem> recordItems;
    recordItems.push_back( RifOpmDeckTools::item( O::TARGET_ARRAY::itemName, targetProperty ) );
    recordItems.push_back( RifOpmDeckTools::item( O::REGION_NUMBER::itemName, regionId ) );
    recordItems.push_back( RifOpmDeckTools::item( O::OPERATION::itemName, equation ) );
    recordItems.push_back( RifOpmDeckTools::item( O::ARRAY_PARAMETER::itemName, inputProperty ) );

    // Add alpha parameter
    if ( alpha.has_value() )
    {
        recordItems.push_back( RifOpmDeckTools::item( O::PARAM1::itemName, alpha.value() ) );
    }
    else
    {
        recordItems.push_back( RifOpmDeckTools::defaultItem( O::PARAM1::itemName ) );
    }

    // Add beta parameter
    if ( beta.has_value() )
    {
        recordItems.push_back( RifOpmDeckTools::item( O::PARAM2::itemName, beta.value() ) );
    }
    else
    {
        recordItems.push_back( RifOpmDeckTools::defaultItem( O::PARAM2::itemName ) );
    }

    // Add final default item
    recordItems.push_back( RifOpmDeckTools::defaultItem( O::REGION_NAME::itemName ) ); // 1* for the last field

    operaterKw.addRecord( Opm::DeckRecord{ std::move( recordItems ) } );
    return operaterKw;
}

} // namespace RimKeywordFactory

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicCompletionDataExportTools.h"

#include "RiaLogging.h"
#include "RiaQStringFormatter.h"
#include "RiaWeightedMeanCalculator.h"
#include "RigStatisticsTools.h"

#include "RicPerforationCellFilterEvaluator.h"
#include "RicTransmissibilityCalculator.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RifTextDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "RimCellFilter.h"
#include "RimEclipseCase.h"
#include "RimNonDarcyPerforationParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

#include "cafAssert.h"

#include <cmath>
#include <format>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData RicCompletionDataExportTools::combineEclipseCellCompletions( const std::vector<RigCompletionData>&    completions,
                                                                               const RicExportCompletionDataSettingsUi& settings )
{
    CAF_ASSERT( !completions.empty() );

    // For detailed description of how the combined completion data is computed, see
    // https://github.com/OPM/ResInsight/issues/7049

    const RigCompletionData& firstCompletion = completions[0];

    const QString&                    wellName       = firstCompletion.wellName();
    const RigCompletionDataGridCell&  cellIndexIJK   = firstCompletion.completionDataGridCell();
    RigCompletionData::CompletionType completionType = firstCompletion.completionType();

    RigCompletionData resultCompletion( wellName, cellIndexIJK, firstCompletion.firstOrderingValue() );
    resultCompletion.setSecondOrderingValue( firstCompletion.secondOrderingValue() );
    resultCompletion.setSourcePdmObject( firstCompletion.sourcePdmObject() );

    RigCompletionData::CellDirection cellDirection                = firstCompletion.direction();
    double                           largestTransmissibilityValue = firstCompletion.transmissibility();

    RiaWeightedMeanCalculator<double> diameterCalculator;
    RiaWeightedMeanCalculator<double> skinFactorCalculator;

    auto isValidTransmissibility = []( double transmissibility )
    { return RigStatisticsTools::isValidNumber<double>( transmissibility ) && transmissibility >= 0.0; };

    auto startMD = completions[0].startMD();
    auto endMD   = completions[0].endMD();

    for ( const RigCompletionData& completion : completions )
    {
        if ( !startMD.has_value() )
        {
            startMD = completion.startMD();
        }
        else
        {
            if ( completion.startMD().has_value() && ( completion.startMD().value() < startMD.value() ) )
            {
                startMD = completion.startMD();
            }
        }

        if ( !endMD.has_value() )
        {
            endMD = completion.endMD();
        }
        else
        {
            if ( completion.endMD().has_value() && ( completion.endMD().value() > endMD.value() ) )
            {
                endMD = completion.endMD();
            }
        }

        double transmissibility = completion.transmissibility();

        if ( !isValidTransmissibility( transmissibility ) )
        {
            QString errorMessage = QString( "Invalid or negative transmissibility value (%1) in cell %3" )
                                       .arg( transmissibility )
                                       .arg( cellIndexIJK.oneBasedLocalCellIndexString() );
            RiaLogging::error( errorMessage.toStdString() );
            resultCompletion.addMetadata( "ERROR", errorMessage );
            continue;
        }

        diameterCalculator.addValueAndWeight( completion.diameter(), transmissibility );
        skinFactorCalculator.addValueAndWeight( completion.skinFactor(), transmissibility );

        if ( transmissibility > largestTransmissibilityValue )
        {
            largestTransmissibilityValue = transmissibility;
            cellDirection                = completion.direction();
        }
    }

    if ( startMD.has_value() && endMD.has_value() )
    {
        resultCompletion.setDepthRange( startMD.value(), endMD.value() );
    }
    if ( auto completionNumber = RicWellPathExportCompletionDataFeatureImpl::completionNumberForCell( completions ) )
        resultCompletion.setCompletionNumber( completionNumber.value() );

    double combinedDiameter   = diameterCalculator.weightedMean();
    double combinedSkinFactor = skinFactorCalculator.weightedMean();

    double combinedTrans   = 0.0;
    double combinedKh      = 0.0;
    double combinedDFactor = 0.0;

    {
        RiaWeightedMeanCalculator<double> dFactorCalculator;

        for ( const RigCompletionData& completion : completions )
        {
            // Error is reported in the loop above, so we can skip this completion
            if ( !isValidTransmissibility( completion.transmissibility() ) ) continue;

            resultCompletion.m_metadata.reserve( resultCompletion.m_metadata.size() + completion.m_metadata.size() );
            resultCompletion.m_metadata.insert( resultCompletion.m_metadata.end(), completion.m_metadata.begin(), completion.m_metadata.end() );

            if ( completion.wellName() != firstCompletion.wellName() )
            {
                QString errorMessage =
                    QString( "Cannot combine completions of different types in same cell %1" ).arg( cellIndexIJK.oneBasedLocalCellIndexString() );
                RiaLogging::error( errorMessage.toStdString() );
                resultCompletion.addMetadata( "ERROR", errorMessage );
                return resultCompletion; // Returning empty completion, should not be exported
            }

            if ( completion.transmissibility() == HUGE_VAL )
            {
                QString errorMessage =
                    QString( "Transmissibility calculation has failed for cell %1" ).arg( cellIndexIJK.oneBasedLocalCellIndexString() );
                RiaLogging::error( errorMessage.toStdString() );
                resultCompletion.addMetadata( "ERROR", errorMessage );
                return resultCompletion; // Returning empty completion, should not be exported
            }

            combinedTrans = combinedTrans + completion.transmissibility();
            combinedKh    = combinedKh + completion.kh();

            dFactorCalculator.addValueAndWeight( completion.dFactor(), completion.transmissibility() );
        }

        // Arithmetic MEAN dFactor weighted by Tj/SumTj from the completions
        // Note : Divide by n is intentional, based on input from @hhgs in mail dated 18.01.2020
        combinedDFactor = dFactorCalculator.weightedMean() / completions.size();
    }

    if ( settings.compdatExport == RicExportCompletionDataSettingsUi::CompdatExport::TRANSMISSIBILITIES )
    {
        resultCompletion.setCombinedValuesExplicitTrans( combinedTrans,
                                                         combinedKh,
                                                         combinedDFactor,
                                                         combinedSkinFactor,
                                                         combinedDiameter,
                                                         cellDirection,
                                                         completionType );
    }
    else if ( settings.compdatExport == RicExportCompletionDataSettingsUi::CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
    {
        // calculate trans for main bore - but as Eclipse will do it!
        double transmissibilityEclipseCalculation =
            RicTransmissibilityCalculator::calculateTransmissibilityAsEclipseDoes( settings.caseToApply(),
                                                                                   combinedSkinFactor,
                                                                                   combinedDiameter / 2,
                                                                                   cellIndexIJK.globalCellIndex(),
                                                                                   cellDirection );

        double defaultKh = RigCompletionData::defaultValue();
        double wpimult   = combinedTrans / transmissibilityEclipseCalculation;
        resultCompletion.setCombinedValuesImplicitTransWPImult( wpimult,
                                                                defaultKh,
                                                                combinedDFactor,
                                                                combinedSkinFactor,
                                                                combinedDiameter,
                                                                cellDirection,
                                                                completionType );
    }

    return resultCompletion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicCompletionDataExportTools::mainGridCompletions( const std::vector<RigCompletionData>& allCompletions )
{
    std::vector<RigCompletionData> completions;

    for ( const auto& completion : allCompletions )
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if ( gridName.isEmpty() )
        {
            completions.push_back( completion );
        }
    }
    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RigCompletionData>>
    RicCompletionDataExportTools::subGridsCompletions( const std::vector<RigCompletionData>& allCompletions )
{
    std::map<QString, std::vector<RigCompletionData>> completions;

    for ( const auto& completion : allCompletions )
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if ( !gridName.isEmpty() )
        {
            auto it = completions.find( gridName );
            if ( it == completions.end() )
            {
                completions.insert( std::pair<QString, std::vector<RigCompletionData>>( gridName, { completion } ) );
            }
            else
            {
                it->second.push_back( completion );
            }
        }
    }
    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCompletionDataExportTools::exportCompdatTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                                     const QString&                        gridName,
                                                                     const std::vector<RigCompletionData>& completionData )
{
    if ( gridName.isEmpty() )
    {
        std::vector<RifTextDataTableColumn> header =
            { RifTextDataTableColumn( "WELL", "NAME" ),
              RifTextDataTableColumn( "", "I" ),
              RifTextDataTableColumn( "", "J" ),
              RifTextDataTableColumn( "", "K1" ),
              RifTextDataTableColumn( "", "K2" ),
              RifTextDataTableColumn( "OPEN", "SHUT" ),
              RifTextDataTableColumn( "SAT", "TAB" ),
              RifTextDataTableColumn( "CONN", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "WELL", "DIA" ),
              RifTextDataTableColumn( "KH", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "SKIN", "FACT" ),
              RifTextDataTableColumn( "D", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "DIR", "PEN" ) };

        formatter.header( header );
        formatter.keyword( "COMPDAT" );
    }
    else
    {
        std::vector<RifTextDataTableColumn> header =
            { RifTextDataTableColumn( "WELL", "NAME" ),
              RifTextDataTableColumn( "LGR", "NAME" ),
              RifTextDataTableColumn( "", "I" ),
              RifTextDataTableColumn( "", "J" ),
              RifTextDataTableColumn( "", "K1" ),
              RifTextDataTableColumn( "", "K2" ),
              RifTextDataTableColumn( "OPEN", "SHUT" ),
              RifTextDataTableColumn( "SAT", "TAB" ),
              RifTextDataTableColumn( "CONN", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "WELL", "DIA" ),
              RifTextDataTableColumn( "KH", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "SKIN", "FACT" ),
              RifTextDataTableColumn( "D", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "DIR", "PEN" ) };

        formatter.header( header );
        formatter.keyword( "COMPDATL" );
    }

    RigCompletionData::CompletionType currentCompletionType = RigCompletionData::CompletionType::CT_UNDEFINED;

    for ( const RigCompletionData& data : completionData )
    {
        if ( currentCompletionType != data.completionType() )
        {
            // The completions are sorted by completion type, write out a heading when completion type changes

            QString txt;
            if ( data.completionType() == RigCompletionData::CompletionType::FISHBONES ) txt = "Fishbones";
            if ( data.completionType() == RigCompletionData::CompletionType::FRACTURE ) txt = "Fracture";
            if ( data.completionType() == RigCompletionData::CompletionType::PERFORATION ) txt = "Perforation";

            formatter.addOptionalComment( "---- Completions for completion type " + txt + " ----" );

            currentCompletionType = data.completionType();
        }

        for ( const RigCompletionMetaData& metadata : data.metadata() )
        {
            formatter.addOptionalComment( QString( "%1 : %2" ).arg( metadata.name ).arg( metadata.comment ) );
        }

        if ( data.transmissibility() == 0.0 || data.wpimult() == 0.0 )
        {
            // Don't export completions without transmissibility
            continue;
        }

        formatter.add( data.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( data.completionDataGridCell().lgrName() );
        }

        formatter.add( data.completionDataGridCell().localCellIndexI() + 1 )
            .add( data.completionDataGridCell().localCellIndexJ() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 );

        formatter.add( "OPEN" );

        formatter.addValueOrDefaultMarker( data.saturation(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.transmissibility(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.diameter(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.kh(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.skinFactor(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.dFactor(), RigCompletionData::defaultValue() );

        switch ( data.direction() )
        {
            case RigCompletionData::CellDirection::DIR_I:
                formatter.add( "'X'" );
                break;
            case RigCompletionData::CellDirection::DIR_J:
                formatter.add( "'Y'" );
                break;
            case RigCompletionData::CellDirection::DIR_K:
            default:
                formatter.add( "'Z'" );
                break;
        }

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCompletionDataExportTools::exportComplumpTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                                      const QString&                        gridName,
                                                                      const std::vector<RigCompletionData>& completionData )
{
    if ( gridName.isEmpty() )
    {
        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "WELL", "NAME" ),
                                                       RifTextDataTableColumn( "", "I" ),
                                                       RifTextDataTableColumn( "", "J" ),
                                                       RifTextDataTableColumn( "", "K1" ),
                                                       RifTextDataTableColumn( "", "K2" ),
                                                       RifTextDataTableColumn( "COMP", "NUM" ) };

        formatter.header( header );
        formatter.keyword( "COMPLUMP" );
    }
    else
    {
        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "WELL", "NAME" ),
                                                       RifTextDataTableColumn( "LGR", "NAME" ),
                                                       RifTextDataTableColumn( "", "I" ),
                                                       RifTextDataTableColumn( "", "J" ),
                                                       RifTextDataTableColumn( "", "K1" ),
                                                       RifTextDataTableColumn( "", "K2" ),
                                                       RifTextDataTableColumn( "COMP", "NUM" ) };

        formatter.header( header );
        formatter.keyword( "COMPLMPL" );
    }

    for ( const RigCompletionData& data : completionData )
    {
        if ( !data.completionNumber().has_value() || data.transmissibility() == 0.0 || data.wpimult() == 0.0 )
        {
            // Don't export completions without transmissibility or completion number set
            continue;
        }

        formatter.add( data.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( data.completionDataGridCell().lgrName() );
        }

        formatter.add( data.completionDataGridCell().localCellIndexI() + 1 )
            .add( data.completionDataGridCell().localCellIndexJ() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 )
            .add( data.completionNumber().value() );

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCompletionDataExportTools::exportWpimultTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                                     const QString&                        gridName,
                                                                     const std::vector<RigCompletionData>& completionData )
{
    std::vector<RifTextDataTableColumn> header;

    if ( gridName.isEmpty() )
    {
        header = {
            RifTextDataTableColumn( "Well" ),
            RifTextDataTableColumn( "Mult" ),
            RifTextDataTableColumn( "I" ),
            RifTextDataTableColumn( "J" ),
            RifTextDataTableColumn( "K" ),
        };
        formatter.keyword( "WPIMULT" );
    }
    else
    {
        header = {
            RifTextDataTableColumn( "Well" ),
            RifTextDataTableColumn( "LgrName" ),
            RifTextDataTableColumn( "Mult" ),
            RifTextDataTableColumn( "I" ),
            RifTextDataTableColumn( "J" ),
            RifTextDataTableColumn( "K" ),
        };
        formatter.keyword( "WPIMULTL" );
    }
    formatter.header( header );

    for ( auto& completion : completionData )
    {
        if ( completion.wpimult() == 0.0 || RigCompletionData::isDefaultValue( completion.wpimult() ) )
        {
            continue;
        }

        formatter.add( completion.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( completion.completionDataGridCell().lgrName() );
        }

        formatter.add( completion.wpimult() );

        formatter.add( completion.completionDataGridCell().localCellIndexI() + 1 )
            .add( completion.completionDataGridCell().localCellIndexJ() + 1 )
            .add( completion.completionDataGridCell().localCellIndexK() + 1 );
        formatter.rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicCompletionDataExportTools::generatePerforationsCompdatValues( const RimWellPath*                                wellPath,
                                                                     const std::vector<const RimPerforationInterval*>& intervals,
                                                                     const RicExportCompletionDataSettingsUi&          settings,
                                                                     const std::optional<QDateTime>&                   exportDate )
{
    RiaDefines::EclipseUnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    std::vector<RigCompletionData> completionData;

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return completionData;

    const RimNonDarcyPerforationParameters* nonDarcyParameters = wellPath->perforationIntervalCollection()->nonDarcyParameters();

    if ( wellPath->perforationIntervalCollection()->isChecked() )
    {
        for ( const RimPerforationInterval* interval : intervals )
        {
            if ( !interval->isChecked() ) continue;
            if ( exportDate.has_value() && !interval->isActiveOnDate( *exportDate ) ) continue;

            std::pair<std::vector<cvf::Vec3d>, std::vector<double>> perforationPointsAndMD =
                wellPathGeometry->clippedPointSubset( interval->startMD(), interval->endMD() );

            std::vector<WellPathCellIntersectionInfo> intersectedCells =
                RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                                  wellPath->name(),
                                                                                  perforationPointsAndMD.first,
                                                                                  perforationPointsAndMD.second );

            RicPerforationCellFilterEvaluator filterEval( interval->cellFilter(), settings.caseToApply );

            const size_t completionCountBefore = completionData.size();

            for ( auto& cell : intersectedCells )
            {
                bool cellIsActive = activeCellInfo->isActive( ReservoirCellIndex( cell.globCellIndex ) );
                if ( !cellIsActive ) continue;
                if ( !filterEval.includesGlobalCell( cell.globCellIndex ) ) continue;

                RigCompletionData completion( wellPath->completionSettings()->wellNameForExport(),
                                              RigCompletionDataGridCell( cell.globCellIndex, settings.caseToApply->mainGrid() ),
                                              cell.startMD );

                RigCompletionData::CellDirection direction =
                    RicTransmissibilityCalculator::calculateCellMainDirection( settings.caseToApply,
                                                                               cell.globCellIndex,
                                                                               cell.intersectionLengthsInCellCS );

                double transmissibility = 0.0;
                double kh               = RigCompletionData::defaultValue();
                double dFactor          = RigCompletionData::defaultValue();

                {
                    auto transmissibilityData =
                        RicTransmissibilityCalculator::calculateTransmissibilityData( settings.caseToApply,
                                                                                      wellPath,
                                                                                      cell.intersectionLengthsInCellCS,
                                                                                      interval->skinFactor(),
                                                                                      interval->diameter( unitSystem ) / 2,
                                                                                      cell.globCellIndex,
                                                                                      settings.useLateralNTG );

                    transmissibility = transmissibilityData.connectionFactor();
                    kh               = transmissibilityData.kh();

                    if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED )
                    {
                        const double effectiveH = transmissibilityData.effectiveH();

                        const double effectivePermeability =
                            nonDarcyParameters->gridPermeabilityScalingFactor() * transmissibilityData.effectiveK();

                        dFactor =
                            RicTransmissibilityCalculator::calculateDFactor( settings.caseToApply,
                                                                             effectiveH,
                                                                             cell.globCellIndex,
                                                                             wellPath->perforationIntervalCollection()->nonDarcyParameters(),
                                                                             effectivePermeability );
                    }
                }

                completion.setTransAndWPImultBackgroundDataFromPerforation( transmissibility,
                                                                            interval->skinFactor(),
                                                                            interval->diameter( unitSystem ),
                                                                            dFactor,
                                                                            kh,
                                                                            direction );
                completion.setDepthRange( cell.startMD, cell.endMD );
                if ( interval->completionNumber() > 0 ) completion.setCompletionNumber( interval->completionNumber() );

                completion.addMetadata( "Perforation Completion",
                                        QString( "MD In: %1 - MD Out: %2" ).arg( cell.startMD ).arg( cell.endMD ) +
                                            QString( " Transmissibility: " ) + QString::number( transmissibility ) );
                completion.setSourcePdmObject( interval );
                completionData.push_back( completion );
            }

            if ( filterEval.isEnabled() && completionData.size() == completionCountBefore && filterEval.rejectedCellCount() > 0 )
            {
                RiaLogging::info(
                    std::format( "Perforation '{}' on well '{}': all {} intersected cells suppressed by attached cell filter '{}'.",
                                 interval->name(),
                                 wellPath->name(),
                                 filterEval.rejectedCellCount(),
                                 interval->cellFilter() ? interval->cellFilter()->fullName() : QString() ) );
            }
        }
    }

    if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED )
    {
        // Compute D-factor for completion as
        // D_cell = D_well * Sum_Tran_cells / Tran_cell
        //
        // See https://github.com/OPM/ResInsight/issues/7450

        double accumulatedTransmissibility = 0.0;
        for ( const auto& completion : completionData )
        {
            accumulatedTransmissibility += completion.transmissibility();
        }

        double userDefinedDFactor = nonDarcyParameters->userDefinedDFactor();
        for ( auto& completion : completionData )
        {
            double dFactorForCompletion = userDefinedDFactor * accumulatedTransmissibility / completion.transmissibility();
            completion.setDFactor( dFactorForCompletion );
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCompletionDataExportTools::appendCompletionData( std::map<size_t, std::vector<RigCompletionData>>* completionData,
                                                         const std::vector<RigCompletionData>&             completionsToAppend )
{
    for ( const auto& completion : completionsToAppend )
    {
        auto it = completionData->find( completion.completionDataGridCell().globalCellIndex() );
        if ( it != completionData->end() )
        {
            it->second.push_back( completion );
        }
        else
        {
            completionData->insert( std::pair<size_t, std::vector<RigCompletionData>>( completion.completionDataGridCell().globalCellIndex(),
                                                                                       std::vector<RigCompletionData>{ completion } ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicCompletionDataExportTools::topLevelWellPath( const RigCompletionData& completion )
{
    if ( completion.sourcePdmObject() )
    {
        auto parentWellPath = completion.sourcePdmObject()->firstAncestorOrThisOfType<RimWellPath>();
        if ( parentWellPath )
        {
            return parentWellPath->topLevelWellPath();
        }
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimEclipseResultDefinitionTools.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaLogging.h"

#include "RigAllanDiagramData.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigVisibleCategoriesCalculator.h"

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseView.h"
#include "RimFlowDiagnosticsTools.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTernaryLegendConfig.h"

#include <QString>

#include "cafCategoryMapper.h"
#include "cvfColor3.h"
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinitionTools::isDivideByCellFaceAreaPossible( const QString& resultName )
{
    if ( resultName == "FLRWATI+" ) return true;
    if ( resultName == "FLRWATJ+" ) return true;
    if ( resultName == "FLRWATK+" ) return true;

    if ( resultName == "FLROILI+" ) return true;
    if ( resultName == "FLROILJ+" ) return true;
    if ( resultName == "FLROILK+" ) return true;

    if ( resultName == "FLRGASI+" ) return true;
    if ( resultName == "FLRGASJ+" ) return true;
    if ( resultName == "FLRGASK+" ) return true;

    if ( resultName == "TRANX" ) return true;
    if ( resultName == "TRANY" ) return true;
    if ( resultName == "TRANZ" ) return true;

    if ( resultName == "riTRANX" ) return true;
    if ( resultName == "riTRANY" ) return true;
    if ( resultName == "riTRANZ" ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RD::FlowTracerSelectionState RimEclipseResultDefinitionTools::getFlowTracerSelectionState( bool                        isInjector,
                                                                                           RD::FlowTracerSelectionType tracerSelectionType,
                                                                                           const RimFlowDiagSolution*  flowDiagSolution,
                                                                                           size_t                      selectedTracerCount )
{
    if ( isInjector && ( tracerSelectionType == RD::FlowTracerSelectionType::FLOW_TR_INJECTORS ||
                         tracerSelectionType == RD::FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD ) )
    {
        return RD::FlowTracerSelectionState::ALL_SELECTED;
    }

    if ( !isInjector && ( tracerSelectionType == RD::FlowTracerSelectionType::FLOW_TR_PRODUCERS ||
                          tracerSelectionType == RD::FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD ) )
    {
        return RD::FlowTracerSelectionState::ALL_SELECTED;
    }

    if ( tracerSelectionType == RD::FlowTracerSelectionType::FLOW_TR_BY_SELECTION )
    {
        if ( selectedTracerCount == RimFlowDiagnosticsTools::setOfTracersOfType( flowDiagSolution, isInjector ).size() )
        {
            return RD::FlowTracerSelectionState::ALL_SELECTED;
        }
        if ( selectedTracerCount == 1 )
        {
            return RD::FlowTracerSelectionState::ONE_SELECTED;
        }
        if ( selectedTracerCount > 1 )
        {
            return RD::FlowTracerSelectionState::MULTIPLE_SELECTED;
        }
    }

    return RD::FlowTracerSelectionState::NONE_SELECTED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseResultDefinitionTools::getResultNamesForResultType( RiaDefines::ResultCatType     resultCatType,
                                                                          const RigCaseCellResultsData* results )
{
    if ( resultCatType != RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        if ( !results ) return {};

        return results->resultNames( resultCatType );
    }

    QStringList flowVars;
    flowVars.push_back( RIG_FLD_TOF_RESNAME );
    flowVars.push_back( RIG_FLD_CELL_FRACTION_RESNAME );
    flowVars.push_back( RIG_FLD_MAX_FRACTION_TRACER_RESNAME );
    flowVars.push_back( RIG_FLD_COMMUNICATION_RESNAME );
    return flowVars;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinitionTools::timeOfFlightString( RD::FlowTracerSelectionState injectorState,
                                                             RD::FlowTracerSelectionState producerState,
                                                             bool                         shorter )
{
    QString tofString;
    bool    multipleSelected = false;
    if ( injectorState != RD::FlowTracerSelectionState::NONE_SELECTED && producerState != RD::FlowTracerSelectionState::NONE_SELECTED )
    {
        tofString        = shorter ? "Res.Time" : "Residence Time";
        multipleSelected = true;
    }
    else if ( injectorState != RD::FlowTracerSelectionState::NONE_SELECTED )
    {
        tofString = shorter ? "Fwd.TOF" : "Forward Time of Flight";
    }
    else if ( producerState != RD::FlowTracerSelectionState::NONE_SELECTED )
    {
        tofString = shorter ? "Rev.TOF" : "Reverse Time of Flight";
    }
    else
    {
        tofString = shorter ? "TOF" : "Time of Flight";
    }

    multipleSelected = multipleSelected || injectorState >= RD::FlowTracerSelectionState::MULTIPLE_SELECTED ||
                       producerState >= RD::FlowTracerSelectionState::MULTIPLE_SELECTED;

    if ( multipleSelected && !shorter )
    {
        tofString += " (Average)";
    }

    tofString += " [days]";
    // Conversion from seconds in flow module to days is done in RigFlowDiagTimeStepResult::setTracerTOF()

    return tofString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinitionTools::maxFractionTracerString( RD::FlowTracerSelectionState injectorState,
                                                                  RD::FlowTracerSelectionState producerState,
                                                                  bool                         shorter )
{
    QString mfString;
    if ( injectorState >= RD::FlowTracerSelectionState::ONE_SELECTED && producerState == RD::FlowTracerSelectionState::NONE_SELECTED )
    {
        mfString = shorter ? "FloodReg" : "Flooding Region";
        if ( injectorState >= RD::FlowTracerSelectionState::MULTIPLE_SELECTED ) mfString += "s";
    }
    else if ( injectorState == RD::FlowTracerSelectionState::NONE_SELECTED && producerState >= RD::FlowTracerSelectionState::ONE_SELECTED )
    {
        mfString = shorter ? "DrainReg" : "Drainage Region";
        if ( producerState >= RD::FlowTracerSelectionState::MULTIPLE_SELECTED ) mfString += "s";
    }
    else
    {
        mfString = shorter ? "Drain&FloodReg" : "Drainage/Flooding Regions";
    }
    return mfString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinitionTools::selectedTracersString( RD::FlowTracerSelectionState injectorState,
                                                                RD::FlowTracerSelectionState producerState,
                                                                const std::vector<QString>&  selectedInjectors,
                                                                const std::vector<QString>&  selectedProducers,
                                                                int                          maxTracerStringLength )
{
    QStringList fullTracersList;

    if ( injectorState == RD::FlowTracerSelectionState::ALL_SELECTED && producerState == RD::FlowTracerSelectionState::ALL_SELECTED )
    {
        fullTracersList += caf::AppEnum<RD::FlowTracerSelectionType>::uiText( RD::FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD );
    }
    else
    {
        if ( injectorState == RD::FlowTracerSelectionState::ALL_SELECTED )
        {
            fullTracersList += caf::AppEnum<RD::FlowTracerSelectionType>::uiText( RD::FlowTracerSelectionType::FLOW_TR_INJECTORS );
        }

        if ( producerState == RD::FlowTracerSelectionState::ALL_SELECTED )
        {
            fullTracersList += caf::AppEnum<RD::FlowTracerSelectionType>::uiText( RD::FlowTracerSelectionType::FLOW_TR_PRODUCERS );
        }

        if ( injectorState == RD::FlowTracerSelectionState::ONE_SELECTED || injectorState == RD::FlowTracerSelectionState::MULTIPLE_SELECTED )
        {
            QStringList listOfSelectedInjectors;
            for ( const QString& injector : selectedInjectors )
            {
                listOfSelectedInjectors.push_back( injector );
            }
            if ( !listOfSelectedInjectors.empty() )
            {
                fullTracersList += listOfSelectedInjectors.join( ", " );
            }
        }

        if ( producerState == RD::FlowTracerSelectionState::ONE_SELECTED || producerState == RD::FlowTracerSelectionState::MULTIPLE_SELECTED )
        {
            QStringList listOfSelectedProducers;
            for ( const QString& producer : selectedProducers )
            {
                listOfSelectedProducers.push_back( producer );
            }
            if ( !listOfSelectedProducers.empty() )
            {
                fullTracersList.push_back( listOfSelectedProducers.join( ", " ) );
            }
        }
    }

    QString tracersText;
    if ( !fullTracersList.empty() )
    {
        tracersText = fullTracersList.join( ", " );
    }

    if ( !tracersText.isEmpty() )
    {
        const QString postfix = "...";

        if ( tracersText.size() > maxTracerStringLength + postfix.size() )
        {
            tracersText = tracersText.left( maxTracerStringLength );
            tracersText += postfix;
        }
    }

    return tracersText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinitionTools::getInputPropertyFileName( const RimEclipseCase* eclipseCase, const QString& resultName )
{
    if ( eclipseCase )
    {
        RimEclipseInputPropertyCollection* inputPropertyCollection = eclipseCase->inputPropertyCollection();
        if ( inputPropertyCollection )
        {
            RimEclipseInputProperty* inputProperty = inputPropertyCollection->findInputProperty( resultName );
            if ( inputProperty )
            {
                return inputProperty->fileName.v().path();
            }
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinitionTools::updateTernaryLegend( RigCaseCellResultsData* cellResultsData,
                                                           RimTernaryLegendConfig* ternaryLegendConfigToUpdate,
                                                           int                     timeStep )
{
    if ( !cellResultsData ) return;

    size_t maxTimeStepCount = cellResultsData->maxTimeStepCount();
    if ( maxTimeStepCount > 1 )
    {
        {
            RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() );

            if ( cellResultsData->ensureKnownResultLoaded( resAddr ) )
            {
                double globalMin = 0.0;
                double globalMax = 1.0;
                double localMin  = 0.0;
                double localMax  = 1.0;

                cellResultsData->minMaxCellScalarValues( resAddr, globalMin, globalMax );
                cellResultsData->minMaxCellScalarValues( resAddr, timeStep, localMin, localMax );

                ternaryLegendConfigToUpdate->setAutomaticRanges( RimTernaryLegendConfig::TERNARY_SOIL_IDX, globalMin, globalMax, localMin, localMax );
            }
        }

        {
            RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );

            if ( cellResultsData->ensureKnownResultLoaded( resAddr ) )
            {
                double globalMin = 0.0;
                double globalMax = 1.0;
                double localMin  = 0.0;
                double localMax  = 1.0;

                cellResultsData->minMaxCellScalarValues( resAddr, globalMin, globalMax );
                cellResultsData->minMaxCellScalarValues( resAddr, timeStep, localMin, localMax );

                ternaryLegendConfigToUpdate->setAutomaticRanges( RimTernaryLegendConfig::TERNARY_SGAS_IDX, globalMin, globalMax, localMin, localMax );
            }
        }

        {
            RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() );

            if ( cellResultsData->ensureKnownResultLoaded( resAddr ) )
            {
                double globalMin = 0.0;
                double globalMax = 1.0;
                double localMin  = 0.0;
                double localMax  = 1.0;

                cellResultsData->minMaxCellScalarValues( resAddr, globalMin, globalMax );
                cellResultsData->minMaxCellScalarValues( resAddr, timeStep, localMin, localMax );

                ternaryLegendConfigToUpdate->setAutomaticRanges( RimTernaryLegendConfig::TERNARY_SWAT_IDX, globalMin, globalMax, localMin, localMax );
            }
        }
    }
}

namespace RimEclipseResultDefinitionTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator<( const cvf::Color3ub first, const cvf::Color3ub second )
{
    if ( first.r() != second.r() ) return first.r() < second.r();
    if ( first.g() != second.g() ) return first.g() < second.g();
    if ( first.b() != second.b() ) return first.b() < second.b();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class TupleCompare
{
public:
    bool operator()( const std::tuple<QString, int, cvf::Color3ub>& t1, const std::tuple<QString, int, cvf::Color3ub>& t2 ) const
    {
        using namespace std;
        if ( get<0>( t1 ) != get<0>( t2 ) ) return get<0>( t1 ) < get<0>( t2 );
        if ( get<1>( t1 ) != get<1>( t2 ) ) return get<1>( t1 ) < get<1>( t2 );
        if ( get<2>( t1 ) != get<2>( t2 ) ) return get<2>( t1 ) < get<2>( t2 );

        return false;
    }
};
} // namespace RimEclipseResultDefinitionTools

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinitionTools::updateLegendForFlowDiagnostics( const RimEclipseResultDefinition* resultDefinition,
                                                                      RimRegularLegendConfig*           legendConfigToUpdate,
                                                                      int                               timeStep )
{
    if ( timeStep < 0 ) return;
    if ( !resultDefinition || !legendConfigToUpdate ) return;

    auto flowDiagSolution = resultDefinition->flowDiagSolution();
    if ( !flowDiagSolution ) return;

    RigFlowDiagResults* flowResultsData = flowDiagSolution->flowDiagResults();
    if ( !flowResultsData ) return;

    RigFlowDiagResultAddress resAddr = resultDefinition->flowDiagResAddress();

    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    flowResultsData->minMaxScalarValues( resAddr, timeStep, &globalMin, &globalMax );
    flowResultsData->posNegClosestToZero( resAddr, timeStep, &globalPosClosestToZero, &globalNegClosestToZero );

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    if ( resultDefinition->hasDynamicResult() )
    {
        flowResultsData->minMaxScalarValues( resAddr, timeStep, &localMin, &localMax );
        flowResultsData->posNegClosestToZero( resAddr, timeStep, &localPosClosestToZero, &localNegClosestToZero );
    }
    else
    {
        localMin = globalMin;
        localMax = globalMax;

        localPosClosestToZero = globalPosClosestToZero;
        localNegClosestToZero = globalNegClosestToZero;
    }

    CVF_ASSERT( legendConfigToUpdate );

    legendConfigToUpdate->disableAllTimeStepsRange( true );
    legendConfigToUpdate->setClosestToZeroValues( globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero );
    legendConfigToUpdate->setAutomaticRanges( globalMin, globalMax, localMin, localMax );

    if ( resultDefinition->hasCategoryResult() )
    {
        auto eclView = resultDefinition->firstAncestorOrThisOfType<RimEclipseView>();
        if ( eclView )
        {
            std::set<std::tuple<QString, int, cvf::Color3ub>, TupleCompare> categories;

            std::vector<QString> tracerNames = flowDiagSolution->tracerNames();
            int                  tracerIndex = 0;

            for ( const auto& tracerName : tracerNames )
            {
                cvf::Color3ub color( cvf::Color3::GRAY );

                RimSimWellInView* well = eclView->wellCollection()->findWell( RimFlowDiagSolution::removeCrossFlowEnding( tracerName ) );

                if ( well ) color = cvf::Color3ub( well->wellPipeColor() );

                categories.insert( std::make_tuple( tracerName, tracerIndex, color ) );
                ++tracerIndex;
            }

            std::vector<std::tuple<QString, int, cvf::Color3ub>> categoryVector;

            if ( resultDefinition->showOnlyVisibleCategoriesInLegend() )
            {
                std::set<int> visibleTracers =
                    RigVisibleCategoriesCalculator::visibleFlowDiagCategories( *eclView, *flowResultsData, resAddr, timeStep );
                for ( auto tupIt : categories )
                {
                    int tracerIndex = std::get<1>( tupIt );
                    if ( visibleTracers.count( tracerIndex ) ) categoryVector.push_back( tupIt );
                }
            }
            else
            {
                for ( const auto& tupIt : categories )
                {
                    categoryVector.push_back( tupIt );
                }
            }

            legendConfigToUpdate->setCategoryItems( categoryVector );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinitionTools::updateCellResultLegend( const RimEclipseResultDefinition* resultDefinition,
                                                              RimRegularLegendConfig*           legendConfigToUpdate,
                                                              int                               timeStep )
{
    if ( !resultDefinition || !legendConfigToUpdate ) return;

    auto cellResultsData = resultDefinition->currentGridCellResults();
    if ( !cellResultsData ) return;

    auto eclipseCaseData = resultDefinition->eclipseCase()->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    cellResultsData->ensureKnownResultLoaded( resultDefinition->eclipseResultAddress() );

    double globalMin, globalMax;
    double globalPosClosestToZero, globalNegClosestToZero;

    cellResultsData->minMaxCellScalarValues( resultDefinition->eclipseResultAddress(), globalMin, globalMax );
    cellResultsData->posNegClosestToZero( resultDefinition->eclipseResultAddress(), globalPosClosestToZero, globalNegClosestToZero );

    double localMin, localMax;
    double localPosClosestToZero, localNegClosestToZero;
    if ( resultDefinition->hasDynamicResult() && timeStep >= 0 )
    {
        cellResultsData->minMaxCellScalarValues( resultDefinition->eclipseResultAddress(), timeStep, localMin, localMax );
        cellResultsData->posNegClosestToZero( resultDefinition->eclipseResultAddress(), timeStep, localPosClosestToZero, localNegClosestToZero );
    }
    else
    {
        localMin = globalMin;
        localMax = globalMax;

        localPosClosestToZero = globalPosClosestToZero;
        localNegClosestToZero = globalNegClosestToZero;
    }

    CVF_ASSERT( legendConfigToUpdate );

    legendConfigToUpdate->disableAllTimeStepsRange( false );
    legendConfigToUpdate->setClosestToZeroValues( globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero );
    legendConfigToUpdate->setAutomaticRanges( globalMin, globalMax, localMin, localMax );

    if ( resultDefinition->hasCategoryResult() )
    {
        if ( resultDefinition->resultType() == RiaDefines::ResultCatType::ALLAN_DIAGRAMS )
        {
            if ( resultDefinition->resultVariable() == RiaResultNames::formationAllanResultName() )
            {
                const std::vector<QString> fnVector = eclipseCaseData->formationNames();
                std::vector<int>           fnameIdxes;
                for ( int i = 0; i < static_cast<int>( fnVector.size() ); i++ )
                {
                    fnameIdxes.push_back( i );
                }

                cvf::Color3ubArray legendBaseColors = legendConfigToUpdate->colorLegend()->colorArray();

                cvf::ref<caf::CategoryMapper> formationColorMapper = new caf::CategoryMapper;
                formationColorMapper->setCategories( fnameIdxes );
                formationColorMapper->setInterpolateColors( legendBaseColors );

                const std::map<std::pair<int, int>, int>& formationCombToCategory =
                    eclipseCaseData->allanDiagramData()->formationCombinationToCategory();

                std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
                for ( int frmNameIdx : fnameIdxes )
                {
                    cvf::Color3ub formationColor = formationColorMapper->mapToColor( frmNameIdx );
                    categories.emplace_back( std::make_tuple( fnVector[frmNameIdx], frmNameIdx, formationColor ) );
                }

                std::set<size_t> visibleAllanCategories;
                {
                    auto eclView = resultDefinition->firstAncestorOrThisOfType<RimEclipseView>();

                    visibleAllanCategories = RigVisibleCategoriesCalculator::visibleAllanCategories( eclView );
                }

                for ( auto [formationPair, categoryIndex] : formationCombToCategory )
                {
                    int frmIdx1 = formationPair.first;
                    int frmIdx2 = formationPair.second;

                    if ( visibleAllanCategories.count( categoryIndex ) == 0 ) continue;

                    auto fnVectorSize = static_cast<int>( fnVector.size() );
                    if ( frmIdx1 >= fnVectorSize || frmIdx2 >= fnVectorSize ) continue;

                    QString frmName1 = fnVector[frmIdx1];
                    QString frmName2 = fnVector[frmIdx2];

                    cvf::Color3f formationColor1 = cvf::Color3f( formationColorMapper->mapToColor( frmIdx1 ) );
                    cvf::Color3f formationColor2 = cvf::Color3f( formationColorMapper->mapToColor( frmIdx2 ) );

                    cvf::Color3ub blendColor = cvf::Color3ub( RiaColorTools::blendCvfColors( formationColor1, formationColor2 ) );

                    categories.emplace_back( std::make_tuple( frmName1 + "-" + frmName2, categoryIndex, blendColor ) );
                }

                legendConfigToUpdate->setCategoryItems( categories );
            }
            else if ( resultDefinition->resultVariable() == RiaResultNames::formationBinaryAllanResultName() )
            {
                std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
                categories.emplace_back( std::make_tuple( "Same formation", 0, cvf::Color3ub::BROWN ) );
                categories.emplace_back( std::make_tuple( "Different formation", 1, cvf::Color3ub::ORANGE ) );

                legendConfigToUpdate->setCategoryItems( categories );
            }
        }
        else if ( resultDefinition->resultType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE &&
                  resultDefinition->resultVariable() == RiaResultNames::completionTypeResultName() )
        {
            const std::vector<int>& visibleCategories = cellResultsData->uniqueCellScalarValues( resultDefinition->eclipseResultAddress() );

            std::vector<RiaDefines::WellPathComponentType> supportedCompletionTypes = { RiaDefines::WellPathComponentType::WELL_PATH,
                                                                                        RiaDefines::WellPathComponentType::FISHBONES,
                                                                                        RiaDefines::WellPathComponentType::PERFORATION_INTERVAL,
                                                                                        RiaDefines::WellPathComponentType::FRACTURE };

            RiaColorTables::WellPathComponentColors colors = RiaColorTables::wellPathComponentColors();

            std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
            for ( auto completionType : supportedCompletionTypes )
            {
                if ( std::find( visibleCategories.begin(), visibleCategories.end(), static_cast<int>( completionType ) ) !=
                     visibleCategories.end() )
                {
                    QString categoryText = caf::AppEnum<RiaDefines::WellPathComponentType>::uiText( completionType );
                    categories.emplace_back( categoryText, static_cast<int>( completionType ), colors[completionType] );
                }
            }

            legendConfigToUpdate->setCategoryItems( categories );
        }
        else
        {
            auto uniqueValues = cellResultsData->uniqueCellScalarValues( resultDefinition->eclipseResultAddress() );
            if ( resultDefinition->eclipseResultAddress().resultCatType() == RiaDefines::ResultCatType::FORMATION_NAMES )
            {
                std::vector<QString> fnVector = eclipseCaseData->formationNames();
                uniqueValues.resize( fnVector.size() );
                std::iota( uniqueValues.begin(), uniqueValues.end(), 0 );
            }

            std::vector<int> visibleCategoryValues = uniqueValues;

            if ( resultDefinition->showOnlyVisibleCategoriesInLegend() )
            {
                auto eclView = resultDefinition->firstAncestorOrThisOfType<RimEclipseView>();
                if ( eclView && eclView->showWindow() )
                {
                    // Check if current result is cell result, and update the visible set of values
                    // TODO: Can be extended to the separate geometry results (separate fault result, separate
                    // intersection results), but this requires some refactoring
                    if ( eclView->cellResult() == resultDefinition )
                    {
                        std::set<int> visibleCategorySet = RigVisibleCategoriesCalculator::visibleCategories( eclView );

                        visibleCategoryValues.clear();
                        visibleCategoryValues.insert( visibleCategoryValues.begin(), visibleCategorySet.begin(), visibleCategorySet.end() );
                    }
                }
            }

            const size_t maxCategoryCount = 500;
            if ( legendConfigToUpdate->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER &&
                 visibleCategoryValues.size() > maxCategoryCount )
            {
                QString txt = QString( "Detected %1 category values. Maximum number of categories is %2. Only the first %2 "
                                       "categories will be displayed. Please use a different color mapping." )
                                  .arg( visibleCategoryValues.size() )
                                  .arg( maxCategoryCount );
                RiaLogging::error( txt );

                visibleCategoryValues.resize( maxCategoryCount );
            }

            cvf::Color3ubArray legendBaseColors = legendConfigToUpdate->colorLegend()->colorArray();

            cvf::ref<caf::CategoryMapper> categoryMapper = new caf::CategoryMapper;
            categoryMapper->setCategories( visibleCategoryValues );
            categoryMapper->setInterpolateColors( legendBaseColors );

            std::vector<std::tuple<QString, int, cvf::Color3ub>> categoryVector;

            for ( auto value : visibleCategoryValues )
            {
                cvf::Color3ub categoryColor = categoryMapper->mapToColor( value );

                QString valueTxt;
                if ( resultDefinition->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES )
                {
                    std::vector<QString> fnVector = eclipseCaseData->formationNames();

                    if ( value < static_cast<int>( fnVector.size() ) )
                    {
                        valueTxt = fnVector[value];
                    }
                }
                else
                {
                    auto items = legendConfigToUpdate->colorLegend()->colorLegendItems();
                    auto it    = std::find_if( items.begin(),
                                            items.end(),
                                            [value]( const RimColorLegendItem* const item ) { return item->categoryValue() == value; } );
                    if ( it != items.end() && !( *it )->categoryName().isEmpty() )
                    {
                        valueTxt = QString( "%1 %2" ).arg( value ).arg( ( *it )->categoryName() );
                    }
                    else
                    {
                        valueTxt = QString( "%1" ).arg( value );
                    }
                }

                categoryVector.emplace_back( valueTxt, value, categoryColor );
            }
            legendConfigToUpdate->setCategoryItems( categoryVector );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseResultDefinitionTools::calcOptionsForVariableUiFieldStandard( RiaDefines::ResultCatType resultCatType,
                                                                                                      const RigCaseCellResultsData* results,
                                                                                                      bool showDerivedResultsFirst,
                                                                                                      bool addPerCellFaceOptionItems,
                                                                                                      bool enableTernary )
{
    if ( !results ) return {};

    QList<caf::PdmOptionItemInfo> optionList;

    QStringList cellCenterResultNames;
    QStringList cellFaceResultNames;

    for ( const QString& s : RimEclipseResultDefinitionTools::getResultNamesForResultType( resultCatType, results ) )
    {
        if ( s == RiaResultNames::completionTypeResultName() )
        {
            if ( results->timeStepDates().empty() ) continue;
        }

        if ( RiaResultNames::isPerCellFaceResult( s ) )
        {
            cellFaceResultNames.push_back( s );
        }
        else
        {
            cellCenterResultNames.push_back( s );
        }
    }

    cellCenterResultNames.sort();
    cellFaceResultNames.sort();

    // Cell Center result names
    for ( const QString& s : cellCenterResultNames )
    {
        optionList.push_back( caf::PdmOptionItemInfo( s, s ) );
    }

    if ( addPerCellFaceOptionItems )
    {
        for ( const QString& s : cellFaceResultNames )
        {
            if ( showDerivedResultsFirst )
            {
                optionList.push_front( caf::PdmOptionItemInfo( s, s ) );
            }
            else
            {
                optionList.push_back( caf::PdmOptionItemInfo( s, s ) );
            }
        }

        // Ternary Result
        if ( enableTernary )
        {
            bool hasAtLeastOneTernaryComponent = false;
            if ( cellCenterResultNames.contains( RiaResultNames::soil() ) )
                hasAtLeastOneTernaryComponent = true;
            else if ( cellCenterResultNames.contains( RiaResultNames::sgas() ) )
                hasAtLeastOneTernaryComponent = true;
            else if ( cellCenterResultNames.contains( RiaResultNames::swat() ) )
                hasAtLeastOneTernaryComponent = true;

            if ( resultCatType == RiaDefines::ResultCatType::DYNAMIC_NATIVE && hasAtLeastOneTernaryComponent )
            {
                optionList.push_front( caf::PdmOptionItemInfo( RiaResultNames::ternarySaturationResultName(),
                                                               RiaResultNames::ternarySaturationResultName() ) );
            }
        }
    }

    optionList.push_front( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), RiaResultNames::undefinedResultName() ) );

    return optionList;
}

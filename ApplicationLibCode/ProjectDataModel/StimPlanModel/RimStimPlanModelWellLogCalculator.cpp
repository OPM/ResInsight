/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimStimPlanModelWellLogCalculator.h"

#include "RiaDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigWellLogCurveData.h"
#include "Well/RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimExtractionConfiguration.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelPressureCalculator.h"
#include "RimStimPlanModelTemplate.h"
#include "RimWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelWellLogCalculator::RimStimPlanModelWellLogCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator )
    : m_stimPlanModelCalculator( stimPlanModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = {
        RiaDefines::CurveProperty::FACIES,
        RiaDefines::CurveProperty::POROSITY,
        RiaDefines::CurveProperty::POROSITY_UNSCALED,
        RiaDefines::CurveProperty::PERMEABILITY_X,
        RiaDefines::CurveProperty::PERMEABILITY_Z,
        RiaDefines::CurveProperty::NET_TO_GROSS,
        RiaDefines::CurveProperty::EQLNUM,
    };

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                   const RimStimPlanModel*   stimPlanModel,
                                                   int                       timeStep,
                                                   std::vector<double>&      values,
                                                   std::vector<double>&      measuredDepthValues,
                                                   std::vector<double>&      tvDepthValues,
                                                   double&                   rkbDiff ) const
{
    RiaLogging::debug( QString( "Calculating well log for '%1'." ).arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );

    std::deque<RimExtractionConfiguration> extractionConfigurations = stimPlanModel->extractionConfigurations( curveProperty );

    std::deque<RimStimPlanModel::MissingValueStrategy> missingValueStratgies = stimPlanModel->missingValueStrategies( curveProperty );

    if ( extractionConfigurations.empty() )
    {
        if ( !extractValuesForProperty( curveProperty, stimPlanModel, timeStep, values, measuredDepthValues, tvDepthValues, rkbDiff ) )
        {
            if ( std::find( missingValueStratgies.begin(), missingValueStratgies.end(), RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE ) !=
                 missingValueStratgies.end() )
            {
                RiaLogging::warning( QString( "Extraction failed. Trying fallback" ) );
                if ( !replaceMissingValuesWithDefault( curveProperty, stimPlanModel, values, measuredDepthValues, tvDepthValues, rkbDiff ) )
                {
                    RiaLogging::error( "Fallback failed too." );
                    return false;
                }
            }
        }
    }
    else
    {
        if ( !extractValuesForPropertyWithConfigurations( curveProperty, stimPlanModel, timeStep, values, measuredDepthValues, tvDepthValues, rkbDiff ) )

        {
            return false;
        }
    }

    double overburdenHeight = stimPlanModel->overburdenHeight();
    if ( overburdenHeight > 0.0 )
    {
        addOverburden( curveProperty, stimPlanModel, tvDepthValues, measuredDepthValues, values );
    }

    double underburdenHeight = stimPlanModel->underburdenHeight();
    if ( underburdenHeight > 0.0 )
    {
        addUnderburden( curveProperty, stimPlanModel, tvDepthValues, measuredDepthValues, values );
    }

    while ( hasMissingValues( values ) && !missingValueStratgies.empty() )
    {
        RimStimPlanModel::MissingValueStrategy strategy = missingValueStratgies.front();
        missingValueStratgies.pop_front();

        if ( strategy == RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE )
        {
            if ( !replaceMissingValuesWithDefault( curveProperty, stimPlanModel, values, measuredDepthValues, tvDepthValues, rkbDiff ) )
            {
                return false;
            }
        }
        else if ( strategy == RimStimPlanModel::MissingValueStrategy::LINEAR_INTERPOLATION )
        {
            RiaInterpolationTools::interpolateMissingValues( measuredDepthValues, values );
        }
        else if ( strategy == RimStimPlanModel::MissingValueStrategy::OTHER_CURVE_PROPERTY )
        {
            if ( !replaceMissingValuesWithOtherProperty( curveProperty, stimPlanModel, timeStep, values ) )
            {
                return false;
            }
        }
        else if ( strategy == RimStimPlanModel::MissingValueStrategy::CELLS_ABOVE )
        {
            // K-1 is up
            int kDirection = -1;
            if ( !replaceMissingValuesWithOtherKLayer( curveProperty, stimPlanModel, timeStep, measuredDepthValues, tvDepthValues, values, kDirection ) )
            {
                return false;
            }
        }
        else if ( strategy == RimStimPlanModel::MissingValueStrategy::CELLS_BELOW )
        {
            // K+1 is down
            int kDirection = 1;
            if ( !replaceMissingValuesWithOtherKLayer( curveProperty, stimPlanModel, timeStep, measuredDepthValues, tvDepthValues, values, kDirection ) )
            {
                return false;
            }
        }
    }

    if ( stimPlanModel->isScaledByNetToGross( curveProperty ) )
    {
        std::vector<double> netToGross = m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );

        scaleByNetToGross( stimPlanModel, netToGross, values );
    }

    // Extracted well log needs to be sampled at same depths as well logs from static grid.
    // If the well log is extracted from a different model it needs to be resampled.
    if ( curveProperty != RiaDefines::CurveProperty::FACIES )
    {
        std::vector<double> targetMds;
        std::vector<double> targetTvds;
        std::vector<double> faciesValues;
        if ( !stimPlanModel->calculator()->extractCurveData( RiaDefines::CurveProperty::FACIES, timeStep, faciesValues, targetMds, targetTvds, rkbDiff ) )
        {
            return false;
        }

        if ( targetMds.size() != measuredDepthValues.size() )
        {
            RiaLogging::info( "Resampling data to fit static case." );
            auto [tvds, mds, results] =
                RimStimPlanModelPressureCalculator::interpolateMissingValues( targetTvds, targetMds, measuredDepthValues, values );
            tvDepthValues       = tvds;
            measuredDepthValues = mds;
            values              = results;
        }
    }

    RiaLogging::debug( QString( "Well log for '%1' done. Size: %2." )
                           .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() )
                           .arg( values.size() ) );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::hasMissingValues( const std::vector<double>& values )
{
    for ( double v : values )
    {
        if ( v == std::numeric_limits<double>::infinity() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelWellLogCalculator::replaceMissingValues( std::vector<double>& values, double defaultValue )
{
    for ( double& v : values )
    {
        if ( v == std::numeric_limits<double>::infinity() )
        {
            v = defaultValue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelWellLogCalculator::replaceMissingValues( std::vector<double>& values, const std::vector<double>& replacementValues )
{
    CVF_ASSERT( values.size() == replacementValues.size() );
    for ( size_t i = 0; i < values.size(); i++ )
    {
        if ( values[i] == std::numeric_limits<double>::infinity() )
        {
            values[i] = replacementValues[i];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor>
    RimStimPlanModelWellLogCalculator::findMissingValuesAccessor( RigEclipseCaseData*                caseData,
                                                                  RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                  int                                gridIndex,
                                                                  int                                timeStepIndex,
                                                                  const QString&                     resultName ) const
{
    if ( resultName.isEmpty() ) return nullptr;

    RiaDefines::PorosityModelType porosityModelType = RiaDefines::PorosityModelType::MATRIX_MODEL;

    for ( RimEclipseInputProperty* inputProperty : inputPropertyCollection->inputProperties() )
    {
        // Look for input properties starting with the same name as result definition
        if ( inputProperty && inputProperty->resultName().startsWith( resultName ) )
        {
            RiaLogging::info( QString( "Found missing values result for %1: %2" ).arg( resultName ).arg( inputProperty->resultName() ) );

            RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::INPUT_PROPERTY, inputProperty->resultName() );
            caseData->results( porosityModelType )->ensureKnownResultLoaded( resultAddress );
            cvf::ref<RigResultAccessor> resAcc =
                RigResultAccessorFactory::createFromResultAddress( caseData, gridIndex, porosityModelType, timeStepIndex, resultAddress );

            return resAcc;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelWellLogCalculator::addOverburden( RiaDefines::CurveProperty curveProperty,
                                                       const RimStimPlanModel*   stimPlanModel,
                                                       std::vector<double>&      tvDepthValues,
                                                       std::vector<double>&      measuredDepthValues,
                                                       std::vector<double>&      values ) const
{
    if ( !values.empty() )
    {
        double overburdenHeight    = stimPlanModel->overburdenHeight();
        double tvdOverburdenBottom = tvDepthValues[0];
        double tvdOverburdenTop    = tvdOverburdenBottom - overburdenHeight;

        double overburdenTopValue    = std::numeric_limits<double>::infinity();
        double overburdenBottomValue = std::numeric_limits<double>::infinity();
        if ( stimPlanModel->burdenStrategy( curveProperty ) == RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE )
        {
            overburdenTopValue    = stimPlanModel->getDefaultForMissingOverburdenValue( curveProperty );
            overburdenBottomValue = overburdenTopValue;
        }
        else
        {
            double gradient       = stimPlanModel->getOverburdenGradient( curveProperty );
            overburdenBottomValue = values[0];
            overburdenTopValue    = overburdenBottomValue + gradient * -overburdenHeight;
        }

        // Prepend the new "fake" depth for start of overburden
        tvDepthValues.insert( tvDepthValues.begin(), tvdOverburdenBottom );
        tvDepthValues.insert( tvDepthValues.begin(), tvdOverburdenTop );

        // TODO: this is not always correct
        double mdTop = measuredDepthValues[0];
        measuredDepthValues.insert( measuredDepthValues.begin(), mdTop );
        measuredDepthValues.insert( measuredDepthValues.begin(), mdTop - overburdenHeight );

        values.insert( values.begin(), overburdenBottomValue );
        values.insert( values.begin(), overburdenTopValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelWellLogCalculator::addUnderburden( RiaDefines::CurveProperty curveProperty,
                                                        const RimStimPlanModel*   stimPlanModel,
                                                        std::vector<double>&      tvDepthValues,
                                                        std::vector<double>&      measuredDepthValues,
                                                        std::vector<double>&      values ) const
{
    if ( !values.empty() )
    {
        size_t lastIndex = tvDepthValues.size() - 1;

        double underburdenHeight    = stimPlanModel->underburdenHeight();
        double tvdUnderburdenTop    = tvDepthValues[lastIndex];
        double tvdUnderburdenBottom = tvdUnderburdenTop + underburdenHeight;

        double underburdenTopValue    = std::numeric_limits<double>::infinity();
        double underburdenBottomValue = std::numeric_limits<double>::infinity();
        if ( stimPlanModel->burdenStrategy( curveProperty ) == RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE )
        {
            underburdenTopValue    = stimPlanModel->getDefaultForMissingUnderburdenValue( curveProperty );
            underburdenBottomValue = underburdenTopValue;
        }
        else
        {
            double gradient        = stimPlanModel->getUnderburdenGradient( curveProperty );
            underburdenTopValue    = values[lastIndex];
            underburdenBottomValue = underburdenTopValue + gradient * underburdenHeight;
        }

        // Append the new "fake" depth for start of underburden
        tvDepthValues.push_back( tvdUnderburdenTop );
        tvDepthValues.push_back( tvdUnderburdenBottom );

        // Append the new "fake" md
        // TODO: check if this is correct???
        double mdBottom = measuredDepthValues[lastIndex];
        measuredDepthValues.push_back( mdBottom );
        measuredDepthValues.push_back( mdBottom + underburdenHeight );

        values.push_back( underburdenTopValue );
        values.push_back( underburdenBottomValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelWellLogCalculator::scaleByNetToGross( const RimStimPlanModel*    stimPlanModel,
                                                           const std::vector<double>& netToGross,
                                                           std::vector<double>&       values )
{
    if ( netToGross.size() != values.size() )
    {
        RiaLogging::error( QString( "Different sizes for net to gross calculation. NTG length: %1. Values length: %2" )
                               .arg( netToGross.size() )
                               .arg( values.size() ) );
        return;
    }

    double netToGrossCutoff = 1.0;
    bool   useNetToGross    = false;

    if ( stimPlanModel->stimPlanModelTemplate() && stimPlanModel->stimPlanModelTemplate()->nonNetLayers() )
    {
        netToGrossCutoff = stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->cutOff();
        useNetToGross    = !netToGross.empty() && stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->isChecked();
    }

    for ( size_t i = 0; i < values.size(); i++ )
    {
        double ntg = netToGross[i];
        if ( useNetToGross && ntg <= netToGrossCutoff )
        {
            values[i] = ntg * values[i];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::extractValuesForPropertyWithConfigurations( RiaDefines::CurveProperty curveProperty,
                                                                                    const RimStimPlanModel*   stimPlanModel,
                                                                                    int                       timeStep,
                                                                                    std::vector<double>&      values,
                                                                                    std::vector<double>&      measuredDepthValues,
                                                                                    std::vector<double>&      tvDepthValues,
                                                                                    double&                   rkbDiff ) const
{
    std::deque<RimExtractionConfiguration> extractionConfigurations = stimPlanModel->extractionConfigurations( curveProperty );

    QString curvePropertyName = caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText();

    for ( auto extractionConfig : extractionConfigurations )
    {
        RiaDefines::ResultCatType                   resultType      = extractionConfig.resultCategory;
        QString                                     resultVariable  = extractionConfig.resultVariable;
        RimExtractionConfiguration::EclipseCaseType eclipseCaseType = extractionConfig.eclipseCaseType;

        RiaLogging::info( QString( "Trying extraction option for '%1': result property: '%2' result type: '%3' case type: '%4'" )
                              .arg( curvePropertyName )
                              .arg( resultVariable )
                              .arg( caf::AppEnum<RiaDefines::ResultCatType>( resultType ).uiText() )
                              .arg( caf::AppEnum<RimExtractionConfiguration::EclipseCaseType>( eclipseCaseType ).uiText() ) );

        RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForType( eclipseCaseType );

        if ( !eclipseCase )
        {
            RiaLogging::info( "Skipping extraction config due to missing model." );
        }
        else
        {
            bool isOk = extractValuesForProperty( curveProperty,
                                                  stimPlanModel,
                                                  eclipseCase,
                                                  resultType,
                                                  resultVariable,
                                                  timeStep,
                                                  values,
                                                  measuredDepthValues,
                                                  tvDepthValues,
                                                  rkbDiff );
            if ( isOk )
            {
                RiaLogging::info( "Extraction succeeded" );
                return true;
            }
        }
    }

    RiaLogging::info( QString( "Extraction failed. Tried %1 configurations." ).arg( extractionConfigurations.size() ) );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                                                  const RimStimPlanModel*   stimPlanModel,
                                                                  int                       timeStep,
                                                                  std::vector<double>&      values,
                                                                  std::vector<double>&      measuredDepthValues,
                                                                  std::vector<double>&      tvDepthValues,
                                                                  double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( curveProperty );
    if ( !eclipseCase ) return false;

    RiaDefines::ResultCatType resultType     = stimPlanModel->eclipseResultCategory( curveProperty );
    QString                   resultVariable = stimPlanModel->eclipseResultVariable( curveProperty );

    return extractValuesForProperty( curveProperty,
                                     stimPlanModel,
                                     eclipseCase,
                                     resultType,
                                     resultVariable,
                                     timeStep,
                                     values,
                                     measuredDepthValues,
                                     tvDepthValues,
                                     rkbDiff );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                                                  const RimStimPlanModel*   stimPlanModel,
                                                                  RimEclipseCase*           eclipseCase,
                                                                  RiaDefines::ResultCatType resultCategory,
                                                                  const QString             resultVariable,
                                                                  int                       timeStep,
                                                                  std::vector<double>&      values,
                                                                  std::vector<double>&      measuredDepthValues,
                                                                  std::vector<double>&      tvDepthValues,
                                                                  double&                   rkbDiff ) const
{
    if ( !stimPlanModel->thicknessDirectionWellPath() )
    {
        return false;
    }

    RigWellPath* wellPathGeometry = stimPlanModel->thicknessDirectionWellPath()->wellPathGeometry();
    if ( !wellPathGeometry )
    {
        RiaLogging::error( "No well path geometry found for well log exctration" );
        return false;
    }

    RiaLogging::info( QString( "Extracting values for '%1' from grid '%2'." )
                          .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() )
                          .arg( eclipseCase->caseUserDescription() ) );

    auto eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData )
    {
        RiaLogging::error( "Missing Eclipse case data." );
        return false;
    }

    RigEclipseWellLogExtractor eclExtractor( eclipseCaseData, wellPathGeometry, "fracture model" );

    measuredDepthValues = eclExtractor.cellIntersectionMDs();
    tvDepthValues       = eclExtractor.cellIntersectionTVDs();
    rkbDiff             = eclExtractor.wellPathGeometry()->rkbDiff();

    RimEclipseResultDefinition eclipseResultDefinition;
    eclipseResultDefinition.setEclipseCase( eclipseCase );
    eclipseResultDefinition.setResultType( resultCategory );
    eclipseResultDefinition.setPorosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL );
    eclipseResultDefinition.setResultVariable( resultVariable );

    eclipseResultDefinition.loadResult();

    if ( resultCategory != RiaDefines::ResultCatType::DYNAMIC_NATIVE || curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
    {
        timeStep = 0;
    }

    cvf::ref<RigResultAccessor> resAcc =
        RigResultAccessorFactory::createFromResultDefinition( eclipseCaseData, 0, timeStep, &eclipseResultDefinition );

    if ( resAcc.notNull() )
    {
        eclExtractor.curveData( resAcc.p(), &values );
        RiaLogging::info( QString( "Extracted values %1 from grid '%2' for '%3'." )
                              .arg( values.size() )
                              .arg( eclipseCase->caseUserDescription() )
                              .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
    }
    else
    {
        RiaLogging::error( QString( "No result found for %1" ).arg( eclipseResultDefinition.resultVariable() ) );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::replaceMissingValuesWithDefault( RiaDefines::CurveProperty curveProperty,
                                                                         const RimStimPlanModel*   stimPlanModel,
                                                                         std::vector<double>&      values,
                                                                         std::vector<double>&      measuredDepthValues,
                                                                         std::vector<double>&      tvDepthValues,
                                                                         double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( curveProperty );

    if ( !stimPlanModel->thicknessDirectionWellPath() )
    {
        return false;
    }

    RigWellPath* wellPathGeometry = stimPlanModel->thicknessDirectionWellPath()->wellPathGeometry();
    if ( !wellPathGeometry )
    {
        RiaLogging::error( "No well path geometry found for well log exctration" );
        return false;
    }

    // Input properties must use first time step
    int replacementTimeStep = 0;

    QString resultVariable = stimPlanModel->eclipseResultVariable( curveProperty );

    auto eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData )
    {
        RiaLogging::error( "Missing Eclipse case data." );
        return false;
    }

    // Try to locate a backup accessor (e.g. PORO_1 for PORO)
    cvf::ref<RigResultAccessor> backupResAcc =
        findMissingValuesAccessor( eclipseCaseData, eclipseCase->inputPropertyCollection(), 0, replacementTimeStep, resultVariable );

    if ( backupResAcc.notNull() )
    {
        RiaLogging::info( QString( "Reading missing values from input properties for %1." ).arg( resultVariable ) );
        std::vector<double> replacementValues;

        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );
        eclExtractor.curveData( backupResAcc.p(), &replacementValues );

        RiaLogging::debug( QString( "Read %1 values for '%2'" ).arg( replacementValues.size() ).arg( resultVariable ) );

        if ( values.empty() )
        {
            values              = replacementValues;
            measuredDepthValues = eclExtractor.cellIntersectionMDs();
            tvDepthValues       = eclExtractor.cellIntersectionTVDs();
            rkbDiff             = eclExtractor.wellPathGeometry()->rkbDiff();
        }
        else
        {
            double overburdenHeight = stimPlanModel->overburdenHeight();
            if ( overburdenHeight > 0.0 )
            {
                double defaultOverburdenValue = std::numeric_limits<double>::infinity();
                if ( stimPlanModel->burdenStrategy( curveProperty ) == RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE )
                {
                    defaultOverburdenValue = stimPlanModel->getDefaultForMissingOverburdenValue( curveProperty );
                }

                replacementValues.insert( replacementValues.begin(), defaultOverburdenValue );
                replacementValues.insert( replacementValues.begin(), defaultOverburdenValue );
            }

            double underburdenHeight = stimPlanModel->underburdenHeight();
            if ( underburdenHeight > 0.0 )
            {
                double defaultUnderburdenValue = std::numeric_limits<double>::infinity();
                if ( stimPlanModel->burdenStrategy( curveProperty ) == RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE )
                {
                    defaultUnderburdenValue = stimPlanModel->getDefaultForMissingUnderburdenValue( curveProperty );
                }

                replacementValues.push_back( defaultUnderburdenValue );
                replacementValues.push_back( defaultUnderburdenValue );
            }

            replaceMissingValues( values, replacementValues );
        }
    }
    else
    {
        RiaLogging::debug( "No backup result accessor found." );
    }

    // If the backup accessor is not found, or does not provide all the missing values:
    // use default value from the fracture model
    if ( !backupResAcc.notNull() || hasMissingValues( values ) )
    {
        double defaultValue = stimPlanModel->getDefaultForMissingValue( curveProperty );
        replaceMissingValues( values, defaultValue );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::replaceMissingValuesWithOtherProperty( RiaDefines::CurveProperty curveProperty,
                                                                               const RimStimPlanModel*   stimPlanModel,
                                                                               int                       timeStep,
                                                                               std::vector<double>&      values ) const
{
    // Get the missing data from other curve
    RiaDefines::CurveProperty replacementProperty = stimPlanModel->getDefaultPropertyForMissingValues( curveProperty );

    std::vector<double> initialValues;
    std::vector<double> initialMds;
    std::vector<double> initialTvds;
    double              initialRkbDiff = -1.0;
    calculate( replacementProperty, stimPlanModel, timeStep, initialValues, initialMds, initialTvds, initialRkbDiff );

    if ( initialValues.empty() )
    {
        RiaLogging::error( QString( "Empty replacement data found for fracture model curve." ) );
        return false;
    }

    CVF_ASSERT( values.size() == initialValues.size() );
    replaceMissingValues( values, initialValues );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimStimPlanModelWellLogCalculator::loadResults( RigEclipseCaseData*           caseData,
                                                                           RiaDefines::PorosityModelType porosityModel,
                                                                           RiaDefines::ResultCatType     resultType,
                                                                           const QString&                propertyName )
{
    // TODO: is this always enough?
    auto resultData = caseData->results( porosityModel );

    int                     timeStepIndex = 0;
    RigEclipseResultAddress resultAddress( resultType, propertyName );

    if ( !resultData->hasResultEntry( resultAddress ) && resultType != RiaDefines::ResultCatType::INPUT_PROPERTY )
    {
        return loadResults( caseData, porosityModel, RiaDefines::ResultCatType::INPUT_PROPERTY, propertyName );
    }

    CAF_ASSERT( resultData->hasResultEntry( resultAddress ) );
    resultData->ensureKnownResultLoaded( resultAddress );
    return caseData->results( porosityModel )->cellScalarResults( resultAddress, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelWellLogCalculator::replaceMissingValuesWithOtherKLayer( RiaDefines::CurveProperty  curveProperty,
                                                                             const RimStimPlanModel*    stimPlanModel,
                                                                             int                        timeStep,
                                                                             const std::vector<double>& measuredDepths,
                                                                             const std::vector<double>& tvDepthValues,
                                                                             std::vector<double>&       values,
                                                                             int                        moveDirection ) const
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( curveProperty );
    if ( !eclipseCase ) return false;

    if ( !stimPlanModel->thicknessDirectionWellPath() ) return false;

    RigWellPath* wellPathGeometry = stimPlanModel->thicknessDirectionWellPath()->wellPathGeometry();
    if ( !wellPathGeometry ) return false;

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
    if ( !caseData )
    {
        RiaLogging::error( "Missing Eclipse case data." );
        return false;
    }

    RiaDefines::PorosityModelType porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RiaDefines::ResultCatType resultType = stimPlanModel->eclipseResultCategory( curveProperty );
    QString                   resultName = stimPlanModel->eclipseResultVariable( curveProperty );
    if ( resultName.isEmpty() )
    {
        RiaLogging::error( QString( "Invalid result for k layer replacement: %1" ).arg( stimPlanModel->name() ) );
        return false;
    }

    const std::vector<double>& cellValues     = loadResults( caseData, porosityModel, resultType, resultName );
    auto                       activeCellInfo = caseData->activeCellInfo( porosityModel );

    for ( size_t idx = 0; idx < values.size(); idx++ )
    {
        if ( std::isinf( values[idx] ) )
        {
            double     measuredDepth = measuredDepths[idx];
            double     tvd           = tvDepthValues[idx];
            cvf::Vec3d position      = wellPathGeometry->interpolatedPointAlongWellPath( measuredDepth );

            size_t cellIdx = mainGrid->findReservoirCellIndexFromPoint( position );

            if ( cellIdx != cvf::UNDEFINED_SIZE_T )
            {
                size_t i;
                size_t j;
                size_t k;
                bool   isValid = mainGrid->ijkFromCellIndex( cellIdx, &i, &j, &k );
                if ( isValid )
                {
                    RiaLogging::debug( QString( "K-Layer replacement: Replace missing value at MD: %1 TVD: %2. Cell [%3, %4, %5]" )
                                           .arg( measuredDepth )
                                           .arg( tvd )
                                           .arg( i + 1 )
                                           .arg( j + 1 )
                                           .arg( k + 1 ) );

                    int       neighborK = static_cast<int>( k ) + moveDirection;
                    const int minK      = static_cast<int>( 1 );
                    const int maxK      = static_cast<int>( mainGrid->cellCountK() );

                    bool isFound = false;
                    while ( !isFound && neighborK >= minK && neighborK <= maxK )
                    {
                        size_t neighborCellIdx = mainGrid->cellIndexFromIJK( i, j, neighborK );
                        size_t resultIdx       = activeCellInfo->cellResultIndex( neighborCellIdx );

                        if ( neighborCellIdx != cvf::UNDEFINED_SIZE_T && resultIdx < cellValues.size() )
                        {
                            double neighborValue = cellValues[resultIdx];

                            if ( !std::isinf( neighborValue ) )
                            {
                                RiaLogging::info( QString( "Found value. [%1, %2, %3]. Idx=%4 Value=%5." )
                                                      .arg( i + 1 )
                                                      .arg( j + 1 )
                                                      .arg( neighborK + 1 )
                                                      .arg( neighborCellIdx )
                                                      .arg( neighborValue ) );
                                isFound     = true;
                                values[idx] = neighborValue;
                            }
                        }

                        neighborK += moveDirection;
                    }
                }
            }
            else
            {
                RiaLogging::debug( QString( "K-Layer Replacement: Invalid cell idx: MD=%1 TVD=%2 Pos: [%3 %4 %5]" )
                                       .arg( measuredDepth )
                                       .arg( tvd )
                                       .arg( position.x() )
                                       .arg( position.y() )
                                       .arg( position.z() ) );
            }
        }
    }

    return true;
}

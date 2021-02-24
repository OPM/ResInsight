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

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
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
    if ( !extractValuesForProperty( curveProperty, stimPlanModel, timeStep, values, measuredDepthValues, tvDepthValues, rkbDiff ) )
    {
        return false;
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

    std::deque<RimStimPlanModel::MissingValueStrategy> missingValueStratgies =
        stimPlanModel->missingValueStrategies( curveProperty );

    while ( hasMissingValues( values ) && !missingValueStratgies.empty() )
    {
        RimStimPlanModel::MissingValueStrategy strategy = missingValueStratgies.front();
        missingValueStratgies.pop_front();

        if ( strategy == RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE )
        {
            QString resultVariable = stimPlanModel->eclipseResultVariable( curveProperty );

            // Input properties must use first time step
            int replacementTimeStep = 0;
            if ( !replaceMissingValuesWithDefault( curveProperty, stimPlanModel, replacementTimeStep, resultVariable, values ) )
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
            // Get the missing data from other curve
            RiaDefines::CurveProperty replacementProperty =
                stimPlanModel->getDefaultPropertyForMissingValues( curveProperty );

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
        }
    }

    if ( stimPlanModel->isScaledByNetToGross( curveProperty ) )
    {
        std::vector<double> netToGross =
            m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );

        scaleByNetToGross( stimPlanModel, netToGross, values );
    }

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
void RimStimPlanModelWellLogCalculator::replaceMissingValues( std::vector<double>&       values,
                                                              const std::vector<double>& replacementValues )
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
    RiaDefines::PorosityModelType porosityModelType = RiaDefines::PorosityModelType::MATRIX_MODEL;

    for ( RimEclipseInputProperty* inputProperty : inputPropertyCollection->inputProperties() )
    {
        // Look for input properties starting with the same name as result definition
        if ( inputProperty && inputProperty->resultName().startsWith( resultName ) )
        {
            RiaLogging::info(
                QString( "Found missing values result for %1: %2" ).arg( resultName ).arg( inputProperty->resultName() ) );

            RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::INPUT_PROPERTY, inputProperty->resultName() );
            caseData->results( porosityModelType )->ensureKnownResultLoaded( resultAddress );
            cvf::ref<RigResultAccessor> resAcc = RigResultAccessorFactory::createFromResultAddress( caseData,
                                                                                                    gridIndex,
                                                                                                    porosityModelType,
                                                                                                    timeStepIndex,
                                                                                                    resultAddress );

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
        RiaLogging::error( QString( "Different sizes for net to gross calculation." ) );
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
bool RimStimPlanModelWellLogCalculator::extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                                                  const RimStimPlanModel*   stimPlanModel,
                                                                  int                       timeStep,
                                                                  std::vector<double>&      values,
                                                                  std::vector<double>&      measuredDepthValues,
                                                                  std::vector<double>&      tvDepthValues,
                                                                  double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( curveProperty );
    if ( !eclipseCase )
    {
        return false;
    }

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

    RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );

    measuredDepthValues = eclExtractor.cellIntersectionMDs();
    tvDepthValues       = eclExtractor.cellIntersectionTVDs();
    rkbDiff             = eclExtractor.wellPathGeometry()->rkbDiff();

    RimEclipseResultDefinition eclipseResultDefinition;
    eclipseResultDefinition.setEclipseCase( eclipseCase );
    eclipseResultDefinition.setResultType( stimPlanModel->eclipseResultCategory( curveProperty ) );
    eclipseResultDefinition.setPorosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL );
    eclipseResultDefinition.setResultVariable( stimPlanModel->eclipseResultVariable( curveProperty ) );

    eclipseResultDefinition.loadResult();

    if ( stimPlanModel->eclipseResultCategory( curveProperty ) != RiaDefines::ResultCatType::DYNAMIC_NATIVE ||
         curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
    {
        timeStep = 0;
    }

    cvf::ref<RigResultAccessor> resAcc =
        RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                              0,
                                                              timeStep,
                                                              &eclipseResultDefinition );

    if ( resAcc.notNull() )
    {
        eclExtractor.curveData( resAcc.p(), &values );
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
                                                                         int                       timeStep,
                                                                         const QString&            resultVariable,
                                                                         std::vector<double>&      values ) const

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

    // Try to locate a backup accessor (e.g. PORO_1 for PORO)
    cvf::ref<RigResultAccessor> backupResAcc = findMissingValuesAccessor( eclipseCase->eclipseCaseData(),
                                                                          eclipseCase->inputPropertyCollection(),
                                                                          0,
                                                                          timeStep,
                                                                          resultVariable );

    if ( backupResAcc.notNull() )
    {
        RiaLogging::info( QString( "Reading missing values from input properties for %1." ).arg( resultVariable ) );
        std::vector<double> replacementValues;

        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );
        eclExtractor.curveData( backupResAcc.p(), &replacementValues );

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

    // If the backup accessor is not found, or does not provide all the missing values:
    // use default value from the fracture model
    if ( !backupResAcc.notNull() || hasMissingValues( values ) )
    {
        double defaultValue = stimPlanModel->getDefaultForMissingValue( curveProperty );
        replaceMissingValues( values, defaultValue );
    }

    return true;
}

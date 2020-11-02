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
#include "RimFractureModelWellLogCalculator.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"

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
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelTemplate.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelWellLogCalculator::RimFractureModelWellLogCalculator( RimFractureModelCalculator* fractureModelCalculator )
    : m_fractureModelCalculator( fractureModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelWellLogCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = {
        RiaDefines::CurveProperty::FACIES,
        RiaDefines::CurveProperty::POROSITY,
        RiaDefines::CurveProperty::PERMEABILITY_X,
        RiaDefines::CurveProperty::PERMEABILITY_Z,
        RiaDefines::CurveProperty::INITIAL_PRESSURE,
        RiaDefines::CurveProperty::PRESSURE,
        RiaDefines::CurveProperty::NET_TO_GROSS,
    };

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelWellLogCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                   const RimFractureModel*   fractureModel,
                                                   int                       timeStep,
                                                   std::vector<double>&      values,
                                                   std::vector<double>&      measuredDepthValues,
                                                   std::vector<double>&      tvDepthValues,
                                                   double&                   rkbDiff ) const
{
    RimEclipseCase* eclipseCase = fractureModel->eclipseCase();
    if ( !eclipseCase )
    {
        return false;
    }

    if ( !fractureModel->thicknessDirectionWellPath() )
    {
        return false;
    }

    RigWellPath* wellPathGeometry = fractureModel->thicknessDirectionWellPath()->wellPathGeometry();
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
    eclipseResultDefinition.setResultType( fractureModel->eclipseResultCategory( curveProperty ) );
    eclipseResultDefinition.setPorosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL );
    eclipseResultDefinition.setResultVariable( fractureModel->eclipseResultVariable( curveProperty ) );

    eclipseResultDefinition.loadResult();

    if ( fractureModel->eclipseResultCategory( curveProperty ) != RiaDefines::ResultCatType::DYNAMIC_NATIVE ||
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

    double overburdenHeight = fractureModel->overburdenHeight();
    if ( overburdenHeight > 0.0 )
    {
        addOverburden( curveProperty, fractureModel, tvDepthValues, measuredDepthValues, values );
    }

    double underburdenHeight = fractureModel->underburdenHeight();
    if ( underburdenHeight > 0.0 )
    {
        addUnderburden( curveProperty, fractureModel, tvDepthValues, measuredDepthValues, values );
    }

    if ( hasMissingValues( values ) )
    {
        if ( fractureModel->missingValueStrategy( curveProperty ) == RimFractureModel::MissingValueStrategy::DEFAULT_VALUE )
        {
            // Try to locate a backup accessor (e.g. PORO_1 for PORO)
            cvf::ref<RigResultAccessor> backupResAcc = findMissingValuesAccessor( eclipseCase->eclipseCaseData(),
                                                                                  eclipseCase->inputPropertyCollection(),
                                                                                  0,
                                                                                  timeStep,
                                                                                  &eclipseResultDefinition );

            if ( backupResAcc.notNull() )
            {
                RiaLogging::info( QString( "Reading missing values from input properties for %1." )
                                      .arg( eclipseResultDefinition.resultVariable() ) );
                std::vector<double> replacementValues;
                eclExtractor.curveData( backupResAcc.p(), &replacementValues );

                double overburdenHeight = fractureModel->overburdenHeight();
                if ( overburdenHeight > 0.0 )
                {
                    double defaultOverburdenValue = std::numeric_limits<double>::infinity();
                    if ( fractureModel->burdenStrategy( curveProperty ) == RimFractureModel::BurdenStrategy::DEFAULT_VALUE )
                    {
                        defaultOverburdenValue = fractureModel->getDefaultForMissingOverburdenValue( curveProperty );
                    }

                    replacementValues.insert( replacementValues.begin(), defaultOverburdenValue );
                    replacementValues.insert( replacementValues.begin(), defaultOverburdenValue );
                }

                double underburdenHeight = fractureModel->underburdenHeight();
                if ( underburdenHeight > 0.0 )
                {
                    double defaultUnderburdenValue = std::numeric_limits<double>::infinity();
                    if ( fractureModel->burdenStrategy( curveProperty ) == RimFractureModel::BurdenStrategy::DEFAULT_VALUE )
                    {
                        defaultUnderburdenValue = fractureModel->getDefaultForMissingUnderburdenValue( curveProperty );
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
                RiaLogging::info( QString( "Using default value for %1" ).arg( eclipseResultDefinition.resultVariable() ) );

                double defaultValue = fractureModel->getDefaultForMissingValue( curveProperty );

                replaceMissingValues( values, defaultValue );
            }
        }
        else if ( fractureModel->missingValueStrategy( curveProperty ) ==
                  RimFractureModel::MissingValueStrategy::LINEAR_INTERPOLATION )
        {
            RiaLogging::info(
                QString( "Interpolating missing values for %1" ).arg( eclipseResultDefinition.resultVariable() ) );
            RiaInterpolationTools::interpolateMissingValues( measuredDepthValues, values );
        }
        else
        {
            // Get the missing data from other curve
            RiaDefines::CurveProperty replacementProperty =
                fractureModel->getDefaultPropertyForMissingValues( curveProperty );

            std::vector<double> initialValues;
            std::vector<double> initialMds;
            std::vector<double> initialTvds;
            double              initialRkbDiff = -1.0;
            calculate( replacementProperty, fractureModel, timeStep, initialValues, initialMds, initialTvds, initialRkbDiff );

            if ( initialValues.empty() )
            {
                RiaLogging::error( QString( "Empty replacement data found for fracture model curve." ) );
                return false;
            }

            CVF_ASSERT( values.size() == initialValues.size() );
            replaceMissingValues( values, initialValues );
        }
    }

    if ( fractureModel->isScaledByNetToGross( curveProperty ) )
    {
        std::vector<double> netToGross =
            m_fractureModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );

        scaleByNetToGross( fractureModel, netToGross, values );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelWellLogCalculator::hasMissingValues( const std::vector<double>& values )
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
void RimFractureModelWellLogCalculator::replaceMissingValues( std::vector<double>& values, double defaultValue )
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
void RimFractureModelWellLogCalculator::replaceMissingValues( std::vector<double>&       values,
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
    RimFractureModelWellLogCalculator::findMissingValuesAccessor( RigEclipseCaseData*                caseData,
                                                                  RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                  int                                gridIndex,
                                                                  int                                timeStepIndex,
                                                                  RimEclipseResultDefinition* eclipseResultDefinition ) const
{
    QString resultName = eclipseResultDefinition->resultVariable();

    for ( RimEclipseInputProperty* inputProperty : inputPropertyCollection->inputProperties() )
    {
        // Look for input properties starting with the same name as result definition
        if ( inputProperty && inputProperty->resultName().startsWith( resultName ) )
        {
            RiaLogging::info(
                QString( "Found missing values result for %1: %2" ).arg( resultName ).arg( inputProperty->resultName() ) );

            RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::INPUT_PROPERTY, inputProperty->resultName() );
            caseData->results( eclipseResultDefinition->porosityModel() )->ensureKnownResultLoaded( resultAddress );
            cvf::ref<RigResultAccessor> resAcc =
                RigResultAccessorFactory::createFromResultAddress( caseData,
                                                                   gridIndex,
                                                                   eclipseResultDefinition->porosityModel(),
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
void RimFractureModelWellLogCalculator::addOverburden( RiaDefines::CurveProperty curveProperty,
                                                       const RimFractureModel*   fractureModel,
                                                       std::vector<double>&      tvDepthValues,
                                                       std::vector<double>&      measuredDepthValues,
                                                       std::vector<double>&      values ) const
{
    if ( !values.empty() )
    {
        double overburdenHeight    = fractureModel->overburdenHeight();
        double tvdOverburdenBottom = tvDepthValues[0];
        double tvdOverburdenTop    = tvdOverburdenBottom - overburdenHeight;

        double overburdenTopValue    = std::numeric_limits<double>::infinity();
        double overburdenBottomValue = std::numeric_limits<double>::infinity();
        if ( fractureModel->burdenStrategy( curveProperty ) == RimFractureModel::BurdenStrategy::DEFAULT_VALUE )
        {
            overburdenTopValue    = fractureModel->getDefaultForMissingOverburdenValue( curveProperty );
            overburdenBottomValue = overburdenTopValue;
        }
        else
        {
            double gradient       = fractureModel->getOverburdenGradient( curveProperty );
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
void RimFractureModelWellLogCalculator::addUnderburden( RiaDefines::CurveProperty curveProperty,
                                                        const RimFractureModel*   fractureModel,
                                                        std::vector<double>&      tvDepthValues,
                                                        std::vector<double>&      measuredDepthValues,
                                                        std::vector<double>&      values ) const
{
    if ( !values.empty() )
    {
        size_t lastIndex = tvDepthValues.size() - 1;

        double underburdenHeight    = fractureModel->underburdenHeight();
        double tvdUnderburdenTop    = tvDepthValues[lastIndex];
        double tvdUnderburdenBottom = tvdUnderburdenTop + underburdenHeight;

        double underburdenTopValue    = std::numeric_limits<double>::infinity();
        double underburdenBottomValue = std::numeric_limits<double>::infinity();
        if ( fractureModel->burdenStrategy( curveProperty ) == RimFractureModel::BurdenStrategy::DEFAULT_VALUE )
        {
            underburdenTopValue    = fractureModel->getDefaultForMissingUnderburdenValue( curveProperty );
            underburdenBottomValue = underburdenTopValue;
        }
        else
        {
            double gradient        = fractureModel->getUnderburdenGradient( curveProperty );
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
void RimFractureModelWellLogCalculator::scaleByNetToGross( const RimFractureModel*    fractureModel,
                                                           const std::vector<double>& netToGross,
                                                           std::vector<double>&       values )
{
    if ( netToGross.size() != values.size() )
    {
        RiaLogging::error( QString( "Different sizes for net to gross calculation." ) );
        return;
    }

    double cutoff = 1.0;
    if ( fractureModel->fractureModelTemplate() && fractureModel->fractureModelTemplate()->nonNetLayers() &&
         fractureModel->fractureModelTemplate()->nonNetLayers()->isChecked() )
    {
        cutoff = fractureModel->fractureModelTemplate()->nonNetLayers()->cutOff();
    }

    for ( size_t i = 0; i < values.size(); i++ )
    {
        double ntg = netToGross[i];
        if ( ntg <= cutoff )
        {
            values[i] = ntg * values[i];
        }
    }
}

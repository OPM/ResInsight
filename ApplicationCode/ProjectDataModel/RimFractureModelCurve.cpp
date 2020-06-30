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

#include "RimFractureModelCurve.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
#include "RimModeledWellPath.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "RiaApplication.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimFractureModelCurve, "FractureModelCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimFractureModelCurve::MissingValueStrategy>::setUp()
{
    addItem( RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE, "DEFAULT_VALUE", "Default value" );
    addItem( RimFractureModelCurve::MissingValueStrategy::LINEAR_INTERPOLATION,
             "LINEAR_INTERPOLATION",
             "Linear interpolation" );

    setDefault( RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCurve::RimFractureModelCurve()
{
    CAF_PDM_InitObject( "Fracture Model Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    caf::AppEnum<RimFractureModelCurve::MissingValueStrategy> defaultValue =
        RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE;
    CAF_PDM_InitField( &m_missingValueStrategy, "MissingValueStrategy", defaultValue, "Missing Value Strategy", "", "", "" );
    m_missingValueStrategy.uiCapability()->setUiHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCurve::~RimFractureModelCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
    m_wellPath      = fractureModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::setEclipseResultCategory( RiaDefines::ResultCatType catType )
{
    m_eclipseResultDefinition->setResultType( catType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    *isUsingPseudoLength = false;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase )
    {
        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(),
                                                 m_fractureModel->thicknessDirectionWellPath()->wellPathGeometry(),
                                                 "fracture model" );

        measuredDepthValues = eclExtractor.cellIntersectionMDs();
        tvDepthValues       = eclExtractor.cellIntersectionTVDs();
        rkbDiff             = eclExtractor.wellPathData()->rkbDiff();

        m_eclipseResultDefinition->setEclipseCase( eclipseCase );

        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> resAcc =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                  0,
                                                                  m_timeStep,
                                                                  m_eclipseResultDefinition );

        if ( resAcc.notNull() )
        {
            eclExtractor.curveData( resAcc.p(), &values );
        }
        else
        {
            RiaLogging::error( QString( "No result found for %1" ).arg( m_eclipseResultDefinition()->resultVariable() ) );
        }

        double overburdenHeight = m_fractureModel->overburdenHeight();
        if ( overburdenHeight > 0.0 )
        {
            double defaultOverburdenValue = std::numeric_limits<double>::infinity();
            if ( m_missingValueStrategy() == RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE )
            {
                defaultOverburdenValue =
                    m_fractureModel->getDefaultForMissingOverburdenValue( m_eclipseResultDefinition()->resultVariable() );
            }

            addOverburden( tvDepthValues, measuredDepthValues, values, overburdenHeight, defaultOverburdenValue );
        }

        double underburdenHeight = m_fractureModel->underburdenHeight();
        if ( underburdenHeight > 0.0 )
        {
            double defaultUnderburdenValue = std::numeric_limits<double>::infinity();
            if ( m_missingValueStrategy() == RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE )
            {
                defaultUnderburdenValue =
                    m_fractureModel->getDefaultForMissingUnderburdenValue( m_eclipseResultDefinition()->resultVariable() );
            }

            addUnderburden( tvDepthValues, measuredDepthValues, values, underburdenHeight, defaultUnderburdenValue );
        }

        if ( hasMissingValues( values ) )
        {
            if ( m_missingValueStrategy() == RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE )
            {
                // Try to locate a backup accessor (e.g. PORO_1 for PORO)
                cvf::ref<RigResultAccessor> backupResAcc =
                    findMissingValuesAccessor( eclipseCase->eclipseCaseData(),
                                               eclipseCase->inputPropertyCollection(),
                                               0,
                                               m_timeStep,
                                               m_eclipseResultDefinition() );

                if ( backupResAcc.notNull() )
                {
                    RiaLogging::info( QString( "Reading missing values from input properties for %1." )
                                          .arg( m_eclipseResultDefinition()->resultVariable() ) );
                    std::vector<double> replacementValues;
                    eclExtractor.curveData( backupResAcc.p(), &replacementValues );
                    replaceMissingValues( values, replacementValues );
                }

                // If the backup accessor is not found, or does not provide all the missing values:
                // use default value from the fracture model
                if ( !backupResAcc.notNull() || hasMissingValues( values ) )
                {
                    RiaLogging::info(
                        QString( "Using default value for %1" ).arg( m_eclipseResultDefinition()->resultVariable() ) );

                    double defaultValue =
                        m_fractureModel->getDefaultForMissingValue( m_eclipseResultDefinition.value()->resultVariable() );

                    replaceMissingValues( values, defaultValue );
                }
            }
            else
            {
                RiaLogging::info(
                    QString( "Interpolating missing values for %1" ).arg( m_eclipseResultDefinition()->resultVariable() ) );
                RiaInterpolationTools::interpolateMissingValues( measuredDepthValues, values );
            }
        }

        RiaEclipseUnitTools::UnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
        if ( eclipseUnitsType == RiaEclipseUnitTools::UnitSystem::UNITS_FIELD )
        {
            // See https://github.com/OPM/ResInsight/issues/538

            depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
        }
    }

    bool performDataSmoothing = false;
    if ( !values.empty() && !measuredDepthValues.empty() )
    {
        if ( tvDepthValues.empty() )
        {
            this->setValuesAndDepths( values,
                                      measuredDepthValues,
                                      RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                      0.0,
                                      depthUnit,
                                      !performDataSmoothing,
                                      xUnits );
        }
        else
        {
            this->setValuesWithMdAndTVD( values,
                                         measuredDepthValues,
                                         tvDepthValues,
                                         rkbDiff,
                                         depthUnit,
                                         !performDataSmoothing,
                                         xUnits );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::setMissingValueStrategy( MissingValueStrategy strategy )
{
    m_missingValueStrategy = strategy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelCurve::hasMissingValues( const std::vector<double>& values )
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
void RimFractureModelCurve::replaceMissingValues( std::vector<double>& values, double defaultValue )
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
void RimFractureModelCurve::replaceMissingValues( std::vector<double>& values, const std::vector<double>& replacementValues )
{
    assert( values.size() == replacementValues.size() );
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
    RimFractureModelCurve::findMissingValuesAccessor( RigEclipseCaseData*                caseData,
                                                      RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                      int                                gridIndex,
                                                      int                                timeStepIndex,
                                                      RimEclipseResultDefinition*        eclipseResultDefinition )
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
void RimFractureModelCurve::addOverburden( std::vector<double>& tvDepthValues,
                                           std::vector<double>& measuredDepthValues,
                                           std::vector<double>& values,
                                           double               overburdenHeight,
                                           double               defaultOverburdenValue )
{
    if ( !values.empty() )
    {
        // Prepend the new "fake" depth for start of overburden
        double tvdTop = tvDepthValues[0];
        tvDepthValues.insert( tvDepthValues.begin(), tvdTop );
        tvDepthValues.insert( tvDepthValues.begin(), tvdTop - overburdenHeight );

        // TODO: this is not always correct
        double mdTop = measuredDepthValues[0];
        measuredDepthValues.insert( measuredDepthValues.begin(), mdTop );
        measuredDepthValues.insert( measuredDepthValues.begin(), mdTop - overburdenHeight );

        values.insert( values.begin(), defaultOverburdenValue );
        values.insert( values.begin(), defaultOverburdenValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModelCurve::addUnderburden( std::vector<double>& tvDepthValues,
                                            std::vector<double>& measuredDepthValues,
                                            std::vector<double>& values,
                                            double               underburdenHeight,
                                            double               defaultUnderburdenValue )
{
    if ( !values.empty() )
    {
        size_t lastIndex = tvDepthValues.size() - 1;

        // Append the new "fake" depth for start of underburden
        double tvdBottom = tvDepthValues[lastIndex];
        tvDepthValues.push_back( tvdBottom );
        tvDepthValues.push_back( tvdBottom + underburdenHeight );

        // Append the new "fake" md
        // TODO: check if this is correct???
        double mdBottom = measuredDepthValues[lastIndex];
        measuredDepthValues.push_back( mdBottom );
        measuredDepthValues.push_back( mdBottom + underburdenHeight );

        values.push_back( defaultUnderburdenValue );
        values.push_back( defaultUnderburdenValue );
    }
}

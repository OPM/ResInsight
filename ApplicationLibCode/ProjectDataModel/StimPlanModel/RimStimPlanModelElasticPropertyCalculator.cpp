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
#include "RimStimPlanModelElasticPropertyCalculator.h"

#include "RiaDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigElasticProperties.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimElasticProperties.h"
#include "RimFaciesProperties.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelElasticPropertyCalculator.h"
#include "RimStimPlanModelTemplate.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelElasticPropertyCalculator::RimStimPlanModelElasticPropertyCalculator(
    RimStimPlanModelCalculator* stimPlanModelCalculator )
    : m_stimPlanModelCalculator( stimPlanModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelElasticPropertyCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = { RiaDefines::CurveProperty::YOUNGS_MODULUS,
                                                        RiaDefines::CurveProperty::POISSONS_RATIO,
                                                        RiaDefines::CurveProperty::BIOT_COEFFICIENT,
                                                        RiaDefines::CurveProperty::K0,
                                                        RiaDefines::CurveProperty::K_IC,
                                                        RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
                                                        RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT,
                                                        RiaDefines::CurveProperty::SPURT_LOSS,
                                                        RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
                                                        RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT,
                                                        RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT,
                                                        RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION };

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelElasticPropertyCalculator::calculate( RiaDefines::CurveProperty curveProperty,
                                                           const RimStimPlanModel*   stimPlanModel,
                                                           int                       timeStep,
                                                           std::vector<double>&      values,
                                                           std::vector<double>&      measuredDepthValues,
                                                           std::vector<double>&      tvDepthValues,
                                                           double&                   rkbDiff ) const
{
    // Use the static model for extracting elastic properties
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::FACIES );
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
        RiaLogging::error( "No well path geometry found for layer data exctration." );
        return false;
    }

    RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(), wellPathGeometry, "fracture model" );

    measuredDepthValues = eclExtractor.cellIntersectionMDs();
    tvDepthValues       = eclExtractor.cellIntersectionTVDs();
    rkbDiff             = eclExtractor.wellPathGeometry()->rkbDiff();

    // Extract formation data
    cvf::ref<RigResultAccessor> formationResultAccessor =
        RigResultAccessorFactory::createFromResultAddress( eclipseCase->eclipseCaseData(),
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                                                    RiaResultNames::activeFormationNamesResultName() ) );
    if ( !formationResultAccessor.notNull() )
    {
        RiaLogging::error( QString( "No formation result found." ) );
        return false;
    }

    CurveSamplingPointData curveData =
        RimWellLogTrack::curveSamplingPointData( &eclExtractor, formationResultAccessor.p() );

    std::vector<double> formationValues = curveData.data;

    std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

    RimStimPlanModelTemplate* stimPlanModelTemplate = stimPlanModel->stimPlanModelTemplate();
    if ( !stimPlanModelTemplate )
    {
        RiaLogging::error( QString( "No fracture model template found" ) );
        return false;
    }

    RimFaciesProperties* faciesProperties = stimPlanModelTemplate->faciesProperties();
    if ( !faciesProperties )
    {
        RiaLogging::error( QString( "No facies properties found when extracting elastic properties." ) );
        return false;
    }

    RimColorLegend* colorLegend = faciesProperties->colorLegend();
    if ( !colorLegend )
    {
        RiaLogging::error( QString( "No color legend found when extracting elastic properties." ) );
        return false;
    }

    RimElasticProperties* elasticProperties = stimPlanModelTemplate->elasticProperties();
    if ( !elasticProperties )
    {
        RiaLogging::error( QString( "No elastic properties found" ) );
        return false;
    }

    std::vector<double> faciesValues =
        m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::FACIES, timeStep );

    if ( faciesValues.empty() )
    {
        RiaLogging::error( QString( "No facies values found." ) );
        return false;
    }

    std::vector<double> poroValues =
        m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::POROSITY_UNSCALED, timeStep );

    double overburdenHeight = stimPlanModel->overburdenHeight();
    if ( overburdenHeight > 0.0 )
    {
        QString overburdenFormation = stimPlanModel->overburdenFormation();
        addOverburden( formationNamesVector,
                       formationValues,
                       tvDepthValues,
                       measuredDepthValues,
                       overburdenHeight,
                       overburdenFormation );
    }

    double underburdenHeight = stimPlanModel->underburdenHeight();
    if ( underburdenHeight > 0.0 )
    {
        QString underburdenFormation = stimPlanModel->underburdenFormation();
        addUnderburden( formationNamesVector,
                        formationValues,
                        tvDepthValues,
                        measuredDepthValues,
                        underburdenHeight,
                        underburdenFormation );
    }

    std::vector<double> netToGrossValues =
        m_stimPlanModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );

    CAF_ASSERT( tvDepthValues.size() == faciesValues.size() );
    CAF_ASSERT( tvDepthValues.size() == poroValues.size() );
    CAF_ASSERT( tvDepthValues.size() == formationValues.size() );

    bool    isScaledByNetToGross = false;
    double  netToGrossCutoff     = 1.0;
    QString netToGrossFaciesName = "";
    if ( stimPlanModel->stimPlanModelTemplate() && stimPlanModel->stimPlanModelTemplate()->nonNetLayers() )
    {
        isScaledByNetToGross = stimPlanModel->isScaledByNetToGross( curveProperty ) && !netToGrossValues.empty() &&
                               stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->isChecked();
        netToGrossCutoff     = stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->cutOff();
        netToGrossFaciesName = stimPlanModel->stimPlanModelTemplate()->nonNetLayers()->facies();
    }

    for ( size_t i = 0; i < tvDepthValues.size(); i++ )
    {
        // Avoid using the field name in the match for now
        QString fieldName  = "";
        QString faciesName = findFaciesName( *colorLegend, faciesValues[i] );
        int     idx        = static_cast<int>( formationValues[i] );
        if ( std::isinf( formationValues[i] ) || idx < 0 || idx >= static_cast<int>( formationNamesVector.size() ) )
        {
            RiaLogging::error( QString( "Unknown formation found in elastic properties. Value: %1, tvd: %2" )
                                   .arg( formationValues[i] )
                                   .arg( tvDepthValues[i] ) );
            values.clear();
            return false;
        }

        QString formationName = formationNamesVector[idx];
        double  porosity      = poroValues[i];

        FaciesKey faciesKey = std::make_tuple( fieldName, formationName, faciesName );
        if ( elasticProperties->hasPropertiesForFacies( faciesKey ) )
        {
            if ( RimElasticProperties::isScalableProperty( curveProperty ) )
            {
                const RigElasticProperties& rigElasticProperties = elasticProperties->propertiesForFacies( faciesKey );
                double scale = elasticProperties->getPropertyScaling( formationName, faciesName, curveProperty );
                auto [val, isExtrapolated] = rigElasticProperties.getValueForPorosity( curveProperty, porosity, scale );
                if ( isExtrapolated )
                {
                    QString propertyName = caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText();
                    RiaLogging::error(
                        QString( "Elastic property '%1' outside porosity range [%2, %3] for formation='%4', "
                                 "facies='%5', depth=%6, porosity=%7. Extrapolated value: %8" )
                            .arg( propertyName )
                            .arg( rigElasticProperties.porosityMin() )
                            .arg( rigElasticProperties.porosityMax() )
                            .arg( formationName )
                            .arg( faciesName )
                            .arg( tvDepthValues[i] )
                            .arg( porosity )
                            .arg( val ) );
                }

                //
                if ( isScaledByNetToGross && !std::isinf( val ) )
                {
                    double netToGross = netToGrossValues[i];
                    if ( netToGross < netToGrossCutoff )
                    {
                        FaciesKey ntgFaciesKey = std::make_tuple( "", formationName, netToGrossFaciesName );
                        const RigElasticProperties& rigNtgElasticProperties =
                            elasticProperties->propertiesForFacies( ntgFaciesKey );
                        double ntgScale =
                            elasticProperties->getPropertyScaling( formationName, netToGrossFaciesName, curveProperty );
                        auto [ntgValue, isExtrapolated] =
                            rigNtgElasticProperties.getValueForPorosity( curveProperty, porosity, ntgScale );
                        val = val * netToGross + ( 1.0 - netToGross ) * ntgValue;
                        if ( std::isinf( val ) || isExtrapolated )
                        {
                            QString propertyName = caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText();
                            RiaLogging::error(
                                QString( "Elastic property '%1' outside porosity range [%2, %3] for formation='%4', "
                                         "facies='%5', depth=%6, porosity=%7 (NTG). Extrapolated value: %8" )
                                    .arg( propertyName )
                                    .arg( rigNtgElasticProperties.porosityMin() )
                                    .arg( rigNtgElasticProperties.porosityMax() )
                                    .arg( formationName )
                                    .arg( netToGrossFaciesName )
                                    .arg( tvDepthValues[i] )
                                    .arg( porosity )
                                    .arg( val ) );
                        }
                    }
                }

                values.push_back( val );
            }
            else if ( stimPlanModel->hasDefaultValueForProperty( curveProperty ) )
            {
                double val = stimPlanModel->getDefaultValueForProperty( curveProperty );
                values.push_back( val );
            }
        }
        else
        {
            RiaLogging::error( QString( "Missing elastic properties. Field='%1', formation='%2', facies='%3'" )
                                   .arg( fieldName )
                                   .arg( formationName )
                                   .arg( faciesName ) );
            values.clear();
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelElasticPropertyCalculator::findFaciesName( const RimColorLegend& colorLegend, double value )
{
    for ( auto item : colorLegend.colorLegendItems() )
    {
        if ( item->categoryValue() == static_cast<int>( value ) ) return item->categoryName();
    }

    return "not found";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelElasticPropertyCalculator::addOverburden( std::vector<QString>& formationNames,
                                                               std::vector<double>&  formationValues,
                                                               std::vector<double>&  tvDepthValues,
                                                               std::vector<double>&  measuredDepthValues,
                                                               double                overburdenHeight,
                                                               const QString&        formationName )
{
    if ( !tvDepthValues.empty() )
    {
        // Prepend the new "fake" depth for start of overburden
        double tvdTop = tvDepthValues[0];
        tvDepthValues.insert( tvDepthValues.begin(), tvdTop );
        tvDepthValues.insert( tvDepthValues.begin(), tvdTop - overburdenHeight );

        // TODO: this is not always correct
        double mdTop = measuredDepthValues[0];
        measuredDepthValues.insert( measuredDepthValues.begin(), mdTop );
        measuredDepthValues.insert( measuredDepthValues.begin(), mdTop - overburdenHeight );

        formationNames.push_back( formationName );

        formationValues.insert( formationValues.begin(), formationNames.size() - 1 );
        formationValues.insert( formationValues.begin(), formationNames.size() - 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelElasticPropertyCalculator::addUnderburden( std::vector<QString>& formationNames,
                                                                std::vector<double>&  formationValues,
                                                                std::vector<double>&  tvDepthValues,
                                                                std::vector<double>&  measuredDepthValues,
                                                                double                underburdenHeight,
                                                                const QString&        formationName )
{
    if ( !tvDepthValues.empty() )
    {
        size_t lastIndex = tvDepthValues.size() - 1;

        double tvdBottom = tvDepthValues[lastIndex];
        tvDepthValues.push_back( tvdBottom );
        tvDepthValues.push_back( tvdBottom + underburdenHeight );

        // TODO: this is not always correct
        double mdBottom = measuredDepthValues[lastIndex];
        measuredDepthValues.push_back( mdBottom );
        measuredDepthValues.push_back( mdBottom + underburdenHeight );

        formationNames.push_back( formationName );

        formationValues.push_back( formationNames.size() - 1 );
        formationValues.push_back( formationNames.size() - 1 );
    }
}

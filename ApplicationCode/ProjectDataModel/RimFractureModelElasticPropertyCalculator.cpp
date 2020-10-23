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
#include "RimFractureModelElasticPropertyCalculator.h"

#include "RiaDefines.h"
#include "RiaFractureModelDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"

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
#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"
#include "RimFractureModelElasticPropertyCalculator.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelTemplate.h"
#include "RimLayerCurve.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
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
RimFractureModelElasticPropertyCalculator::RimFractureModelElasticPropertyCalculator(
    RimFractureModelCalculator* fractureModelCalculator )
    : m_fractureModelCalculator( fractureModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelElasticPropertyCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = {RiaDefines::CurveProperty::YOUNGS_MODULUS,
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
                                                       RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION};

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModelElasticPropertyCalculator::calculate( RiaDefines::CurveProperty curveProperty,
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

    RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(),
                                             fractureModel->thicknessDirectionWellPath()->wellPathGeometry(),
                                             "fracture model" );

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
                                                                                    RiaDefines::activeFormationNamesResultName() ) );
    if ( !formationResultAccessor.notNull() )
    {
        RiaLogging::error( QString( "No formation result found." ) );
        return false;
    }

    CurveSamplingPointData curveData =
        RimWellLogTrack::curveSamplingPointData( &eclExtractor, formationResultAccessor.p() );

    std::vector<double> formationValues = curveData.data;

    std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

    RimFractureModelTemplate* fractureModelTemplate = fractureModel->fractureModelTemplate();
    if ( !fractureModelTemplate )
    {
        RiaLogging::error( QString( "No fracture model template found" ) );
        return false;
    }

    RimFaciesProperties* faciesProperties = fractureModelTemplate->faciesProperties();
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

    RimElasticProperties* elasticProperties = fractureModelTemplate->elasticProperties();
    if ( !elasticProperties )
    {
        RiaLogging::error( QString( "No elastic properties found" ) );
        return false;
    }

    std::vector<double> faciesValues =
        m_fractureModelCalculator->extractValues( RiaDefines::CurveProperty::FACIES, timeStep );
    std::vector<double> poroValues =
        m_fractureModelCalculator->extractValues( RiaDefines::CurveProperty::POROSITY, timeStep );

    double overburdenHeight = fractureModel->overburdenHeight();
    if ( overburdenHeight > 0.0 )
    {
        QString overburdenFormation = fractureModel->overburdenFormation();
        addOverburden( formationNamesVector,
                       formationValues,
                       tvDepthValues,
                       measuredDepthValues,
                       overburdenHeight,
                       overburdenFormation );
    }

    double underburdenHeight = fractureModel->underburdenHeight();
    if ( underburdenHeight > 0.0 )
    {
        QString underburdenFormation = fractureModel->underburdenFormation();
        addUnderburden( formationNamesVector,
                        formationValues,
                        tvDepthValues,
                        measuredDepthValues,
                        underburdenHeight,
                        underburdenFormation );
    }

    std::vector<double> netToGrossValues =
        m_fractureModelCalculator->extractValues( RiaDefines::CurveProperty::NET_TO_GROSS, timeStep );

    CAF_ASSERT( tvDepthValues.size() == faciesValues.size() );
    CAF_ASSERT( tvDepthValues.size() == poroValues.size() );
    CAF_ASSERT( tvDepthValues.size() == formationValues.size() );

    bool    isScaledByNetToGross    = fractureModel->isScaledByNetToGross( curveProperty ) && !netToGrossValues.empty();
    double  netToGrossCutoff        = 1.0;
    QString netToGrossFaciesName    = "";
    QString netToGrossFormationName = "";
    if ( fractureModel->fractureModelTemplate() && fractureModel->fractureModelTemplate()->nonNetLayers() )
    {
        netToGrossCutoff        = fractureModel->fractureModelTemplate()->nonNetLayers()->cutOff();
        netToGrossFaciesName    = fractureModel->fractureModelTemplate()->nonNetLayers()->facies();
        netToGrossFormationName = fractureModel->fractureModelTemplate()->nonNetLayers()->formation();
    }

    FaciesKey ntgFaciesKey = std::make_tuple( "", netToGrossFormationName, netToGrossFaciesName );

    for ( size_t i = 0; i < tvDepthValues.size(); i++ )
    {
        // Avoid using the field name in the match for now
        QString fieldName     = "";
        QString faciesName    = findFaciesName( *colorLegend, faciesValues[i] );
        int     idx           = static_cast<int>( formationValues[i] );
        QString formationName = formationNamesVector[idx];
        double  porosity      = poroValues[i];

        FaciesKey faciesKey = std::make_tuple( fieldName, formationName, faciesName );
        if ( elasticProperties->hasPropertiesForFacies( faciesKey ) )
        {
            if ( RimElasticProperties::isScalableProperty( curveProperty ) )
            {
                const RigElasticProperties& rigElasticProperties = elasticProperties->propertiesForFacies( faciesKey );
                double scale = elasticProperties->getPropertyScaling( formationName, faciesName, curveProperty );
                double val   = rigElasticProperties.getValueForPorosity( curveProperty, porosity, scale );

                //
                if ( isScaledByNetToGross )
                {
                    double netToGross = netToGrossValues[i];
                    if ( netToGross < netToGrossCutoff )
                    {
                        double ntgScale = elasticProperties->getPropertyScaling( netToGrossFormationName,
                                                                                 netToGrossFaciesName,
                                                                                 curveProperty );
                        double ntgValue = rigElasticProperties.getValueForPorosity( curveProperty, porosity, ntgScale );
                        val             = val * netToGross + ( 1.0 - netToGross ) * ntgValue;
                    }
                }

                values.push_back( val );
            }
            else if ( fractureModel->hasDefaultValueForProperty( curveProperty ) )
            {
                double val = fractureModel->getDefaultValueForProperty( curveProperty );
                values.push_back( val );
            }
        }
        else
        {
            RiaLogging::error( QString( "Missing elastic properties. Field='%1', formation='%2', facies='%3'" )
                                   .arg( fieldName )
                                   .arg( formationName )
                                   .arg( faciesName ) );
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModelElasticPropertyCalculator::findFaciesName( const RimColorLegend& colorLegend, double value )
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
void RimFractureModelElasticPropertyCalculator::addOverburden( std::vector<QString>& formationNames,
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
void RimFractureModelElasticPropertyCalculator::addUnderburden( std::vector<QString>& formationNames,
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

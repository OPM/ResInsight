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

#include "RimElasticPropertiesCurve.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigElasticProperties.h"
#include "RigResultAccessorFactory.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimCase.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimElasticProperties.h"
#include "RimFaciesProperties.h"
#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelTemplate.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
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
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimElasticPropertiesCurve, "ElasticPropertiesCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertiesCurve::RimElasticPropertiesCurve()
{
    CAF_PDM_InitObject( "Fracture Model Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_curveProperty, "CurveProperty", "Curve Property", "", "", "" );
    m_curveProperty.uiCapability()->setUiHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticPropertiesCurve::~RimElasticPropertiesCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertiesCurve::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
    m_wellPath      = fractureModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertiesCurve::setEclipseResultCategory( RiaDefines::ResultCatType catType )
{
    m_eclipseResultDefinition->setResultType( catType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertiesCurve::setCurveProperty( RiaDefines::CurveProperty curveProperty )
{
    m_curveProperty = curveProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimElasticPropertiesCurve::curveProperty() const
{
    return m_curveProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertiesCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    *isUsingPseudoLength = false;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase && m_fractureModel )
    {
        RigEclipseWellLogExtractor eclExtractor( eclipseCase->eclipseCaseData(),
                                                 m_fractureModel->thicknessDirectionWellPath()->wellPathGeometry(),
                                                 "fracture model" );

        measuredDepthValues = eclExtractor.cellIntersectionMDs();
        tvDepthValues       = eclExtractor.cellIntersectionTVDs();
        rkbDiff             = eclExtractor.wellPathData()->rkbDiff();

        // Extract formation data
        cvf::ref<RigResultAccessor> formationResultAccessor = RigResultAccessorFactory::
            createFromResultAddress( eclipseCase->eclipseCaseData(),
                                     0,
                                     RiaDefines::PorosityModelType::MATRIX_MODEL,
                                     0,
                                     RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                              RiaDefines::activeFormationNamesResultName() ) );
        if ( !formationResultAccessor.notNull() )
        {
            RiaLogging::error( QString( "No formation result found." ) );
            return;
        }

        CurveSamplingPointData curveData =
            RimWellLogTrack::curveSamplingPointData( &eclExtractor, formationResultAccessor.p() );

        std::vector<double> formationValues = curveData.data;

        std::vector<std::pair<double, double>> yValues;
        std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

        RimFractureModelTemplate* fractureModelTemplate = m_fractureModel->fractureModelTemplate();
        if ( !fractureModelTemplate )
        {
            RiaLogging::error( QString( "No fracture model template found" ) );
            return;
        }

        RimFaciesProperties* faciesProperties = fractureModelTemplate->faciesProperties();
        if ( !faciesProperties )
        {
            RiaLogging::error( QString( "No facies properties found when extracting elastic properties." ) );
            return;
        }

        const RimEclipseResultDefinition* faciesDefinition = faciesProperties->faciesDefinition();

        // Extract facies data
        m_eclipseResultDefinition->setResultVariable( faciesDefinition->resultVariable() );
        m_eclipseResultDefinition->setResultType( faciesDefinition->resultType() );
        m_eclipseResultDefinition->setEclipseCase( eclipseCase );
        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> faciesResultAccessor =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                  0,
                                                                  m_timeStep,
                                                                  m_eclipseResultDefinition );

        if ( !faciesResultAccessor.notNull() )
        {
            RiaLogging::error( QString( "No facies result found." ) );
            return;
        }

        std::vector<double> faciesValues;
        eclExtractor.curveData( faciesResultAccessor.p(), &faciesValues );

        // Extract porosity data: get the porosity values from parent
        RimFractureModelPlot* fractureModelPlot;
        firstAncestorOrThisOfType( fractureModelPlot );
        if ( !fractureModelPlot )
        {
            RiaLogging::error( QString( "No porosity data found when extracting elastic properties." ) );
            return;
        }

        std::vector<double> poroValues;
        fractureModelPlot->getPorosityValues( poroValues );
        if ( poroValues.empty() )
        {
            RiaLogging::error( QString( "Empty porosity data found when extracting elastic properties." ) );
            return;
        }

        RimColorLegend* colorLegend = faciesProperties->colorLegend();
        if ( !colorLegend )
        {
            RiaLogging::error( QString( "No color legend found when extracting elastic properties." ) );
            return;
        }

        RimElasticProperties* elasticProperties = fractureModelTemplate->elasticProperties();
        if ( !elasticProperties )
        {
            RiaLogging::error( QString( "No elastic properties found" ) );
            return;
        }

        double overburdenHeight = m_fractureModel->overburdenHeight();
        if ( overburdenHeight > 0.0 )
        {
            double  defaultPoroValue    = m_fractureModel->defaultOverburdenPorosity();
            QString overburdenFormation = m_fractureModel->overburdenFormation();
            QString overburdenFacies    = m_fractureModel->overburdenFacies();

            addOverburden( formationNamesVector,
                           formationValues,
                           faciesValues,
                           tvDepthValues,
                           measuredDepthValues,
                           overburdenHeight,
                           defaultPoroValue,
                           overburdenFormation,
                           findFaciesValue( *colorLegend, overburdenFacies ) );
        }

        double underburdenHeight = m_fractureModel->underburdenHeight();
        if ( underburdenHeight > 0.0 )
        {
            double  defaultPoroValue     = m_fractureModel->defaultUnderburdenPorosity();
            QString underburdenFormation = m_fractureModel->underburdenFormation();
            QString underburdenFacies    = m_fractureModel->underburdenFacies();

            addUnderburden( formationNamesVector,
                            formationValues,
                            faciesValues,
                            tvDepthValues,
                            measuredDepthValues,
                            underburdenHeight,
                            defaultPoroValue,
                            underburdenFormation,
                            findFaciesValue( *colorLegend, underburdenFacies ) );
        }

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
                const RigElasticProperties& rigElasticProperties = elasticProperties->propertiesForFacies( faciesKey );

                if ( m_curveProperty() == RiaDefines::CurveProperty::YOUNGS_MODULUS )
                {
                    double val = rigElasticProperties.getYoungsModulus( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::POISSONS_RATIO )
                {
                    double val = rigElasticProperties.getPoissonsRatio( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::K_IC )
                {
                    double val = rigElasticProperties.getK_Ic( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::PROPPANT_EMBEDMENT )
                {
                    double val = rigElasticProperties.getProppantEmbedment( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::BIOT_COEFFICIENT )
                {
                    double val = rigElasticProperties.getBiotCoefficient( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::K0 )
                {
                    double val = rigElasticProperties.getK0( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT )
                {
                    double val = rigElasticProperties.getFluidLossCoefficient( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::SPURT_LOSS )
                {
                    double val = rigElasticProperties.getSpurtLoss( porosity );
                    values.push_back( val );
                }
                else if ( m_curveProperty() == RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION )
                {
                    double val = rigElasticProperties.getImmobileFluidSaturation( porosity );
                    values.push_back( val );
                }
                else if ( m_fractureModel->hasDefaultValueForProperty( curveProperty() ) )
                {
                    double val = m_fractureModel->getDefaultValueForProperty( curveProperty() );
                    values.push_back( val );
                }
            }
            else
            {
                RiaLogging::error( QString( "Missing elastic properties. Field='%1', formation='%2', facies='%3'" )
                                       .arg( fieldName )
                                       .arg( formationName )
                                       .arg( faciesName ) );
                return;
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
QString RimElasticPropertiesCurve::findFaciesName( const RimColorLegend& colorLegend, double value )
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
double RimElasticPropertiesCurve::findFaciesValue( const RimColorLegend& colorLegend, const QString& name )
{
    for ( auto item : colorLegend.colorLegendItems() )
    {
        if ( item->categoryName() == name ) return item->categoryValue();
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimElasticPropertiesCurve::createCurveAutoName()
{
    return caf::AppEnum<RiaDefines::CurveProperty>::uiText( m_curveProperty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertiesCurve::addOverburden( std::vector<QString>& formationNames,
                                               std::vector<double>&  formationValues,
                                               std::vector<double>&  faciesValues,
                                               std::vector<double>&  tvDepthValues,
                                               std::vector<double>&  measuredDepthValues,
                                               double                overburdenHeight,
                                               double                defaultPoroValue,
                                               const QString&        formationName,
                                               double                faciesValue )
{
    if ( !faciesValues.empty() )
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

        faciesValues.insert( faciesValues.begin(), faciesValue );
        faciesValues.insert( faciesValues.begin(), faciesValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimElasticPropertiesCurve::addUnderburden( std::vector<QString>& formationNames,
                                                std::vector<double>&  formationValues,
                                                std::vector<double>&  faciesValues,
                                                std::vector<double>&  tvDepthValues,
                                                std::vector<double>&  measuredDepthValues,
                                                double                underburdenHeight,
                                                double                defaultPoroValue,
                                                const QString&        formationName,
                                                double                faciesValue )
{
    if ( !faciesValues.empty() )
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

        faciesValues.push_back( faciesValue );
        faciesValues.push_back( faciesValue );
    }
}

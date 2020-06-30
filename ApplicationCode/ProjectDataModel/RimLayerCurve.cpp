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

#include "RimLayerCurve.h"

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
#include "RimFractureModel.h"
#include "RimFractureModelPlot.h"
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
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimLayerCurve, "LayerCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimLayerCurve::RimLayerCurve()
{
    CAF_PDM_InitObject( "Fracture Model Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModel, "FractureModel", "Fracture Model", "", "", "" );
    m_fractureModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_fractureModel.uiCapability()->setUiHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimLayerCurve::~RimLayerCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimLayerCurve::setFractureModel( RimFractureModel* fractureModel )
{
    m_fractureModel = fractureModel;
    m_wellPath      = fractureModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimLayerCurve::setEclipseResultCategory( RiaDefines::ResultCatType catType )
// {
//     m_eclipseResultDefinition->setResultType( catType );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimLayerCurve::setPropertyType( PropertyType propertyType )
// {
//     m_propertyType = propertyType;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimLayerCurve::performDataExtraction( bool* isUsingPseudoLength )
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

        rkbDiff = eclExtractor.wellPathData()->rkbDiff();

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

        std::vector<std::pair<double, double>> yValues;
        std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( eclipseCase );

        double overburdenHeight = m_fractureModel->overburdenHeight();
        if ( overburdenHeight > 0.0 )
        {
            RimWellLogTrack::addOverburden( formationNamesVector, curveData, overburdenHeight );
        }

        double underburdenHeight = m_fractureModel->underburdenHeight();
        if ( underburdenHeight > 0.0 )
        {
            RimWellLogTrack::addUnderburden( formationNamesVector, curveData, underburdenHeight );
        }

        measuredDepthValues = curveData.md;
        tvDepthValues       = curveData.tvd;

        std::vector<QString> formationNamesToPlot;
        RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                formationNamesVector,
                                                RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH,
                                                &formationNamesToPlot,
                                                &yValues );

        // Extract facies data
        m_eclipseResultDefinition->setResultVariable( "OPERNUM_1" );
        m_eclipseResultDefinition->setResultType( RiaDefines::ResultCatType::INPUT_PROPERTY );
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
        if ( overburdenHeight > 0.0 )
        {
            faciesValues.insert( faciesValues.begin(), std::numeric_limits<double>::infinity() );
            faciesValues.insert( faciesValues.begin(), std::numeric_limits<double>::infinity() );
        }

        if ( underburdenHeight > 0.0 )
        {
            faciesValues.push_back( std::numeric_limits<double>::infinity() );
            faciesValues.push_back( std::numeric_limits<double>::infinity() );
        }

        assert( faciesValues.size() == curveData.data.size() );

        values.resize( faciesValues.size() );

        int    layerNo           = 0;
        double previousFormation = -1.0;
        double previousFacies    = -1.0;
        for ( size_t i = 0; i < faciesValues.size(); i++ )
        {
            if ( previousFormation != curveData.data[i] || previousFacies != faciesValues[i] )
            {
                layerNo++;
            }

            values[i]         = layerNo;
            previousFormation = curveData.data[i];
            previousFacies    = faciesValues[i];
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
// QString RimLayerCurve::findFaciesName( const RimColorLegend& colorLegend, double value )
// {
//     for ( auto item : colorLegend.colorLegendItems() )
//     {
//         if ( item->categoryValue() == static_cast<int>( value ) ) return item->categoryName();
//     }

//     return "not found";
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimLayerCurve::findFormationNameForDepth( const std::vector<QString>&                   formationNames,
                                                  const std::vector<std::pair<double, double>>& depthRanges,
                                                  double                                        depth )
{
    //    assert(formationNames.size() == depthRanges.size());
    for ( size_t i = 0; i < formationNames.size(); i++ )
    {
        double high = depthRanges[i].second;
        double low  = depthRanges[i].first;
        if ( depth >= low && depth <= high )
        {
            return formationNames[i];
        }
    }

    return "not found";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimLayerCurve::createCurveAutoName()
{
    return "Layers";
}

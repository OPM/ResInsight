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

    CAF_PDM_InitFieldNoDefault( &m_curveProperty, "CurveProperty", "Curve Property", "", "", "" );
    m_curveProperty.uiCapability()->setUiHidden( true );

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
    if ( eclipseCase && m_fractureModel )
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

        // Extract facies data
        RimFractureModelPlot* fractureModelPlot;
        firstAncestorOrThisOfType( fractureModelPlot );
        if ( !fractureModelPlot )
        {
            RiaLogging::error( QString( "No facies data found for layer curve." ) );
            return;
        }

        std::vector<double> faciesValues;
        fractureModelPlot->getFaciesValues( faciesValues );
        if ( faciesValues.empty() )
        {
            RiaLogging::error( QString( "Empty facies data found for layer curve." ) );
            return;
        }

        if ( faciesValues.size() != curveData.data.size() || faciesValues.size() != measuredDepthValues.size() ) return;

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
QString RimLayerCurve::createCurveAutoName()
{
    return "Layers";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimLayerCurve::setCurveProperty( RiaDefines::CurveProperty curveProperty )
{
    m_curveProperty = curveProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimLayerCurve::curveProperty() const
{
    return m_curveProperty();
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimStimPlanModelCurve.h"

#include "RiaApplication.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaStimPlanModelDefines.h"

#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimModeledWellPath.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelPlot.h"
#include "RimWellLogTrack.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimStimPlanModelCurve, "StimPlanModelCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCurve::RimStimPlanModelCurve()
{
    CAF_PDM_InitObject( "StimPlan Model Curve" );

    CAF_PDM_InitFieldNoDefault( &m_stimPlanModel, "StimPlanModel", "StimPlan Model" );
    m_stimPlanModel.uiCapability()->setUiTreeChildrenHidden( true );
    m_stimPlanModel.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_curveProperty, "CurveProperty", "Curve Property" );
    m_curveProperty.uiCapability()->setUiHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelCurve::~RimStimPlanModelCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCurve::setStimPlanModel( RimStimPlanModel* stimPlanModel )
{
    m_stimPlanModel = stimPlanModel;
    m_case          = stimPlanModel->eclipseCaseForProperty( m_curveProperty() );
    m_timeStep      = stimPlanModel->timeStep();
    m_wellPath      = stimPlanModel->thicknessDirectionWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCurve::setCurveProperty( RiaDefines::CurveProperty curveProperty )
{
    m_curveProperty = curveProperty;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimStimPlanModelCurve::curveProperty() const
{
    return m_curveProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    RimWellLogExtractionCurve::fieldChangedByUi( changedField, oldValue, newValue );
    RimStimPlanModelPlot* stimPlanModelPlot;
    firstAncestorOrThisOfTypeAsserted( stimPlanModelPlot );

    if ( stimPlanModelPlot )
    {
        stimPlanModelPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    *isUsingPseudoLength = false;

    if ( m_stimPlanModel && m_stimPlanModel->eclipseCaseForProperty( m_curveProperty() ) )
    {
        RimEclipseCase* eclipseCase = m_stimPlanModel->eclipseCaseForProperty( m_curveProperty() );

        bool isOk = m_stimPlanModel->calculator()->extractCurveData( curveProperty(),
                                                                     m_stimPlanModel->timeStep(),
                                                                     values,
                                                                     measuredDepthValues,
                                                                     tvDepthValues,
                                                                     rkbDiff );
        if ( !isOk )
        {
            return;
        }

        RiaDefines::EclipseUnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
        if ( eclipseUnitsType == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            // See https://github.com/OPM/ResInsight/issues/538

            depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
        }
    }

    bool performDataSmoothing = false;
    if ( !values.empty() && !measuredDepthValues.empty() && measuredDepthValues.size() == values.size() )
    {
        RimWellLogTrack* track = nullptr;
        firstAncestorOfType( track );
        if ( track && track->isLogarithmicScale() )
        {
            filterInvalidValuesForLogarithmicScale( values );
        }

        this->setValuesWithMdAndTVD( values, measuredDepthValues, tvDepthValues, rkbDiff, depthUnit, !performDataSmoothing, xUnits );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModelCurve::createCurveAutoName()
{
    QString textWithLineFeed = caf::AppEnum<RiaDefines::CurveProperty>::uiText( m_curveProperty() ).trimmed();
    if ( m_curveProperty == RiaDefines::CurveProperty::PRESSURE || m_curveProperty == RiaDefines::CurveProperty::STRESS )
    {
        // Append date for pressure
        QString pressureDate = m_stimPlanModel->pressureDate();
        if ( !pressureDate.isEmpty() ) return QString( "%1 (%2)" ).arg( textWithLineFeed ).arg( pressureDate );
    }

    textWithLineFeed.replace( " ", "\n" );

    return textWithLineFeed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelCurve::filterInvalidValuesForLogarithmicScale( std::vector<double>& values )
{
    for ( double& v : values )
    {
        if ( v <= 0.0 )
        {
            v = RiaDefines::zeroReplacementForLogarithmicPlot();
        }
    }
}

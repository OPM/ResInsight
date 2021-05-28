/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleWellLogStatisticsCurve.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"

#include "RimEnsembleWellLogCurveSet.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimEnsembleWellLogStatisticsCurve, "EnsembleWellLogStatisticsCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogStatisticsCurve::RimEnsembleWellLogStatisticsCurve()
{
    CAF_PDM_InitObject( "Ensemble Well Log Statistics Curve", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleWellLogCurveSet, "EnsembleWellLogCurveSet", "Ensemble Well Log Curve Set", "", "", "" );
    m_ensembleWellLogCurveSet.uiCapability()->setUiTreeChildrenHidden( true );
    m_ensembleWellLogCurveSet.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_statisticsType, "StatisticsType", "Statistics Type", "", "", "" );
    m_statisticsType.uiCapability()->setUiHidden( true );

    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogStatisticsCurve::~RimEnsembleWellLogStatisticsCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogStatisticsCurve::setEnsembleWellLogCurveSet( RimEnsembleWellLogCurveSet* ensembleWellLogCurveSet )
{
    m_ensembleWellLogCurveSet = ensembleWellLogCurveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogStatisticsCurve::setStatisticsType( RimEnsembleWellLogStatistics::StatisticsType statisticsType )
{
    m_statisticsType = statisticsType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogStatistics::StatisticsType RimEnsembleWellLogStatisticsCurve::statisticsType() const
{
    return m_statisticsType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogStatisticsCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    // RimWellLogExtractionCurve::fieldChangedByUi( changedField, oldValue, newValue );
    // RimStimPlanModelPlot* ensembleWellLogCurveSetPlot;
    // firstAncestorOrThisOfTypeAsserted( ensembleWellLogCurveSetPlot );

    // if ( ensembleWellLogCurveSetPlot )
    // {
    //     ensembleWellLogCurveSetPlot->loadDataAndUpdate();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogStatisticsCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    // TODO: get if from the file???
    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_FEET; // METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    *isUsingPseudoLength = false;

    if ( m_ensembleWellLogCurveSet )
    {
        const RimEnsembleWellLogStatistics* ensembleWellLogStatistics =
            m_ensembleWellLogCurveSet->ensembleWellLogStatistics();

        if ( m_statisticsType == RimEnsembleWellLogStatistics::StatisticsType::MEAN )
        {
            values              = ensembleWellLogStatistics->mean();
            measuredDepthValues = ensembleWellLogStatistics->measuredDepths();
        }
        else if ( m_statisticsType == RimEnsembleWellLogStatistics::StatisticsType::P10 )
        {
            values              = ensembleWellLogStatistics->mean();
            measuredDepthValues = ensembleWellLogStatistics->measuredDepths();
        }
        else if ( m_statisticsType == RimEnsembleWellLogStatistics::StatisticsType::P50 )
        {
            values              = ensembleWellLogStatistics->p50();
            measuredDepthValues = ensembleWellLogStatistics->measuredDepths();
        }
        else if ( m_statisticsType == RimEnsembleWellLogStatistics::StatisticsType::P90 )
        {
            values              = ensembleWellLogStatistics->p90();
            measuredDepthValues = ensembleWellLogStatistics->measuredDepths();
        }

        // RiaDefines::EclipseUnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
        // if ( eclipseUnitsType == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        // {
        //     // See https://github.com/OPM/ResInsight/issues/538

        //     depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
        // }
    }

    bool performDataSmoothing = false;
    if ( !values.empty() && !measuredDepthValues.empty() && measuredDepthValues.size() == values.size() )
    {
        this->setValuesAndDepths( values,
                                  measuredDepthValues,
                                  RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                  rkbDiff,
                                  depthUnit,
                                  !performDataSmoothing,
                                  xUnits );
        // this->setValuesWithMdAndTVD( values, measuredDepthValues, tvDepthValues, rkbDiff, depthUnit,
        // !performDataSmoothing, xUnits );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleWellLogStatisticsCurve::createCurveAutoName()
{
    return caf::AppEnum<RimEnsembleWellLogStatistics::StatisticsType>::uiText( m_statisticsType() );
}

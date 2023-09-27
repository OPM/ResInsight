/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleStatistics.h"

#include "RiaColorTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetInterface.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT( RimEnsembleStatistics, "RimEnsembleStatistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleStatistics::RimEnsembleStatistics( RimEnsembleCurveSetInterface* parentCurveSet )
{
    CAF_PDM_InitObject( "Ensemble Curve Filter", ":/EnsembleCurveSet16x16.png" );

    m_parentCurveSet = parentCurveSet;

    CAF_PDM_InitField( &m_active, "Active", true, "Show Statistics Curves" );
    CAF_PDM_InitField( &m_showStatisticsCurveLegends, "ShowStatisticsCurveLegends", false, "Show Statistics Curve Legends" );
    CAF_PDM_InitField( &m_hideEnsembleCurves, "HideEnsembleCurves", false, "Hide Ensemble Curves" );
    CAF_PDM_InitField( &m_basedOnFilteredCases, "BasedOnFilteredCases", false, "Based on Filtered Cases" );
    CAF_PDM_InitField( &m_showP10Curve, "ShowP10Curve", true, "P10" );
    CAF_PDM_InitField( &m_showP50Curve, "ShowP50Curve", false, "P50" );
    CAF_PDM_InitField( &m_showP90Curve, "ShowP90Curve", true, "P90" );
    CAF_PDM_InitField( &m_showMeanCurve, "ShowMeanCurve", true, "Mean" );
    CAF_PDM_InitField( &m_showCurveLabels, "ShowCurveLabels", true, "Show Curve Labels" );
    CAF_PDM_InitField( &m_includeIncompleteCurves, "IncludeIncompleteCurves", false, "Include Incomplete Curves" );

    CAF_PDM_InitField( &m_crossPlotCurvesBinCount, "CrossPlotCurvesBinCount", 100, "Bin Count" );
    CAF_PDM_InitField( &m_crossPlotCurvesStatisticsRealizationCountThresholdPerBin,
                       "CrossPlotCurvesStatisticsRealizationCountThresholdPerBin",
                       10,
                       "Realization Count Threshold per Bin" );

    CAF_PDM_InitField( &m_warningLabel, "WarningLabel", QString( "Warning: Ensemble time range mismatch" ), "" );

    CAF_PDM_InitField( &m_color, "Color", RiaColorTools::textColor3f(), "Color" );

    m_warningLabel.xmlCapability()->disableIO();
    m_warningLabel.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_warningLabel.uiCapability()->setUiReadOnly( true );

    if ( RimProject::current() && RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.1.0" ) )
    {
        // Set to always show curves before the version this feature was introduced in
        m_showStatisticsCurveLegends = true;
    }

    m_showColorField = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatistics::isActive() const
{
    return m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::setShowStatisticsCurves( bool show )
{
    m_active = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEnsembleStatistics::crossPlotCurvesBinCount() const
{
    return m_crossPlotCurvesBinCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEnsembleStatistics::crossPlotRealizationCountThresholdPerBin() const
{
    return m_crossPlotCurvesStatisticsRealizationCountThresholdPerBin;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableP10Curve( bool disable )
{
    m_showP10Curve.uiCapability()->setUiReadOnly( disable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableP50Curve( bool disable )
{
    m_showP50Curve.uiCapability()->setUiReadOnly( disable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableP90Curve( bool disable )
{
    m_showP90Curve.uiCapability()->setUiReadOnly( disable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::disableMeanCurve( bool disable )
{
    m_showMeanCurve.uiCapability()->setUiReadOnly( disable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::showColorField( bool show )
{
    m_showColorField = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_hideEnsembleCurves )
    {
        auto curveSet = m_parentCurveSet;
        if ( !curveSet ) return;

        curveSet->updateAllCurves();

        return;
    }

    auto curveSet = m_parentCurveSet;
    if ( !curveSet ) return;

    curveSet->updateStatisticsCurves();

    // Trigger update of tree view editor for ensemble curve set as they depend on these fields
    if ( changedField == &m_active || changedField == &m_basedOnFilteredCases || changedField == &m_color ) curveSet->updateEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatistics::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto curveSet = m_parentCurveSet;

    uiOrdering.add( &m_active );
    m_showStatisticsCurveLegends.uiCapability()->setUiReadOnly( !m_active );
    uiOrdering.add( &m_showStatisticsCurveLegends );
    uiOrdering.add( &m_hideEnsembleCurves );
    uiOrdering.add( &m_basedOnFilteredCases );
    uiOrdering.add( &m_includeIncompleteCurves );
    uiOrdering.add( &m_showCurveLabels );

    auto crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot" );
    crossPlotGroup->add( &m_crossPlotCurvesBinCount );
    crossPlotGroup->add( &m_crossPlotCurvesStatisticsRealizationCountThresholdPerBin );

    if ( m_showColorField ) uiOrdering.add( &m_color );

    auto group = uiOrdering.addNewGroup( "Curves" );
    if ( !curveSet->hasMeanData() ) group->add( &m_warningLabel );
    group->add( &m_showP90Curve );
    group->add( &m_showP50Curve );
    group->add( &m_showMeanCurve );
    group->add( &m_showP10Curve );

    disableP10Curve( !m_active || !curveSet->hasP10Data() );
    disableP50Curve( !m_active || !curveSet->hasP50Data() );
    disableP90Curve( !m_active || !curveSet->hasP90Data() );
    disableMeanCurve( !m_active || !curveSet->hasMeanData() );
    m_showCurveLabels.uiCapability()->setUiReadOnly( !m_active );
    m_color.uiCapability()->setUiReadOnly( !m_active );

    m_showP10Curve.uiCapability()->setUiName( curveSet->hasP10Data() ? "P10" : "P10 (Needs > 8 curves)" );
    m_showP90Curve.uiCapability()->setUiName( curveSet->hasP90Data() ? "P90" : "P90 (Needs > 8 curves)" );

    uiOrdering.skipRemainingFields( true );
}

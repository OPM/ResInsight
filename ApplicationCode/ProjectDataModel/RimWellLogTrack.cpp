/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogTrack.h"

#include "RiaColorTables.h"
#include "RiaExtractionTools.h"
#include "RiaLogging.h"
#include "RiaSimWellBranchTools.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigStatisticsCalculator.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"
#include "RigWellPathFormations.h"

#include "RimCase.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimGeoMechCase.h"
#include "RimMainPlotCollection.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellAllocationPlot.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "RiuMainWindow.h"
#include "RiuPlotAnnotationTool.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellLogTrack.h"
#include "RiuWellPathComponentPlotItem.h"

#include "RiuQwtLinearScaleEngine.h"

#include "cafPdmUiSliderEditor.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QWheelEvent>

#include <algorithm>

#define RI_LOGPLOTTRACK_MINX_DEFAULT -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT 100.0
#define RI_SCROLLWHEEL_ZOOMFACTOR 1.1
#define RI_SCROLLWHEEL_PANFACTOR 0.1

CAF_PDM_SOURCE_INIT( RimWellLogTrack, "WellLogPlotTrack" );

namespace caf
{
template <>
void AppEnum<RimWellLogTrack::TrajectoryType>::setUp()
{
    addItem( RimWellLogTrack::WELL_PATH, "WELL_PATH", "Well Path" );
    addItem( RimWellLogTrack::SIMULATION_WELL, "SIMULATION_WELL", "Simulation Well" );
    setDefault( RimWellLogTrack::WELL_PATH );
}

template <>
void AppEnum<RimWellLogTrack::FormationSource>::setUp()
{
    addItem( RimWellLogTrack::CASE, "CASE", "Case" );
    addItem( RimWellLogTrack::WELL_PICK_FILTER, "WELL_PICK_FILTER", "Well Picks for Well Path" );
    setDefault( RimWellLogTrack::CASE );
}

template <>
void AppEnum<RigWellPathFormations::FormationLevel>::setUp()
{
    addItem( RigWellPathFormations::NONE, "NONE", "None" );
    addItem( RigWellPathFormations::ALL, "ALL", "All" );
    addItem( RigWellPathFormations::GROUP, "GROUP", "Formation Group" );
    addItem( RigWellPathFormations::LEVEL0, "LEVEL0", "Formation" );
    addItem( RigWellPathFormations::LEVEL1, "LEVEL1", "Formation 1" );
    addItem( RigWellPathFormations::LEVEL2, "LEVEL2", "Formation 2" );
    addItem( RigWellPathFormations::LEVEL3, "LEVEL3", "Formation 3" );
    addItem( RigWellPathFormations::LEVEL4, "LEVEL4", "Formation 4" );
    addItem( RigWellPathFormations::LEVEL5, "LEVEL5", "Formation 5" );
    addItem( RigWellPathFormations::LEVEL6, "LEVEL6", "Formation 6" );
    addItem( RigWellPathFormations::LEVEL7, "LEVEL7", "Formation 7" );
    addItem( RigWellPathFormations::LEVEL8, "LEVEL8", "Formation 8" );
    addItem( RigWellPathFormations::LEVEL9, "LEVEL9", "Formation 9" );
    addItem( RigWellPathFormations::LEVEL10, "LEVEL10", "Formation 10" );
    setDefault( RigWellPathFormations::ALL );
}

template <>
void AppEnum<RiuPlotAnnotationTool::RegionAnnotationType>::setUp()
{
    addItem( RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS, "NO_ANNOTATIONS", "No Annotations" );
    addItem( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS, "FORMATIONS", "Formations" );
    addItem( RiuPlotAnnotationTool::RegionAnnotationType::CURVE_ANNOTATIONS, "CURVE_DATA", "Curve Data Annotations" );
    addItem( RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS, "RESULT_PROPERTY", "Result Property" );
    setDefault( RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS );
}

template <>
void AppEnum<RiuPlotAnnotationTool::RegionDisplay>::setUp()
{
    addItem( RiuPlotAnnotationTool::DARK_LINES, "DARK_LINES", "Dark Lines" );
    addItem( RiuPlotAnnotationTool::LIGHT_LINES, "LIGHT_LINES", "Light Lines" );
    addItem( RiuPlotAnnotationTool::COLORED_LINES, "COLORED_LINES", "Colored Lines" );
    addItem( RiuPlotAnnotationTool::COLOR_SHADING, "COLOR_SHADING", "Color Shading" );
    addItem( RiuPlotAnnotationTool::COLOR_SHADING_AND_LINES, "SHADING_AND_LINES", "Color Shading and Lines" );
    setDefault( RiuPlotAnnotationTool::COLOR_SHADING_AND_LINES );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::RimWellLogTrack()
    : m_availableXRangeMin( RI_LOGPLOTTRACK_MINX_DEFAULT )
    , m_availableXRangeMax( RI_LOGPLOTTRACK_MAXX_DEFAULT )
    , m_availableDepthRangeMin( RI_LOGPLOTTRACK_MINX_DEFAULT )
    , m_availableDepthRangeMax( RI_LOGPLOTTRACK_MAXX_DEFAULT )

{
    CAF_PDM_InitObject( "Track", ":/WellLogTrack16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_description, "TrackDescription", "Name", "", "", "" );

    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_curves, "Curves", "", "", "", "" );
    m_curves.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_visibleXRangeMin, "VisibleXRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "" );
    CAF_PDM_InitField( &m_visibleXRangeMax, "VisibleXRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "" );
    CAF_PDM_InitField( &m_visibleDepthRangeMin, "VisibleYRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "" );
    CAF_PDM_InitField( &m_visibleDepthRangeMax, "VisibleYRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "" );
    m_visibleDepthRangeMin.uiCapability()->setUiHidden( true );
    m_visibleDepthRangeMin.xmlCapability()->disableIO();
    m_visibleDepthRangeMax.uiCapability()->setUiHidden( true );
    m_visibleDepthRangeMax.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_isAutoScaleXEnabled, "AutoScaleX", true, "Auto Scale", "", "", "" );
    m_isAutoScaleXEnabled.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_isLogarithmicScaleEnabled, "LogarithmicScaleX", false, "Logarithmic Scale", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_xAxisGridVisibility, "ShowXGridLines", "Show Grid Lines", "", "", "" );

    CAF_PDM_InitField( &m_explicitTickIntervals, "ExplicitTickIntervals", false, "Manually Set Tick Intervals", "", "", "" );
    CAF_PDM_InitField( &m_majorTickInterval, "MajorTickIntervals", 0.0, "Major Tick Interval", "", "", "" );
    CAF_PDM_InitField( &m_minorTickInterval, "MinorTickIntervals", 0.0, "Minor Tick Interval", "", "", "" );
    m_majorTickInterval.uiCapability()->setUiHidden( true );
    m_minorTickInterval.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_regionAnnotationType, "AnnotationType", "Region Annotations", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_regionAnnotationDisplay, "RegionDisplay", "Region Display", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_colorShadingPalette, "ColorShadingPalette", "Colors", "", "", "" );
    m_colorShadingPalette = RimRegularLegendConfig::CATEGORY;

    CAF_PDM_InitField( &m_colorShadingTransparency, "ColorShadingTransparency", 50, "Color Transparency", "", "", "" );
    m_colorShadingTransparency.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showFormations_OBSOLETE, "ShowFormations", false, "Show Lines", "", "", "" );
    m_showFormations_OBSOLETE.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitField( &m_showRegionLabels, "ShowFormationLabels", true, "Show Labels", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_formationSource, "FormationSource", "Source", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_formationTrajectoryType, "FormationTrajectoryType", "Trajectory", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_formationWellPathForSourceCase, "FormationWellPath", "Well Path", "", "", "" );
    m_formationWellPathForSourceCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_formationWellPathForSourceWellPath,
                                "FormationWellPathForSourceWellPath",
                                "Well Path",
                                "",
                                "",
                                "" );
    m_formationWellPathForSourceWellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_formationSimWellName, "FormationSimulationWellName", QString( "None" ), "Simulation Well", "", "", "" );
    CAF_PDM_InitField( &m_formationBranchIndex, "FormationBranchIndex", 0, " ", "", "", "" );
    CAF_PDM_InitField( &m_formationBranchDetection,
                       "FormationBranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_formationCase, "FormationCase", "Formation Case", "", "", "" );
    m_formationCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_formationLevel, "FormationLevel", "Well Pick Filter", "", "", "" );

    CAF_PDM_InitField( &m_showformationFluids, "ShowFormationFluids", false, "Show Fluids", "", "", "" );

    CAF_PDM_InitField( &m_showWellPathAttributes, "ShowWellPathAttributes", false, "Show Well Attributes", "", "", "" );
    CAF_PDM_InitField( &m_wellPathAttributesInLegend, "WellPathAttributesInLegend", true, "Attributes in Legend", "", "", "" );
    CAF_PDM_InitField( &m_showWellPathCompletions, "ShowWellPathCompletions", true, "Show Well Completions", "", "", "" );
    CAF_PDM_InitField( &m_wellPathCompletionsInLegend, "WellPathCompletionsInLegend", true, "Completions in Legend", "", "", "" );
    CAF_PDM_InitField( &m_showWellPathComponentsBothSides, "ShowWellPathAttrBothSides", true, "Show Both Sides", "", "", "" );
    CAF_PDM_InitField( &m_showWellPathComponentLabels, "ShowWellPathAttrLabels", false, "Show Labels", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathComponentSource, "AttributesWellPathSource", "Well Path", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathAttributeCollection, "AttributesCollection", "Well Attributes", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "Result Definition", "", "", "" );
    m_resultDefinition.uiCapability()->setUiHidden( true );
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_resultDefinition = new RimEclipseResultDefinition;

    CAF_PDM_InitField( &m_show_OBSOLETE, "Show", false, "Show Plot", "", "", "" );
    m_show_OBSOLETE.uiCapability()->setUiHidden( true );
    m_show_OBSOLETE.xmlCapability()->setIOWritable( false );

    m_formationsForCaseWithSimWellOnly = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::~RimWellLogTrack()
{
    m_curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::description() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::simWellOptionItems( QList<caf::PdmOptionItemInfo>* options, RimCase* rimCase )
{
    CVF_ASSERT( options );
    if ( !options ) return;

    std::set<QString> sortedWellNames;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        sortedWellNames = eclipseCase->eclipseCaseData()->findSortedWellNames();
    }

    caf::IconProvider simWellIcon( ":/Well.png" );
    for ( const QString& wname : sortedWellNames )
    {
        options->push_back( caf::PdmOptionItemInfo( wname, wname, false, simWellIcon ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::cleanupBeforeClose()
{
    detachAllPlotItems();
    if ( m_plotWidget )
    {
        m_plotWidget->deleteLater();
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::detachAllPlotItems()
{
    for ( RimPlotCurve* curve : m_curves )
    {
        curve->detachQwtCurve();
    }
    for ( auto& plotObjects : m_wellPathAttributePlotObjects )
    {
        plotObjects->detachFromQwt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::calculateXZoomRange()
{
    std::map<int, std::vector<RimWellFlowRateCurve*>> stackCurveGroups = visibleStackedCurves();
    for ( const std::pair<int, std::vector<RimWellFlowRateCurve*>>& curveGroup : stackCurveGroups )
    {
        for ( RimWellFlowRateCurve* stCurve : curveGroup.second )
            stCurve->updateStackedPlotData();
    }

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    size_t visibleCurves = 0u;
    for ( auto curve : m_curves )
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if ( curve->isCurveVisible() )
        {
            visibleCurves++;
            if ( curve->xValueRangeInData( &minCurveValue, &maxCurveValue ) )
            {
                if ( minCurveValue < minValue )
                {
                    minValue = minCurveValue;
                }

                if ( maxCurveValue > maxValue )
                {
                    maxValue = maxCurveValue;
                }
            }
        }
    }

    if ( minValue == HUGE_VAL )
    {
        // Empty axis when there are no sensible visible curves
        minValue = 0;
        maxValue = 0;
    }
    else if ( m_minorTickInterval() != 0.0 )
    {
        std::tie( minValue, maxValue ) = adjustXRange( minValue, maxValue, m_minorTickInterval() );
    }

    m_availableXRangeMin = minValue;
    m_availableXRangeMax = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::calculateYZoomRange()
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    for ( RimWellLogCurve* curve : m_curves )
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

        if ( curve->isCurveVisible() && curve->yValueRangeInData( &minCurveDepth, &maxCurveDepth ) )
        {
            if ( minCurveDepth < minDepth )
            {
                minDepth = minCurveDepth;
            }

            if ( maxCurveDepth > maxDepth )
            {
                maxDepth = maxCurveDepth;
            }
        }
    }

    if ( m_showWellPathAttributes || m_showWellPathCompletions )
    {
        for ( const std::unique_ptr<RiuWellPathComponentPlotItem>& plotObject : m_wellPathAttributePlotObjects )
        {
            double minObjectDepth = HUGE_VAL;
            double maxObjectDepth = -HUGE_VAL;
            if ( plotObject->yValueRange( &minObjectDepth, &maxObjectDepth ) )
            {
                if ( minObjectDepth < minDepth )
                {
                    minDepth = minObjectDepth;
                }

                if ( maxObjectDepth > maxDepth )
                {
                    maxDepth = maxObjectDepth;
                }
            }
        }
    }

    m_availableDepthRangeMin = minDepth;
    m_availableDepthRangeMax = maxDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateXZoom()
{
    if ( !m_plotWidget ) return;

    calculateXZoomRange();

    if ( m_isAutoScaleXEnabled )
    {
        m_visibleXRangeMin = m_availableXRangeMin;
        m_visibleXRangeMax = m_availableXRangeMax;

        computeAndSetXRangeMinForLogarithmicScale();
        updateEditors();
    }

    updateXAxisAndGridTickIntervals();

    // Attribute range. Fixed range where well components are positioned [-1, 1].
    // Set an extended range here to allow for some label space.
    double componentRangeMax = 2.0 / ( static_cast<double>( colSpan() ) );
    double componentRangeMin = -0.25;
    if ( m_showWellPathComponentsBothSides )
    {
        componentRangeMin = -1.5;
        componentRangeMax *= 2.0;
    }
    if ( m_showWellPathComponentLabels )
    {
        componentRangeMax *= 1.5;
    }

    m_plotWidget->setAxisRange( QwtPlot::xBottom, componentRangeMin, componentRangeMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateYZoom()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisRange( QwtPlot::yLeft, m_visibleDepthRangeMin(), m_visibleDepthRangeMax() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::doRemoveFromCollection()
{
    RimWellLogPlot* wellLogPlot = nullptr;
    this->firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot )
    {
        wellLogPlot->removePlot( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showWindow )
    {
        if ( m_plotWidget )
        {
            m_plotWidget->setVisible( m_showWindow );
        }

        updateParentLayout();
    }
    else if ( changedField == &m_description )
    {
        updateParentLayout();
    }
    else if ( changedField == &m_explicitTickIntervals )
    {
        if ( m_plotWidget )
        {
            m_majorTickInterval = m_plotWidget->majorTickInterval( QwtPlot::xTop );
            m_minorTickInterval = m_plotWidget->minorTickInterval( QwtPlot::xTop );
        }
        m_majorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );
        m_minorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );
        if ( !m_explicitTickIntervals() )
        {
            updateXAxisAndGridTickIntervals();
        }
    }
    else if ( changedField == &m_xAxisGridVisibility || changedField == &m_majorTickInterval ||
              changedField == &m_minorTickInterval )
    {
        updateXAxisAndGridTickIntervals();
    }
    else if ( changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax )
    {
        bool emptyRange = std::abs( m_visibleXRangeMax() - m_visibleXRangeMin ) <
                          1.0e-6 * std::max( 1.0, std::max( m_visibleXRangeMax(), m_visibleXRangeMin() ) );
        m_explicitTickIntervals.uiCapability()->setUiReadOnly( emptyRange );
        m_xAxisGridVisibility.uiCapability()->setUiReadOnly( emptyRange );

        m_isAutoScaleXEnabled = false;

        updateXZoom();
        m_plotWidget->scheduleReplot();

        updateEditors();
    }
    else if ( changedField == &m_isAutoScaleXEnabled )
    {
        if ( m_isAutoScaleXEnabled() )
        {
            updateXZoom();
            m_plotWidget->scheduleReplot();
        }
    }
    else if ( changedField == &m_isLogarithmicScaleEnabled )
    {
        updateAxisScaleEngine();
        if ( m_isLogarithmicScaleEnabled() )
        {
            m_explicitTickIntervals = false;
        }
        m_explicitTickIntervals.uiCapability()->setUiHidden( m_isLogarithmicScaleEnabled() );

        updateXZoom();
        m_plotWidget->scheduleReplot();
    }
    else if ( changedField == &m_regionAnnotationType || changedField == &m_regionAnnotationDisplay ||
              changedField == &m_formationSource || changedField == &m_colorShadingTransparency ||
              changedField == &m_colorShadingPalette )
    {
        if ( changedField == &m_formationSource && m_formationSource == WELL_PICK_FILTER )
        {
            std::vector<RimWellPath*> wellPaths;
            RimTools::wellPathWithFormations( &wellPaths );
            for ( RimWellPath* wellPath : wellPaths )
            {
                if ( wellPath == m_formationWellPathForSourceCase )
                {
                    m_formationWellPathForSourceWellPath = m_formationWellPathForSourceCase();
                    break;
                }
            }
        }

        loadDataAndUpdate();
        updateParentLayout();
        updateConnectedEditors();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_showRegionLabels )
    {
        loadDataAndUpdate();
    }
    else if ( changedField == &m_formationCase )
    {
        QList<caf::PdmOptionItemInfo> options;
        RimWellLogTrack::simWellOptionItems( &options, m_formationCase );

        if ( options.isEmpty() || m_formationCase == nullptr )
        {
            m_formationSimWellName = QString( "None" );
        }

        loadDataAndUpdate();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_formationWellPathForSourceCase )
    {
        loadDataAndUpdate();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_formationSimWellName )
    {
        loadDataAndUpdate();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_formationTrajectoryType )
    {
        if ( m_formationTrajectoryType == WELL_PATH )
        {
            RimProject* proj                 = RimProject::current();
            m_formationWellPathForSourceCase = proj->wellPathFromSimWellName( m_formationSimWellName );
        }
        else
        {
            if ( m_formationWellPathForSourceCase )
            {
                m_formationSimWellName = m_formationWellPathForSourceCase->associatedSimulationWellName();
            }
        }

        loadDataAndUpdate();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_formationBranchIndex || changedField == &m_formationBranchDetection )
    {
        m_formationBranchIndex = RiaSimWellBranchTools::clampBranchIndex( m_formationSimWellName,
                                                                          m_formationBranchIndex,
                                                                          m_formationBranchDetection );

        loadDataAndUpdate();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_formationWellPathForSourceWellPath )
    {
        loadDataAndUpdate();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_formationLevel )
    {
        loadDataAndUpdate();
    }
    else if ( changedField == &m_showformationFluids )
    {
        loadDataAndUpdate();
    }
    else if ( changedField == &m_showWellPathAttributes || changedField == &m_showWellPathCompletions ||
              changedField == &m_showWellPathComponentsBothSides || changedField == &m_showWellPathComponentLabels ||
              changedField == &m_wellPathAttributesInLegend || changedField == &m_wellPathCompletionsInLegend )
    {
        updateWellPathAttributesOnPlot();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_wellPathComponentSource )
    {
        updateWellPathAttributesCollection();
        updateWellPathAttributesOnPlot();
        updateParentLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateXAxisAndGridTickIntervals()
{
    if ( !m_plotWidget ) return;

    bool emptyRange = std::abs( m_visibleXRangeMax() - m_visibleXRangeMin ) <
                      1.0e-6 * std::max( 1.0, std::max( m_visibleXRangeMax(), m_visibleXRangeMin() ) );

    if ( emptyRange )
    {
        m_plotWidget->enableGridLines( QwtPlot::xTop, false, false );
        m_plotWidget->setAxisRange( QwtPlot::xTop, 0.0, 1.0 );
        m_plotWidget->setAxisLabelsAndTicksEnabled( QwtPlot::xTop, false, false );
    }
    else
    {
        m_plotWidget->setAxisLabelsAndTicksEnabled( QwtPlot::xTop, true, true );
        if ( m_explicitTickIntervals )
        {
            m_plotWidget->setMajorAndMinorTickIntervals( QwtPlot::xTop,
                                                         m_majorTickInterval(),
                                                         m_minorTickInterval(),
                                                         m_visibleXRangeMin(),
                                                         m_visibleXRangeMax() );
        }
        else
        {
            int majorTickIntervals = 5;
            int minorTickIntervals = 10;
            m_plotWidget->setAutoTickIntervalCounts( QwtPlot::xTop, majorTickIntervals, minorTickIntervals );
            m_plotWidget->setAxisRange( QwtPlot::xTop, m_visibleXRangeMin, m_visibleXRangeMax );
        }

        m_plotWidget->enableGridLines( QwtPlot::xTop,
                                       m_xAxisGridVisibility() & RimWellLogPlot::AXIS_GRID_MAJOR,
                                       m_xAxisGridVisibility() & RimWellLogPlot::AXIS_GRID_MINOR );
    }

    RimWellLogPlot* wellLogPlot = nullptr;
    this->firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot )
    {
        m_plotWidget->enableGridLines( QwtPlot::yLeft,
                                       wellLogPlot->depthAxisGridLinesEnabled() & RimWellLogPlot::AXIS_GRID_MAJOR,
                                       wellLogPlot->depthAxisGridLinesEnabled() & RimWellLogPlot::AXIS_GRID_MINOR );
    }
    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateLegend()
{
    reattachAllCurves();
    if ( m_plotWidget )
    {
        m_plotWidget->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::asciiDataForPlotExport() const
{
    QString out = "\n" + this->description() + "\n";

    std::vector<QString>             curveNames;
    std::vector<double>              curveDepths;
    std::vector<std::vector<double>> curvesPlotXValues;

    auto depthType             = parentWellLogPlot()->depthType();
    auto depthUnit             = parentWellLogPlot()->depthUnit();
    bool isWellAllocInflowPlot = false;
    {
        RimWellAllocationPlot* wapl = nullptr;
        parentWellLogPlot()->firstAncestorOfType( wapl );
        if ( wapl )
        {
            isWellAllocInflowPlot = ( wapl->flowType() == RimWellAllocationPlot::INFLOW );
        }
    }

    for ( RimWellLogCurve* curve : m_curves() )
    {
        if ( !curve->isCurveVisible() ) continue;

        const RigWellLogCurveData* curveData = curve->curveData();
        if ( !curveData ) continue;
        curveNames.push_back( curve->curveName() );

        if ( curveNames.size() == 1 )
        {
            curveDepths = curveData->depthPlotValues( depthType, depthUnit );
        }

        std::vector<double> xPlotValues = curveData->xPlotValues();
        if ( curveDepths.size() != xPlotValues.size() || xPlotValues.empty() )
        {
            curveNames.pop_back();

            if ( curveNames.empty() )
            {
                curveDepths.clear();
            }
            continue;
        }
        curvesPlotXValues.push_back( xPlotValues );
    }

    // Header

    if ( depthType == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER )
    {
        out += "Connection";
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
    {
        out += "MD   ";
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::PSEUDO_LENGTH )
    {
        out += "PL   ";
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH )
    {
        out += "TVDMSL  ";
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
    {
        out += "TVDRKB  ";
    }

    for ( QString name : curveNames )
    {
        out += "  \t" + name;
    }
    out += "\n";

    for ( size_t dIdx = 0; dIdx < curveDepths.size(); ++dIdx )
    {
        size_t i          = dIdx;
        double curveDepth = curveDepths[i];

        if ( depthType == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER )
        {
            if ( dIdx == 0 )
                continue; // Skip the first line. (shallow depth, which is last)
                          // as it is a fictious value added to make
                          // the plot easier to read

            i = curveDepths.size() - 1 - dIdx; // Reverse the order, since the connections are coming bottom to top

            if ( i == 0 )
            {
                if ( curveDepths.size() > 1 && curveDepths[i] == curveDepths[i + 1] )
                {
                    continue; // Skip double depth at last connection
                }
            }

            curveDepth = curveDepths[i];

            if ( isWellAllocInflowPlot )
            {
                curveDepth -= 0.5; // To shift the values that was shifted to get the numbers between the changes
            }
        }

        out += QString::number( curveDepth, 'f', 3 );
        for ( std::vector<double> plotVector : curvesPlotXValues )
        {
            out += " \t" + QString::number( plotVector[i], 'g' );
        }
        out += "\n";
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
{
    if ( fontSettingType == RiaDefines::FontSettingType::PLOT_FONT && m_plotWidget )
    {
        return defaultFontSize != m_plotWidget->axisTitleFontSize( QwtPlot::xTop );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                     int                         oldFontSize,
                                     int                         fontSize,
                                     bool                        forceChange /*= false*/ )
{
    if ( fontSettingType == RiaDefines::FontSettingType::PLOT_FONT && m_plotWidget )
    {
        if ( oldFontSize == m_plotWidget->axisTitleFontSize( QwtPlot::xTop ) || forceChange )
        {
            m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xTop, fontSize, fontSize );
            m_plotWidget->setAxisFontsAndAlignment( QwtPlot::yLeft, fontSize, fontSize );
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateZoomFromQwt()
{
    QwtInterval xInterval     = m_plotWidget->axisRange( QwtPlot::xTop );
    QwtInterval depthInterval = m_plotWidget->axisRange( QwtPlot::yLeft );

    m_visibleXRangeMin     = xInterval.minValue();
    m_visibleXRangeMax     = xInterval.maxValue();
    m_visibleDepthRangeMin = depthInterval.minValue();
    m_visibleDepthRangeMax = depthInterval.maxValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::onAxisSelected( int axis, bool toggle )
{
    if ( toggle )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( this );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxes()
{
    updateXZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogTrack::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_regionAnnotationType )
    {
        options.push_back( caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText(
                                                       RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS ),
                                                   RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS ) );
        options.push_back( caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText(
                                                       RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS ),
                                                   RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS ) );
        RimWellBoreStabilityPlot* wellBoreStabilityPlot = nullptr;
        this->firstAncestorOrThisOfType( wellBoreStabilityPlot );
        if ( wellBoreStabilityPlot )
        {
            options.push_back( caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText(
                                                           RiuPlotAnnotationTool::RegionAnnotationType::CURVE_ANNOTATIONS ),
                                                       RiuPlotAnnotationTool::RegionAnnotationType::CURVE_ANNOTATIONS ) );
        }

        options.push_back(
            caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText(
                                        RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS ),
                                    RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS ) );
    }
    if ( fieldNeedingOptions == &m_formationWellPathForSourceCase )
    {
        RimTools::wellPathOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_formationWellPathForSourceWellPath )
    {
        RimTools::wellPathWithFormationsOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_formationCase )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_formationSimWellName )
    {
        RimWellLogTrack::simWellOptionItems( &options, m_formationCase );
    }
    else if ( fieldNeedingOptions == &m_formationBranchIndex )
    {
        auto simulationWellBranches =
            RiaSimWellBranchTools::simulationWellBranches( m_formationSimWellName(), m_formationBranchDetection );
        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField( simulationWellBranches );
    }
    else if ( fieldNeedingOptions == &m_formationLevel )
    {
        if ( m_formationWellPathForSourceWellPath )
        {
            const RigWellPathFormations* formations = m_formationWellPathForSourceWellPath->formationsGeometry();
            if ( formations )
            {
                using FormationLevelEnum = caf::AppEnum<RigWellPathFormations::FormationLevel>;

                options.push_back( caf::PdmOptionItemInfo( FormationLevelEnum::uiText( RigWellPathFormations::NONE ),
                                                           RigWellPathFormations::NONE ) );

                options.push_back( caf::PdmOptionItemInfo( FormationLevelEnum::uiText( RigWellPathFormations::ALL ),
                                                           RigWellPathFormations::ALL ) );

                for ( const RigWellPathFormations::FormationLevel& level : formations->formationsLevelsPresent() )
                {
                    size_t index = FormationLevelEnum::index( level );
                    if ( index >= FormationLevelEnum::size() ) continue;

                    options.push_back( caf::PdmOptionItemInfo( FormationLevelEnum::uiTextFromIndex( index ),
                                                               FormationLevelEnum::fromIndex( index ) ) );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_wellPathComponentSource )
    {
        RimTools::wellPathOptionItems( &options );
        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_colorShadingPalette )
    {
        std::vector<RimRegularLegendConfig::ColorRangesType> rangeTypes;
        rangeTypes.push_back( RimRegularLegendConfig::NORMAL );
        rangeTypes.push_back( RimRegularLegendConfig::OPPOSITE_NORMAL );
        rangeTypes.push_back( RimRegularLegendConfig::WHITE_PINK );
        rangeTypes.push_back( RimRegularLegendConfig::PINK_WHITE );
        rangeTypes.push_back( RimRegularLegendConfig::BLUE_WHITE_RED );
        rangeTypes.push_back( RimRegularLegendConfig::RED_WHITE_BLUE );
        rangeTypes.push_back( RimRegularLegendConfig::WHITE_BLACK );
        rangeTypes.push_back( RimRegularLegendConfig::BLACK_WHITE );
        rangeTypes.push_back( RimRegularLegendConfig::ANGULAR );
        rangeTypes.push_back( RimRegularLegendConfig::CATEGORY );

        for ( RimRegularLegendConfig::ColorRangesType colType : rangeTypes )
        {
            options.push_back(
                caf::PdmOptionItemInfo( RimRegularLegendConfig::ColorRangeEnum::uiText( colType ), colType ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::addCurve( RimWellLogCurve* curve )
{
    m_curves.push_back( curve );

    if ( m_plotWidget )
    {
        curve->setParentQwtPlotAndReplot( m_plotWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::insertCurve( RimWellLogCurve* curve, size_t index )
{
    m_curves.insert( index, curve );
    // Todo: Mark curve data to use either TVD or MD

    if ( m_plotWidget )
    {
        curve->setParentQwtPlotAndReplot( m_plotWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::removeCurve( RimWellLogCurve* curve )
{
    size_t index = m_curves.index( curve );
    if ( index < m_curves.size() )
    {
        m_curves[index]->detachQwtCurve();
        m_curves.removeChildObject( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::deleteAllCurves()
{
    m_curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::availableXAxisRange( double* minX, double* maxX )
{
    calculateXZoomRange();
    *minX = m_availableXRangeMin;
    *maxX = m_availableXRangeMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::availableDepthRange( double* minimumDepth, double* maximumDepth )
{
    calculateYZoomRange();
    *minimumDepth = m_availableDepthRangeMin;
    *maximumDepth = m_availableDepthRangeMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::visibleXAxisRange( double* minX, double* maxX )
{
    CAF_ASSERT( minX && maxX );
    *minX = m_visibleXRangeMin;
    *maxX = m_visibleXRangeMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::visibleDepthRange( double* minDepth, double* maxDepth )
{
    CAF_ASSERT( minDepth && maxDepth );
    *minDepth = m_visibleDepthRangeMin;
    *maxDepth = m_visibleDepthRangeMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::onLoadDataAndUpdate()
{
    RimWellLogPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfType( wellLogPlot );

    if ( wellLogPlot && m_plotWidget )
    {
        m_plotWidget->setAxisTitleText( QwtPlot::xTop, m_xAxisTitle );
        m_plotWidget->setAxisTitleText( QwtPlot::yLeft, wellLogPlot->depthAxisTitle() );
    }

    for ( size_t cIdx = 0; cIdx < m_curves.size(); ++cIdx )
    {
        m_curves[cIdx]->loadDataAndUpdate( false );
    }

    if ( m_regionAnnotationType == RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS ||
         m_regionAnnotationType == RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS )
    {
        m_resultDefinition->loadDataAndUpdate();
        setFormationFieldsUiReadOnly( false );
    }
    else
    {
        setFormationFieldsUiReadOnly( true );
    }
    bool noAnnotations = m_regionAnnotationType() == RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS;
    m_regionAnnotationDisplay.uiCapability()->setUiReadOnly( noAnnotations );
    m_showRegionLabels.uiCapability()->setUiReadOnly( noAnnotations );

    if ( m_plotWidget )
    {
        this->updateWellPathAttributesCollection();
        this->updateWellPathAttributesOnPlot();
        m_plotWidget->updateLegend();

        this->updateAxisScaleEngine();
        this->updateRegionAnnotationsOnPlot();
        this->updateXZoom();
    }

    this->updateXAxisAndGridTickIntervals();
    m_majorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );
    m_minorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );

    bool emptyRange = std::abs( m_visibleXRangeMax() - m_visibleXRangeMin ) <
                      1.0e-6 * std::max( 1.0, std::max( m_visibleXRangeMax(), m_visibleXRangeMin() ) );
    m_explicitTickIntervals.uiCapability()->setUiReadOnly( emptyRange );
    m_xAxisGridVisibility.uiCapability()->setUiReadOnly( emptyRange );

    updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateWellPathFormationNamesData( RimCase* rimCase, RimWellPath* wellPath )
{
    m_formationCase                  = rimCase;
    m_formationTrajectoryType        = RimWellLogTrack::WELL_PATH;
    m_formationWellPathForSourceCase = wellPath;
    m_formationSimWellName           = "";
    m_formationBranchIndex           = -1;

    updateConnectedEditors();

    if ( m_regionAnnotationType != RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS )
    {
        updateRegionAnnotationsOnPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateSimWellFormationNamesAndBranchData( RimCase*       rimCase,
                                                                      const QString& simWellName,
                                                                      int            branchIndex,
                                                                      bool           useBranchDetection )
{
    m_formationBranchIndex     = branchIndex;
    m_formationBranchDetection = useBranchDetection;

    setAndUpdateSimWellFormationNamesData( rimCase, simWellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAndUpdateSimWellFormationNamesData( RimCase* rimCase, const QString& simWellName )
{
    m_formationCase                  = rimCase;
    m_formationTrajectoryType        = RimWellLogTrack::SIMULATION_WELL;
    m_formationWellPathForSourceCase = nullptr;
    m_formationSimWellName           = simWellName;

    updateConnectedEditors();

    if ( m_regionAnnotationType != RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS )
    {
        updateRegionAnnotationsOnPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAutoScaleXEnabled( bool enabled )
{
    m_isAutoScaleXEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAutoScaleYEnabled( bool enabled )
{
    if ( enabled )
    {
        m_visibleDepthRangeMin = m_availableDepthRangeMin;
        m_visibleDepthRangeMax = m_availableDepthRangeMax;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAutoScaleXIfNecessary()
{
    const double eps = 1.0e-8;
    calculateXZoomRange();

    double maxRange = std::max( m_visibleXRangeMax - m_visibleXRangeMin, m_availableXRangeMax - m_availableXRangeMin );

    double maxLow  = std::max( m_visibleXRangeMin(), m_availableXRangeMin );
    double minHigh = std::min( m_visibleXRangeMax(), m_availableXRangeMax );
    double overlap = minHigh - maxLow;

    if ( maxRange < eps || overlap < eps * maxRange )
    {
        setAutoScaleXEnabled( true );
    }

    updateXZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setXAxisTitle( const QString& text )
{
    m_xAxisTitle = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::yAxisTitle() const
{
    RimWellLogPlot* parent;
    this->firstAncestorOrThisOfType( parent );
    if ( parent )
    {
        return parent->depthAxisTitle();
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationWellPath( RimWellPath* wellPath )
{
    m_formationWellPathForSourceCase = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogTrack::formationWellPath() const
{
    return m_formationWellPathForSourceCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationSimWellName( const QString& simWellName )
{
    m_formationSimWellName = simWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::formationSimWellName() const
{
    return m_formationSimWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationBranchDetection( bool branchDetection )
{
    m_formationBranchDetection = branchDetection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::formationBranchDetection() const
{
    return m_formationBranchDetection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationBranchIndex( int branchIndex )
{
    m_formationBranchIndex = branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogTrack::formationBranchIndex() const
{
    return m_formationBranchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationCase( RimCase* rimCase )
{
    m_formationCase = rimCase;
    m_resultDefinition->setEclipseCase( dynamic_cast<RimEclipseCase*>( rimCase ) );
    m_resultDefinition->setPorosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setRegionPropertyResultType( RiaDefines::ResultCatType resultCatType, const QString& resultVariable )
{
    m_resultDefinition->setResultType( resultCatType );
    m_resultDefinition->setResultVariable( resultVariable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogTrack::formationNamesCase() const
{
    return m_formationCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationTrajectoryType( TrajectoryType trajectoryType )
{
    m_formationTrajectoryType = trajectoryType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::TrajectoryType RimWellLogTrack::formationTrajectoryType() const
{
    return m_formationTrajectoryType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimWellLogTrack::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    if ( m_plotWidget == nullptr )
    {
        m_plotWidget = new RiuWellLogTrack( this, mainWindowParent );
        m_plotWidget->setAxisInverted( QwtPlot::yLeft );
        updateAxisScaleEngine();

        for ( size_t cIdx = 0; cIdx < m_curves.size(); ++cIdx )
        {
            m_curves[cIdx]->setParentQwtPlotNoReplot( this->m_plotWidget );
        }
    }
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::detachAllCurves()
{
    detachAllPlotItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::reattachAllCurves()
{
    for ( RimPlotCurve* curve : m_curves )
    {
        curve->reattachQwtCurve();
    }
    for ( auto& plotObjects : m_wellPathAttributePlotObjects )
    {
        plotObjects->reattachToQwt();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateParentPlotZoom()
{
    if ( m_plotWidget )
    {
        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType( wellLogPlot );
        if ( wellLogPlot )
        {
            wellLogPlot->updateZoom();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateEditors()
{
    this->updateConnectedEditors();
    RimPlotWindow* plotWindow = nullptr;

    firstAncestorOrThisOfTypeAsserted( plotWindow );
    plotWindow->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setVisibleXRange( double minValue, double maxValue )
{
    this->setAutoScaleXEnabled( false );
    m_visibleXRangeMin = minValue;
    m_visibleXRangeMax = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setVisibleYRange( double minValue, double maxValue )
{
    m_visibleDepthRangeMin = minValue;
    m_visibleDepthRangeMax = maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateZoomInQwt()
{
    updateXZoom();
    updateYZoom();
    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setTickIntervals( double majorTickInterval, double minorTickInterval )
{
    m_explicitTickIntervals = true;
    m_majorTickInterval     = majorTickInterval;
    m_minorTickInterval     = minorTickInterval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setXAxisGridVisibility( RimWellLogPlot::AxisGridVisibility gridLines )
{
    m_xAxisGridVisibility = gridLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType annotationType )
{
    m_regionAnnotationType = annotationType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAnnotationDisplay( RiuPlotAnnotationTool::RegionDisplay annotationDisplay )
{
    m_regionAnnotationDisplay = annotationDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setAnnotationTransparency( int percent )
{
    m_colorShadingTransparency = percent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAnnotationTool::RegionAnnotationType RimWellLogTrack::annotationType() const
{
    return m_regionAnnotationType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAnnotationTool::RegionDisplay RimWellLogTrack::annotationDisplay() const
{
    return m_regionAnnotationDisplay();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::showFormations() const
{
    return m_regionAnnotationType() == RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowRegionLabels( bool on )
{
    m_showRegionLabels = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::showWellPathAttributes() const
{
    return m_showWellPathAttributes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowWellPathAttributes( bool on )
{
    m_showWellPathAttributes = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowWellPathAttributesInLegend( bool on )
{
    m_wellPathAttributesInLegend = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowWellPathCompletionsInLegend( bool on )
{
    m_wellPathCompletionsInLegend = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setShowBothSidesOfWell( bool on )
{
    m_showWellPathComponentsBothSides = on;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setWellPathAttributesSource( RimWellPath* wellPath )
{
    m_wellPathComponentSource = wellPath;
    updateWellPathAttributesCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogTrack::wellPathAttributeSource() const
{
    return m_wellPathComponentSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellLogTrack::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimWellLogTrack::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellLogTrack::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimWellLogTrack::findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const
{
    for ( size_t idx = 0; idx < m_curves.size(); idx++ )
    {
        if ( m_curves[idx]->qwtPlotCurve() == curve )
        {
            return m_curves[idx];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_description );
    uiOrdering.add( &m_colSpan );

    caf::PdmUiGroup* annotationGroup = uiOrdering.addNewGroup( "Regions/Annotations" );

    annotationGroup->add( &m_regionAnnotationType );
    annotationGroup->add( &m_regionAnnotationDisplay );
    annotationGroup->add( &m_showRegionLabels );

    if ( m_regionAnnotationDisplay() & RiuPlotAnnotationTool::COLOR_SHADING ||
         m_regionAnnotationDisplay() & RiuPlotAnnotationTool::COLORED_LINES )
    {
        annotationGroup->add( &m_colorShadingPalette );
        if ( m_regionAnnotationDisplay() & RiuPlotAnnotationTool::COLOR_SHADING )
        {
            annotationGroup->add( &m_colorShadingTransparency );
        }
    }

    if ( !m_formationsForCaseWithSimWellOnly )
    {
        annotationGroup->add( &m_formationSource );
    }
    else
    {
        m_formationSource = CASE;
    }

    if ( m_formationSource() == CASE )
    {
        annotationGroup->add( &m_formationCase );

        if ( !m_formationsForCaseWithSimWellOnly )
        {
            annotationGroup->add( &m_formationTrajectoryType );

            if ( m_formationTrajectoryType() == WELL_PATH )
            {
                annotationGroup->add( &m_formationWellPathForSourceCase );
            }
        }

        if ( m_formationsForCaseWithSimWellOnly || m_formationTrajectoryType() == SIMULATION_WELL )
        {
            annotationGroup->add( &m_formationSimWellName );

            RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName( annotationGroup,
                                                                                       m_formationSimWellName,
                                                                                       m_formationBranchDetection,
                                                                                       m_formationBranchIndex );
        }
    }
    else if ( m_formationSource() == WELL_PICK_FILTER )
    {
        annotationGroup->add( &m_formationWellPathForSourceWellPath );
        if ( m_formationWellPathForSourceWellPath() )
        {
            annotationGroup->add( &m_formationLevel );
            annotationGroup->add( &m_showformationFluids );
        }
    }

    if ( m_regionAnnotationType() == RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS )
    {
        m_resultDefinition->uiOrdering( uiConfigName, *annotationGroup );
    }

    caf::PdmUiGroup* componentGroup = uiOrdering.addNewGroup( "Well Path Components" );
    componentGroup->add( &m_showWellPathAttributes );
    componentGroup->add( &m_showWellPathCompletions );
    componentGroup->add( &m_wellPathAttributesInLegend );
    componentGroup->add( &m_wellPathCompletionsInLegend );
    componentGroup->add( &m_showWellPathComponentsBothSides );
    componentGroup->add( &m_showWellPathComponentLabels );

    componentGroup->add( &m_wellPathComponentSource );

    uiOrderingForXAxisSettings( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::initAfterRead()
{
    if ( m_showFormations_OBSOLETE() &&
         m_regionAnnotationType() == RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS )
    {
        m_regionAnnotationType    = RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS;
        m_regionAnnotationDisplay = RiuPlotAnnotationTool::DARK_LINES;
    }

    if ( m_regionAnnotationType() == RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_formationCase.value() );
        m_resultDefinition->setEclipseCase( dynamic_cast<RimEclipseCase*>( eclipseCase ) );
    }

    if ( m_xAxisGridVisibility() == RimWellLogPlot::AXIS_GRID_MINOR )
    {
        m_xAxisGridVisibility = RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR;
    }

    if ( m_show_OBSOLETE )
    {
        m_showWindow = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_colorShadingTransparency )
    {
        auto sliderAttrib = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( sliderAttrib )
        {
            sliderAttrib->m_minimum = 0;
            sliderAttrib->m_maximum = 100;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogTrack::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellLogTrack::curveIndex( RimWellLogCurve* curve )
{
    return m_curves.index( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxisScaleEngine()
{
    if ( !m_plotWidget ) return;

    if ( m_isLogarithmicScaleEnabled )
    {
        m_plotWidget->setAxisScaleEngine( QwtPlot::xTop, new QwtLogScaleEngine );

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_plotWidget->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine );
    }
    else
    {
        m_plotWidget->setAxisScaleEngine( QwtPlot::xTop, new RiuQwtLinearScaleEngine );

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_plotWidget->setAxisScaleEngine( QwtPlot::xBottom, new RiuQwtLinearScaleEngine );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimWellLogTrack::adjustXRange( double minValue, double maxValue, double tickInterval )
{
    double minRemainder = std::fmod( minValue, tickInterval );
    double maxRemainder = std::fmod( maxValue, tickInterval );
    double adjustedMin  = minValue - minRemainder;
    double adjustedMax  = maxValue + ( tickInterval - maxRemainder );
    return std::make_pair( adjustedMin, adjustedMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateWellPathAttributesCollection()
{
    m_wellPathAttributeCollection = nullptr;
    if ( m_wellPathComponentSource )
    {
        std::vector<RimWellPathAttributeCollection*> attributeCollection;
        m_wellPathComponentSource->descendantsIncludingThisOfType( attributeCollection );
        if ( !attributeCollection.empty() )
        {
            m_wellPathAttributeCollection = attributeCollection.front();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot* RimWellLogTrack::parentWellLogPlot() const
{
    RimDepthTrackPlot* wellLogPlot = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( wellLogPlot );
    return wellLogPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::handleWheelEvent( QWheelEvent* event )
{
    RimWellLogPlot* wellLogPlot = nullptr;
    this->firstAncestorOrThisOfType( wellLogPlot );

    if ( wellLogPlot )
    {
        if ( event->modifiers() & Qt::ControlModifier )
        {
            QwtScaleMap scaleMap   = m_plotWidget->canvasMap( QwtPlot::yLeft );
            double      zoomCenter = scaleMap.invTransform( event->pos().y() );

            if ( event->delta() > 0 )
            {
                wellLogPlot->setDepthAxisRangeByFactorAndCenter( RI_SCROLLWHEEL_ZOOMFACTOR, zoomCenter );
            }
            else
            {
                wellLogPlot->setDepthAxisRangeByFactorAndCenter( 1.0 / RI_SCROLLWHEEL_ZOOMFACTOR, zoomCenter );
            }
        }
        else
        {
            wellLogPlot->setDepthAxisRangeByPanDepth( event->delta() < 0 ? RI_SCROLLWHEEL_PANFACTOR
                                                                         : -RI_SCROLLWHEEL_PANFACTOR );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimWellLogTrack::waterAndRockRegions( RiaDefines::DepthTypeEnum  depthType,
                                                                             const RigWellLogExtractor* extractor ) const
{
    if ( depthType == RiaDefines::DepthTypeEnum::MEASURED_DEPTH )
    {
        double waterStartMD = 0.0;
        if ( extractor->wellPathData()->rkbDiff() != std::numeric_limits<double>::infinity() )
        {
            waterStartMD += extractor->wellPathData()->rkbDiff();
        }
        double waterEndMD = extractor->cellIntersectionMDs().front();
        double rockEndMD  = extractor->cellIntersectionMDs().back();
        return {{waterStartMD, waterEndMD}, {waterEndMD, rockEndMD}};
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH )
    {
        double waterStartTVD = 0.0;
        double waterEndTVD   = extractor->cellIntersectionTVDs().front();
        double rockEndTVD    = extractor->cellIntersectionTVDs().back();
        return {{waterStartTVD, waterEndTVD}, {waterEndTVD, rockEndTVD}};
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
    {
        double waterStartTVDRKB = extractor->wellPathData()->rkbDiff();
        double waterEndTVDRKB   = extractor->cellIntersectionTVDs().front() + extractor->wellPathData()->rkbDiff();
        double rockEndTVDRKB    = extractor->cellIntersectionTVDs().back() + extractor->wellPathData()->rkbDiff();
        return {{waterStartTVDRKB, waterEndTVDRKB}, {waterEndTVDRKB, rockEndTVDRKB}};
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::computeAndSetXRangeMinForLogarithmicScale()
{
    if ( m_isAutoScaleXEnabled && m_isLogarithmicScaleEnabled )
    {
        double pos = HUGE_VAL;
        double neg = -HUGE_VAL;

        for ( size_t cIdx = 0; cIdx < m_curves.size(); cIdx++ )
        {
            if ( m_curves[cIdx]->isCurveVisible() && m_curves[cIdx]->curveData() )
            {
                RigStatisticsCalculator::posNegClosestToZero( m_curves[cIdx]->curveData()->xPlotValues(), pos, neg );
            }
        }

        if ( pos != HUGE_VAL )
        {
            m_visibleXRangeMin = pos;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setLogarithmicScale( bool enable )
{
    m_isLogarithmicScaleEnabled = enable;

    updateAxisScaleEngine();
    computeAndSetXRangeMinForLogarithmicScale();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, std::vector<RimWellFlowRateCurve*>> RimWellLogTrack::visibleStackedCurves()
{
    std::map<int, std::vector<RimWellFlowRateCurve*>> stackedCurves;
    for ( RimWellLogCurve* curve : m_curves )
    {
        if ( curve && curve->isCurveVisible() )
        {
            RimWellFlowRateCurve* wfrCurve = dynamic_cast<RimWellFlowRateCurve*>( curve );
            if ( wfrCurve != nullptr )
            {
                stackedCurves[wfrCurve->groupId()].push_back( wfrCurve );
            }
        }
    }

    return stackedCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RimWellLogTrack::curves() const
{
    return m_curves.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RimWellLogTrack::visibleCurves() const
{
    std::vector<RimWellLogCurve*> curvesVector;

    for ( RimWellLogCurve* curve : m_curves.childObjects() )
    {
        if ( curve->isCurveVisible() )
        {
            curvesVector.push_back( curve );
        }
    }

    return curvesVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::uiOrderingForRftPltFormations( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* formationGroup = uiOrdering.addNewGroup( "Zonation/Formation Names" );
    formationGroup->setCollapsedByDefault( true );
    formationGroup->add( &m_regionAnnotationType );
    formationGroup->add( &m_regionAnnotationDisplay );
    formationGroup->add( &m_formationSource );
    if ( m_formationSource == CASE )
    {
        formationGroup->add( &m_formationCase );
    }
    if ( m_formationSource == WELL_PICK_FILTER )
    {
        if ( m_formationWellPathForSourceWellPath() && m_formationWellPathForSourceWellPath()->hasFormations() )
        {
            formationGroup->add( &m_formationLevel );
            formationGroup->add( &m_showformationFluids );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::uiOrderingForXAxisSettings( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "X Axis Settings" );
    gridGroup->add( &m_isLogarithmicScaleEnabled );
    gridGroup->add( &m_visibleXRangeMin );
    gridGroup->add( &m_visibleXRangeMax );
    gridGroup->add( &m_xAxisGridVisibility );

    // TODO Revisit if these settings are required
    // See issue https://github.com/OPM/ResInsight/issues/4367
    //     gridGroup->add(&m_explicitTickIntervals);
    //     gridGroup->add(&m_majorTickInterval);
    //     gridGroup->add(&m_minorTickInterval);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationsForCaseWithSimWellOnly( bool caseWithSimWellOnly )
{
    m_formationsForCaseWithSimWellOnly = caseWithSimWellOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogTrack::createSimWellExtractor( RimWellLogPlotCollection* wellLogCollection,
                                                                     RimCase*                  rimCase,
                                                                     const QString&            simWellName,
                                                                     int                       branchIndex,
                                                                     bool                      useBranchDetection )
{
    if ( !wellLogCollection ) return nullptr;

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( !eclipseCase ) return nullptr;

    std::vector<const RigWellPath*> wellPaths =
        RiaSimWellBranchTools::simulationWellBranches( simWellName, useBranchDetection );

    if ( wellPaths.size() == 0 ) return nullptr;

    CVF_ASSERT( branchIndex >= 0 && branchIndex < static_cast<int>( wellPaths.size() ) );

    return ( wellLogCollection->findOrCreateSimWellExtractor( simWellName,
                                                              QString( "Find or create sim well extractor" ),
                                                              wellPaths[branchIndex],
                                                              eclipseCase->eclipseCaseData() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CurveSamplingPointData RimWellLogTrack::curveSamplingPointData( RigEclipseWellLogExtractor* extractor,
                                                                RigResultAccessor*          resultAccessor )
{
    CurveSamplingPointData curveData;

    curveData.md      = extractor->cellIntersectionMDs();
    curveData.tvd     = extractor->cellIntersectionTVDs();
    curveData.rkbDiff = extractor->wellPathData()->rkbDiff();

    extractor->curveData( resultAccessor, &curveData.data );

    return curveData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CurveSamplingPointData RimWellLogTrack::curveSamplingPointData( RigGeoMechWellLogExtractor* extractor,
                                                                const RigFemResultAddress&  resultAddress )
{
    CurveSamplingPointData curveData;

    curveData.md      = extractor->cellIntersectionMDs();
    curveData.tvd     = extractor->cellIntersectionTVDs();
    curveData.rkbDiff = extractor->wellPathData()->rkbDiff();

    extractor->curveData( resultAddress, 0, &curveData.data );
    return curveData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::findRegionNamesToPlot( const CurveSamplingPointData&           curveData,
                                             const std::vector<QString>&             regionNamesVector,
                                             RimWellLogPlot::DepthTypeEnum           depthType,
                                             std::vector<QString>*                   regionNamesToPlot,
                                             std::vector<std::pair<double, double>>* yValues )
{
    if ( regionNamesVector.empty() ) return;

    std::vector<size_t> regionNameIndicesFromCurve;

    for ( double nameIdx : curveData.data )
    {
        if ( nameIdx != std::numeric_limits<double>::infinity() )
        {
            regionNameIndicesFromCurve.push_back( static_cast<size_t>( round( nameIdx ) ) );
        }
        else
        {
            regionNameIndicesFromCurve.push_back( std::numeric_limits<size_t>::max() );
        }
    }

    if ( regionNameIndicesFromCurve.empty() ) return;

    std::vector<double> depthVector;

    if ( depthType == RiaDefines::DepthTypeEnum::MEASURED_DEPTH || depthType == RiaDefines::DepthTypeEnum::PSEUDO_LENGTH )
    {
        depthVector = curveData.md;
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ||
              depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
    {
        depthVector = curveData.tvd;
        if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
        {
            for ( double& depthValue : depthVector )
            {
                depthValue += curveData.rkbDiff;
            }
        }
    }

    if ( depthVector.empty() ) return;

    double currentYStart = depthVector[0];
    size_t prevNameIndex = regionNameIndicesFromCurve[0];
    size_t currentNameIndex;

    for ( size_t i = 1; i < regionNameIndicesFromCurve.size(); i++ )
    {
        currentNameIndex = regionNameIndicesFromCurve[i];
        if ( currentNameIndex != std::numeric_limits<size_t>::max() && currentNameIndex != prevNameIndex )
        {
            if ( prevNameIndex < regionNamesVector.size() )
            {
                regionNamesToPlot->push_back( regionNamesVector[prevNameIndex] );
                yValues->push_back( std::make_pair( currentYStart, depthVector[i - 1] ) );
            }

            currentYStart = depthVector[i];
            prevNameIndex = currentNameIndex;
        }
    }

    size_t lastFormationIdx = regionNameIndicesFromCurve.back();
    if ( lastFormationIdx < regionNamesVector.size() )
    {
        regionNamesToPlot->push_back( regionNamesVector[lastFormationIdx] );
        yValues->push_back( std::make_pair( currentYStart, depthVector.back() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimWellLogTrack::formationNamesVector( RimCase* rimCase )
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( rimCase );

    if ( eclipseCase )
    {
        return eclipseCase->eclipseCaseData()->formationNames();
    }
    else if ( geoMechCase )
    {
        return geoMechCase->geoMechData()->femPartResults()->formationNames();
    }

    return std::vector<QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setFormationFieldsUiReadOnly( bool readOnly /*= true*/ )
{
    m_formationSource.uiCapability()->setUiReadOnly( readOnly );
    m_formationTrajectoryType.uiCapability()->setUiReadOnly( readOnly );
    m_formationSimWellName.uiCapability()->setUiReadOnly( readOnly );
    m_formationCase.uiCapability()->setUiReadOnly( readOnly );
    m_formationWellPathForSourceCase.uiCapability()->setUiReadOnly( readOnly );
    m_formationWellPathForSourceWellPath.uiCapability()->setUiReadOnly( readOnly );
    m_formationBranchDetection.uiCapability()->setUiReadOnly( readOnly );
    m_formationBranchIndex.uiCapability()->setUiReadOnly( readOnly );
    m_formationLevel.uiCapability()->setUiReadOnly( readOnly );
    m_showformationFluids.uiCapability()->setUiReadOnly( readOnly );
    m_colorShadingTransparency.uiCapability()->setUiReadOnly( readOnly );
    m_colorShadingPalette.uiCapability()->setUiReadOnly( readOnly );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateRegionAnnotationsOnPlot()
{
    removeRegionAnnotations();

    if ( m_regionAnnotationType == RiuPlotAnnotationTool::RegionAnnotationType::NO_ANNOTATIONS ) return;

    if ( m_annotationTool == nullptr )
    {
        m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>( new RiuPlotAnnotationTool() );
    }

    if ( m_regionAnnotationType == RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS )
    {
        updateFormationNamesOnPlot();
    }
    else if ( m_regionAnnotationType == RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS )
    {
        updateResultPropertyNamesOnPlot();
    }
    else
    {
        updateCurveDataRegionsOnPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateFormationNamesOnPlot()
{
    RimDepthTrackPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );

    if ( m_formationSource() == WELL_PICK_FILTER )
    {
        if ( m_formationWellPathForSourceWellPath == nullptr ) return;

        if ( !( plot->depthType() == RiaDefines::DepthTypeEnum::MEASURED_DEPTH ||
                plot->depthType() == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ||
                plot->depthType() == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB ) )
        {
            return;
        }

        std::vector<double> yValues;

        const RigWellPathFormations* formations = m_formationWellPathForSourceWellPath->formationsGeometry();
        if ( !formations ) return;

        std::vector<QString> formationNamesToPlot;
        formations->depthAndFormationNamesUpToLevel( m_formationLevel(),
                                                     &formationNamesToPlot,
                                                     &yValues,
                                                     m_showformationFluids(),
                                                     plot->depthType() );

        if ( plot->depthType() == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
        {
            for ( double& depthValue : yValues )
            {
                depthValue += m_formationWellPathForSourceWellPath->wellPathGeometry()->rkbDiff();
            }
        }

        m_annotationTool->attachWellPicks( m_plotWidget, formationNamesToPlot, yValues );
    }
    else
    {
        RimMainPlotCollection* mainPlotCollection;
        this->firstAncestorOrThisOfTypeAsserted( mainPlotCollection );

        RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();

        CurveSamplingPointData curveData;

        RigEclipseWellLogExtractor* eclWellLogExtractor     = nullptr;
        RigGeoMechWellLogExtractor* geoMechWellLogExtractor = nullptr;
        RigWellLogExtractor*        extractor               = nullptr;

        if ( m_formationTrajectoryType == SIMULATION_WELL )
        {
            eclWellLogExtractor = RimWellLogTrack::createSimWellExtractor( wellLogCollection,
                                                                           m_formationCase,
                                                                           m_formationSimWellName,
                                                                           m_formationBranchIndex,
                                                                           m_formationBranchDetection );
        }
        else
        {
            eclWellLogExtractor =
                RiaExtractionTools::wellLogExtractorEclipseCase( m_formationWellPathForSourceCase,
                                                                 dynamic_cast<RimEclipseCase*>( m_formationCase() ) );
        }

        if ( eclWellLogExtractor )
        {
            RimEclipseCase*             eclipseCase    = dynamic_cast<RimEclipseCase*>( m_formationCase() );
            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::
                createFromResultAddress( eclipseCase->eclipseCaseData(),
                                         0,
                                         RiaDefines::PorosityModelType::MATRIX_MODEL,
                                         0,
                                         RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                                  RiaDefines::activeFormationNamesResultName() ) );

            curveData = RimWellLogTrack::curveSamplingPointData( eclWellLogExtractor, resultAccessor.p() );
            extractor = eclWellLogExtractor;
        }
        else
        {
            geoMechWellLogExtractor =
                RiaExtractionTools::wellLogExtractorGeoMechCase( m_formationWellPathForSourceCase,
                                                                 dynamic_cast<RimGeoMechCase*>( m_formationCase() ) );
            if ( !geoMechWellLogExtractor ) return;

            std::string activeFormationNamesResultName = RiaDefines::activeFormationNamesResultName().toStdString();
            curveData = RimWellLogTrack::curveSamplingPointData( geoMechWellLogExtractor,
                                                                 RigFemResultAddress( RIG_FORMATION_NAMES,
                                                                                      activeFormationNamesResultName,
                                                                                      "" ) );
            extractor = geoMechWellLogExtractor;
        }

        // Attach water and rock base formations
        const std::pair<double, double> xRange = std::make_pair( m_visibleXRangeMin(), m_visibleXRangeMax() );

        if ( geoMechWellLogExtractor )
        {
            const caf::ColorTable waterAndRockColors = RiaColorTables::waterAndRockPaletteColors();
            const std::vector<std::pair<double, double>> waterAndRockIntervals =
                waterAndRockRegions( plot->depthType(), extractor );
            m_annotationTool->attachNamedRegions( m_plotWidget,
                                                  {"Sea Level", ""},
                                                  xRange,
                                                  waterAndRockIntervals,
                                                  m_regionAnnotationDisplay(),
                                                  waterAndRockColors,
                                                  ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100,
                                                  m_showRegionLabels(),
                                                  RiuPlotAnnotationTool::TrackSpan::LEFT_COLUMN,
                                                  {Qt::SolidPattern, Qt::Dense6Pattern} );
        }

        if ( m_formationSource == CASE )
        {
            if ( ( m_formationSimWellName == QString( "None" ) && m_formationWellPathForSourceCase == nullptr ) ||
                 m_formationCase == nullptr )
                return;

            std::vector<std::pair<double, double>> yValues;
            std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( m_formationCase );

            std::vector<QString> formationNamesToPlot;
            RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                    formationNamesVector,
                                                    plot->depthType(),
                                                    &formationNamesToPlot,
                                                    &yValues );

            caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

            m_annotationTool->attachNamedRegions( m_plotWidget,
                                                  formationNamesToPlot,
                                                  xRange,
                                                  yValues,
                                                  m_regionAnnotationDisplay(),
                                                  colorTable,
                                                  ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100,
                                                  m_showRegionLabels() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateResultPropertyNamesOnPlot()
{
    RimDepthTrackPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );

    RigEclipseWellLogExtractor* eclWellLogExtractor =
        RiaExtractionTools::wellLogExtractorEclipseCase( m_formationWellPathForSourceCase,
                                                         dynamic_cast<RimEclipseCase*>( m_formationCase() ) );

    if ( !eclWellLogExtractor )
    {
        RiaLogging::error( "No well log extractor found for case." );
        return;
    }

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_formationCase() );

    m_resultDefinition->loadResult();

    size_t                      m_timeStep = 0;
    cvf::ref<RigResultAccessor> resultAccessor =
        RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(), 0, m_timeStep, m_resultDefinition );
    if ( !resultAccessor.notNull() )
    {
        RiaLogging::error( "Unable to get result accessor" );
        return;
    }

    CurveSamplingPointData curveData = RimWellLogTrack::curveSamplingPointData( eclWellLogExtractor, resultAccessor.p() );

    // Attach water and rock base formations
    const std::pair<double, double> xRange = std::make_pair( m_visibleXRangeMin(), m_visibleXRangeMax() );

    if ( m_formationSource == CASE )
    {
        if ( ( m_formationSimWellName == QString( "None" ) && m_formationWellPathForSourceCase == nullptr ) ||
             m_formationCase == nullptr )
            return;

        RimProject*                  proj                  = RimProject::current();
        RimColorLegendCollection*    colorLegendCollection = proj->colorLegendCollection;
        std::vector<RimColorLegend*> legends               = colorLegendCollection->customColorLegends();
        if ( legends.empty() )
        {
            RiaLogging::error( "No color legend found." );
            return;
        }

        // TODO: let the user select the color legend instead of just picking the first one...
        std::vector<cvf::Color3ub> colors;
        std::vector<QString>       namesVector;
        for ( RimColorLegendItem* legendItem : legends[0]->colorLegendItems() )
        {
            namesVector.push_back( legendItem->categoryName() );
        }

        std::vector<QString>                   namesToPlot;
        std::vector<std::pair<double, double>> yValues;
        RimWellLogTrack::findRegionNamesToPlot( curveData, namesVector, plot->depthType(), &namesToPlot, &yValues );

        // TODO: unecessarily messy!
        // Need to map colors to names (since a category can be used several times)
        for ( QString nameToPlot : namesToPlot )
        {
            for ( RimColorLegendItem* legendItem : legends[0]->colorLegendItems() )
            {
                if ( legendItem->categoryName() == nameToPlot )
                    colors.push_back( cvf::Color3ub( legendItem->color() ) );
            }
        }

        if ( colors.empty() )
        {
            RiaLogging::error( "No colors found." );
            return;
        }

        caf::ColorTable colorTable( colors );
        m_annotationTool->attachNamedRegions( m_plotWidget,
                                              namesToPlot,
                                              xRange,
                                              yValues,
                                              m_regionAnnotationDisplay(),
                                              colorTable,
                                              ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100,
                                              m_showRegionLabels() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateCurveDataRegionsOnPlot()
{
    RimWellBoreStabilityPlot* wellBoreStabilityPlot = nullptr;
    this->firstAncestorOrThisOfType( wellBoreStabilityPlot );
    if ( wellBoreStabilityPlot )
    {
        wellBoreStabilityPlot->updateCommonDataSource();
        RimGeoMechCase* geoMechCase =
            dynamic_cast<RimGeoMechCase*>( wellBoreStabilityPlot->commonDataSource()->caseToApply() );
        RimWellPath* wellPath = wellBoreStabilityPlot->commonDataSource()->wellPathToApply();
        int          timeStep = wellBoreStabilityPlot->commonDataSource()->timeStepToApply();
        if ( geoMechCase && wellPath && timeStep >= 0 )
        {
            RigGeoMechWellLogExtractor* geoMechWellLogExtractor = nullptr;
            geoMechWellLogExtractor =
                RiaExtractionTools::wellLogExtractorGeoMechCase( wellPath, dynamic_cast<RimGeoMechCase*>( geoMechCase ) );
            if ( !geoMechWellLogExtractor ) return;

            std::pair<double, double> xRange = std::make_pair( m_visibleXRangeMin(), m_visibleXRangeMax() );

            CurveSamplingPointData curveData;
            curveData.md  = geoMechWellLogExtractor->cellIntersectionMDs();
            curveData.tvd = geoMechWellLogExtractor->cellIntersectionTVDs();

            RimWellLogExtractionCurve::findAndLoadWbsParametersFromLasFiles( wellPath, geoMechWellLogExtractor );
            RimWellBoreStabilityPlot* wbsPlot;
            this->firstAncestorOrThisOfType( wbsPlot );
            if ( wbsPlot )
            {
                wbsPlot->applyWbsParametersToExtractor( geoMechWellLogExtractor );
            }

            std::vector<double> ppSourceRegions      = geoMechWellLogExtractor->porePressureSourceRegions( timeStep );
            std::vector<double> poissonSourceRegions = geoMechWellLogExtractor->poissonSourceRegions( timeStep );
            std::vector<double> ucsSourceRegions     = geoMechWellLogExtractor->ucsSourceRegions( timeStep );

            {
                caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

                std::vector<QString> sourceNames =
                    RigWbsParameter::PP_Reservoir().allSourceUiLabels( "\n",
                                                                       wbsPlot->userDefinedValue(
                                                                           RigWbsParameter::PP_NonReservoir() ) );
                curveData.data = ppSourceRegions;

                std::vector<QString>                   sourceNamesToPlot;
                std::vector<std::pair<double, double>> yValues;
                RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                        sourceNames,
                                                        wellBoreStabilityPlot->depthType(),
                                                        &sourceNamesToPlot,
                                                        &yValues );
                m_annotationTool->attachNamedRegions( m_plotWidget,
                                                      sourceNamesToPlot,
                                                      xRange,
                                                      yValues,
                                                      m_regionAnnotationDisplay(),
                                                      colorTable,
                                                      ( ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100 ) / 3,
                                                      m_showRegionLabels(),
                                                      RiuPlotAnnotationTool::TrackSpan::LEFT_COLUMN );
            }
            {
                caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

                std::vector<QString> sourceNames =
                    RigWbsParameter::poissonRatio().allSourceUiLabels( "\n",
                                                                       wbsPlot->userDefinedValue(
                                                                           RigWbsParameter::poissonRatio() ) );
                curveData.data = poissonSourceRegions;

                std::vector<QString>                   sourceNamesToPlot;
                std::vector<std::pair<double, double>> yValues;
                RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                        sourceNames,
                                                        wellBoreStabilityPlot->depthType(),
                                                        &sourceNamesToPlot,
                                                        &yValues );
                m_annotationTool->attachNamedRegions( m_plotWidget,
                                                      sourceNamesToPlot,
                                                      xRange,
                                                      yValues,
                                                      m_regionAnnotationDisplay(),
                                                      colorTable,
                                                      ( ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100 ) / 3,
                                                      m_showRegionLabels(),
                                                      RiuPlotAnnotationTool::TrackSpan::CENTRE_COLUMN );
            }
            {
                caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

                std::vector<QString> sourceNames =
                    RigWbsParameter::UCS().allSourceUiLabels( "\n", wbsPlot->userDefinedValue( RigWbsParameter::UCS() ) );

                curveData.data = ucsSourceRegions;

                std::vector<QString>                   sourceNamesToPlot;
                std::vector<std::pair<double, double>> yValues;
                RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                        sourceNames,
                                                        wellBoreStabilityPlot->depthType(),
                                                        &sourceNamesToPlot,
                                                        &yValues );
                m_annotationTool->attachNamedRegions( m_plotWidget,
                                                      sourceNamesToPlot,
                                                      xRange,
                                                      yValues,
                                                      m_regionAnnotationDisplay(),
                                                      colorTable,
                                                      ( ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100 ) / 3,
                                                      m_showRegionLabels(),
                                                      RiuPlotAnnotationTool::TrackSpan::RIGHT_COLUMN );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateWellPathAttributesOnPlot()
{
    m_wellPathAttributePlotObjects.clear();

    if ( wellPathAttributeSource() )
    {
        std::vector<const RimWellPathComponentInterface*> allWellPathComponents;

        if ( wellPathAttributeSource()->wellPathGeometry() && ( m_showWellPathAttributes || m_showWellPathCompletions ) )
        {
            m_wellPathAttributePlotObjects.push_back( std::unique_ptr<RiuWellPathComponentPlotItem>(
                new RiuWellPathComponentPlotItem( wellPathAttributeSource() ) ) );
        }

        if ( m_showWellPathAttributes )
        {
            if ( m_wellPathAttributeCollection )
            {
                std::vector<RimWellPathAttribute*> attributes = m_wellPathAttributeCollection->attributes();
                for ( const RimWellPathAttribute* attribute : attributes )
                {
                    if ( attribute->isEnabled() )
                    {
                        allWellPathComponents.push_back( attribute );
                    }
                }
            }
        }
        if ( m_showWellPathCompletions )
        {
            const RimWellPathCompletions* completionsCollection              = wellPathAttributeSource()->completions();
            std::vector<const RimWellPathComponentInterface*> allCompletions = completionsCollection->allCompletions();

            for ( const RimWellPathComponentInterface* completion : allCompletions )
            {
                if ( completion->isEnabled() )
                {
                    allWellPathComponents.push_back( completion );
                }
            }
        }

        const std::map<RiaDefines::WellPathComponentType, int> sortIndices =
            {{RiaDefines::WellPathComponentType::WELL_PATH, 0},
             {RiaDefines::WellPathComponentType::CASING, 1},
             {RiaDefines::WellPathComponentType::LINER, 2},
             {RiaDefines::WellPathComponentType::PERFORATION_INTERVAL, 3},
             {RiaDefines::WellPathComponentType::FISHBONES, 4},
             {RiaDefines::WellPathComponentType::FRACTURE, 5},
             {RiaDefines::WellPathComponentType::PACKER, 6},
             {RiaDefines::WellPathComponentType::ICD, 7},
             {RiaDefines::WellPathComponentType::AICD, 8},
             {RiaDefines::WellPathComponentType::ICV, 9}};

        std::stable_sort( allWellPathComponents.begin(),
                          allWellPathComponents.end(),
                          [&sortIndices]( const RimWellPathComponentInterface* lhs,
                                          const RimWellPathComponentInterface* rhs ) {
                              return sortIndices.at( lhs->componentType() ) < sortIndices.at( rhs->componentType() );
                          } );

        std::set<QString> completionsAssignedToLegend;
        for ( const RimWellPathComponentInterface* component : allWellPathComponents )
        {
            std::unique_ptr<RiuWellPathComponentPlotItem> plotItem(
                new RiuWellPathComponentPlotItem( wellPathAttributeSource(), component ) );
            QString legendTitle     = plotItem->legendTitle();
            bool contributeToLegend = m_wellPathCompletionsInLegend() && !completionsAssignedToLegend.count( legendTitle );
            plotItem->setContributeToLegend( contributeToLegend );
            m_wellPathAttributePlotObjects.push_back( std::move( plotItem ) );
            completionsAssignedToLegend.insert( legendTitle );
        }

        RimDepthTrackPlot* wellLogPlot;
        this->firstAncestorOrThisOfTypeAsserted( wellLogPlot );
        RimWellLogPlot::DepthTypeEnum depthType = wellLogPlot->depthType();

        for ( auto& attributePlotObject : m_wellPathAttributePlotObjects )
        {
            attributePlotObject->setDepthType( depthType );
            attributePlotObject->setShowLabel( m_showWellPathComponentLabels() );
            attributePlotObject->loadDataAndUpdate( false );
            attributePlotObject->setParentQwtPlotNoReplot( m_plotWidget );
        }
    }
    updateXZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::removeRegionAnnotations()
{
    if ( m_annotationTool )
    {
        m_annotationTool->detachAllAnnotations();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::doUpdateLayout()
{
    m_plotWidget->scheduleReplot();
}

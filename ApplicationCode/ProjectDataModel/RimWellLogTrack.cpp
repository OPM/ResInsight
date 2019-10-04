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

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaExtractionTools.h"
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
#include "RimEclipseCase.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimGeoMechCase.h"
#include "RimMainPlotCollection.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimTools.h"
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
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"
#include "RiuWellPathComponentPlotItem.h"

#include "RiuQwtLinearScaleEngine.h"

#include "cafPdmUiSliderEditor.h"
#include "cvfAssert.h"

#define RI_LOGPLOTTRACK_MINX_DEFAULT -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT 100.0
#define RI_LOGPLOTTRACK_MINOR_TICK_DEFAULT

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
void AppEnum<RimWellLogTrack::WidthScaleFactor>::setUp()
{
    addItem( RimWellLogTrack::EXTRA_NARROW_TRACK, "EXTRA_NARROW_TRACK", "Extra Narrow" );
    addItem( RimWellLogTrack::NARROW_TRACK, "NARROW_TRACK", "Narrow" );
    addItem( RimWellLogTrack::NORMAL_TRACK, "NORMAL_TRACK", "Normal" );
    addItem( RimWellLogTrack::WIDE_TRACK, "WIDE_TRACK", "Wide" );
    addItem( RimWellLogTrack::EXTRA_WIDE_TRACK, "EXTRA_WIDE_TRACK", "Extra wide" );
    setDefault( RimWellLogTrack::NORMAL_TRACK );
}

template <>
void AppEnum<RiuPlotAnnotationTool::RegionAnnotationType>::setUp()
{
    addItem( RiuPlotAnnotationTool::NO_ANNOTATIONS, "NO_ANNOTATIONS", "No Annotations" );
    addItem( RiuPlotAnnotationTool::FORMATION_ANNOTATIONS, "FORMATIONS", "Formations" );
    addItem( RiuPlotAnnotationTool::CURVE_ANNOTATIONS, "CURVE_DATA", "Curve Data Annotations" );
    setDefault( RiuPlotAnnotationTool::NO_ANNOTATIONS );
}

template <>
void AppEnum<RiuPlotAnnotationTool::RegionDisplay>::setUp()
{
    addItem( RiuPlotAnnotationTool::DARK_LINES, "DARK_LINES", "Dark Lines" );
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
{
    CAF_PDM_InitObject( "Track", ":/WellLogTrack16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_userName, "TrackDescription", "Name", "", "", "" );
    m_userName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_show, "Show", true, "Show Track", "", "", "" );
    m_show.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &curves, "Curves", "", "", "", "" );
    curves.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_visibleXRangeMin, "VisibleXRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "" );
    CAF_PDM_InitField( &m_visibleXRangeMax, "VisibleXRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "" );

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

    CAF_PDM_InitField( &m_formationSimWellName,
                       "FormationSimulationWellName",
                       QString( "None" ),
                       "Simulation Well",
                       "",
                       "",
                       "" );
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
    CAF_PDM_InitField( &m_wellPathAttributesInLegend, "WellPathAttributesInLegend", false, "Attributes in Legend", "", "", "" );
    CAF_PDM_InitField( &m_showWellPathCompletions, "ShowWellPathCompletions", true, "Show Well Completions", "", "", "" );
    CAF_PDM_InitField( &m_wellPathCompletionsInLegend,
                       "WellPathCompletionsInLegend",
                       false,
                       "Completions in Legend",
                       "",
                       "",
                       "" );
    CAF_PDM_InitField( &m_showWellPathComponentsBothSides, "ShowWellPathAttrBothSides", true, "Show Both Sides", "", "", "" );
    CAF_PDM_InitField( &m_showWellPathComponentLabels, "ShowWellPathAttrLabels", false, "Show Labels", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathComponentSource, "AttributesWellPathSource", "Well Path", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPathAttributeCollection, "AttributesCollection", "Well Attributes", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_widthScaleFactor, "Width", "Track Width", "", "Set width of track. ", "" );

    m_formationsForCaseWithSimWellOnly = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack::~RimWellLogTrack()
{
    curves.deleteAllChildObjects();

    if ( m_wellLogTrackPlotWidget )
    {
        m_wellLogTrackPlotWidget->deleteLater();
        m_wellLogTrackPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setDescription( const QString& description )
{
    m_userName = description;
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

    caf::QIconProvider simWellIcon( ":/Well.png" );
    for ( const QString& wname : sortedWellNames )
    {
        options->push_back( caf::PdmOptionItemInfo( wname, wname, false, simWellIcon ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    if ( changedField == &m_show )
    {
        if ( m_wellLogTrackPlotWidget )
        {
            m_wellLogTrackPlotWidget->setVisible( m_show() );
        }

        updateParentPlotLayout();
    }
    else if ( changedField == &m_widthScaleFactor )
    {
        updateParentPlotLayout();
        updateAxisAndGridTickIntervals();
        applyXZoomFromVisibleRange();
    }
    else if ( changedField == &m_explicitTickIntervals )
    {
        if ( m_wellLogTrackPlotWidget )
        {
            m_majorTickInterval = m_wellLogTrackPlotWidget->getCurrentMajorTickInterval();
            m_minorTickInterval = m_wellLogTrackPlotWidget->getCurrentMinorTickInterval();
        }
        m_majorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );
        m_minorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );
        if ( !m_explicitTickIntervals() )
        {
            updateAxisAndGridTickIntervals();
        }
    }
    else if ( changedField == &m_xAxisGridVisibility || changedField == &m_majorTickInterval ||
              changedField == &m_minorTickInterval )
    {
        updateAxisAndGridTickIntervals();
    }
    else if ( changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax )
    {
        m_wellLogTrackPlotWidget->setXRange( m_visibleXRangeMin, m_visibleXRangeMax );
        m_wellLogTrackPlotWidget->replot();
        m_isAutoScaleXEnabled = false;
        bool emptyRange       = std::abs( m_visibleXRangeMax() - m_visibleXRangeMin ) <
                          1.0e-6 * std::max( 1.0, std::max( m_visibleXRangeMax(), m_visibleXRangeMin() ) );
        m_explicitTickIntervals.uiCapability()->setUiReadOnly( emptyRange );
        m_xAxisGridVisibility.uiCapability()->setUiReadOnly( emptyRange );

        updateEditors();
        updateParentPlotLayout();
        updateAxisAndGridTickIntervals();
    }
    else if ( changedField == &m_isAutoScaleXEnabled )
    {
        if ( m_isAutoScaleXEnabled() )
        {
            this->calculateXZoomRangeAndUpdateQwt();
            computeAndSetXRangeMinForLogarithmicScale();

            if ( m_wellLogTrackPlotWidget ) m_wellLogTrackPlotWidget->replot();
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

        this->calculateXZoomRangeAndUpdateQwt();
        computeAndSetXRangeMinForLogarithmicScale();

        m_wellLogTrackPlotWidget->setXRange( m_visibleXRangeMin, m_visibleXRangeMax );

        m_wellLogTrackPlotWidget->replot();
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

        loadDataAndUpdate( true );

        RimWellRftPlot* rftPlot( nullptr );

        firstAncestorOrThisOfType( rftPlot );

        if ( rftPlot )
        {
            rftPlot->updateConnectedEditors();
        }
        else
        {
            RimWellPltPlot* pltPlot( nullptr );
            firstAncestorOrThisOfType( pltPlot );

            if ( pltPlot )
            {
                pltPlot->updateConnectedEditors();
            }
        }
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

        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_formationWellPathForSourceCase )
    {
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_formationSimWellName )
    {
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_formationTrajectoryType )
    {
        if ( m_formationTrajectoryType == WELL_PATH )
        {
            RimProject* proj                 = RiaApplication::instance()->project();
            m_formationWellPathForSourceCase = proj->wellPathFromSimWellName( m_formationSimWellName );
        }
        else
        {
            if ( m_formationWellPathForSourceCase )
            {
                m_formationSimWellName = m_formationWellPathForSourceCase->associatedSimulationWellName();
            }
        }

        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_formationBranchIndex || changedField == &m_formationBranchDetection )
    {
        m_formationBranchIndex = RiaSimWellBranchTools::clampBranchIndex( m_formationSimWellName,
                                                                          m_formationBranchIndex,
                                                                          m_formationBranchDetection );

        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_formationWellPathForSourceWellPath )
    {
        loadDataAndUpdate( true );
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
        updateParentPlotLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
    else if ( changedField == &m_wellPathComponentSource )
    {
        updateWellPathAttributesCollection();
        updateWellPathAttributesOnPlot();
        updateParentPlotLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateParentPlotLayout()
{
    RimWellLogPlot* wellLogPlot;
    this->firstAncestorOrThisOfType( wellLogPlot );
    if ( wellLogPlot )
    {
        RiuWellLogPlot* wellLogPlotViewer = dynamic_cast<RiuWellLogPlot*>( wellLogPlot->viewWidget() );
        if ( wellLogPlotViewer )
        {
            wellLogPlotViewer->updateChildrenLayout();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxisAndGridTickIntervals()
{
    if ( !m_wellLogTrackPlotWidget ) return;

    if ( m_explicitTickIntervals )
    {
        m_wellLogTrackPlotWidget->setMajorAndMinorTickIntervals( m_majorTickInterval(), m_minorTickInterval() );
    }
    else
    {
        int xMajorTickIntervals = 3;
        int xMinorTickIntervals = 0;
        switch ( m_widthScaleFactor() )
        {
            case EXTRA_NARROW_TRACK:
                xMajorTickIntervals = 3;
                xMinorTickIntervals = 2;
                break;
            case NARROW_TRACK:
                xMajorTickIntervals = 3;
                xMinorTickIntervals = 5;
                break;
            case NORMAL_TRACK:
                xMajorTickIntervals = 5;
                xMinorTickIntervals = 5;
                break;
            case WIDE_TRACK:
                xMajorTickIntervals = 5;
                xMinorTickIntervals = 10;
                break;
            case EXTRA_WIDE_TRACK:
                xMajorTickIntervals = 10;
                xMinorTickIntervals = 10;
                break;
        }
        m_wellLogTrackPlotWidget->setAutoTickIntervalCounts( xMajorTickIntervals, xMinorTickIntervals );
    }

    switch ( m_xAxisGridVisibility() )
    {
        case RimWellLogPlot::AXIS_GRID_NONE:
            m_wellLogTrackPlotWidget->enableXGridLines( false, false );
            break;
        case RimWellLogPlot::AXIS_GRID_MAJOR:
            m_wellLogTrackPlotWidget->enableXGridLines( true, false );
            break;
        case RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR:
            m_wellLogTrackPlotWidget->enableXGridLines( true, true );
            break;
    }

    RimWellLogPlot* plot = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( plot );
    switch ( plot->depthGridLinesVisibility() )
    {
        case RimWellLogPlot::AXIS_GRID_NONE:
            m_wellLogTrackPlotWidget->enableDepthGridLines( false, false );
            break;
        case RimWellLogPlot::AXIS_GRID_MAJOR:
            m_wellLogTrackPlotWidget->enableDepthGridLines( true, false );
            break;
        case RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR:
            m_wellLogTrackPlotWidget->enableDepthGridLines( true, true );
            break;
    }
    m_wellLogTrackPlotWidget->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAllLegendItems()
{
    reattachAllCurves();
    if ( m_wellLogTrackPlotWidget )
    {
        m_wellLogTrackPlotWidget->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogTrack::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( options.size() > 0 ) return options;

    if ( fieldNeedingOptions == &m_regionAnnotationType )
    {
        options.push_back(
            caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText( RiuPlotAnnotationTool::NO_ANNOTATIONS ),
                                    RiuPlotAnnotationTool::NO_ANNOTATIONS ) );
        options.push_back(
            caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText( RiuPlotAnnotationTool::FORMATION_ANNOTATIONS ),
                                    RiuPlotAnnotationTool::FORMATION_ANNOTATIONS ) );
        RimWellBoreStabilityPlot* wellBoreStabilityPlot = nullptr;
        this->firstAncestorOrThisOfType( wellBoreStabilityPlot );
        if ( wellBoreStabilityPlot )
        {
            options.push_back(
                caf::PdmOptionItemInfo( RegionAnnotationTypeEnum::uiText( RiuPlotAnnotationTool::CURVE_ANNOTATIONS ),
                                        RiuPlotAnnotationTool::CURVE_ANNOTATIONS ) );
        }
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
        auto simulationWellBranches = RiaSimWellBranchTools::simulationWellBranches( m_formationSimWellName(),
                                                                                     m_formationBranchDetection );
        options                     = RiaSimWellBranchTools::valueOptionsForBranchIndexField( simulationWellBranches );
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
caf::PdmFieldHandle* RimWellLogTrack::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogTrack::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::addCurve( RimWellLogCurve* curve )
{
    curves.push_back( curve );

    if ( m_wellLogTrackPlotWidget )
    {
        curve->setParentQwtPlotAndReplot( m_wellLogTrackPlotWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::insertCurve( RimWellLogCurve* curve, size_t index )
{
    curves.insert( index, curve );
    // Todo: Mark curve data to use either TVD or MD

    if ( m_wellLogTrackPlotWidget )
    {
        curve->setParentQwtPlotAndReplot( m_wellLogTrackPlotWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::takeOutCurve( RimWellLogCurve* curve )
{
    size_t index = curves.index( curve );
    if ( index < curves.size() )
    {
        curves[index]->detachQwtCurve();
        curves.removeChildObject( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::deleteAllCurves()
{
    curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogTrack* RimWellLogTrack::viewer()
{
    return m_wellLogTrackPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::availableDepthRange( double* minimumDepth, double* maximumDepth )
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    for ( RimPlotCurve* curve : curves )
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

        if ( curve->isCurveVisible() && curve->yValueRangeInQwt( &minCurveDepth, &maxCurveDepth ) )
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

    *minimumDepth = minDepth;
    *maximumDepth = maxDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::loadDataAndUpdate( bool updateParentPlotAndToolbars )
{
    RimWellLogPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfType( wellLogPlot );

    if ( wellLogPlot && m_wellLogTrackPlotWidget )
    {
        m_wellLogTrackPlotWidget->setXTitle( m_xAxisTitle );
    }

    for ( size_t cIdx = 0; cIdx < curves.size(); ++cIdx )
    {
        curves[cIdx]->loadDataAndUpdate( false );
    }

    if ( m_regionAnnotationType == RiuPlotAnnotationTool::FORMATION_ANNOTATIONS )
    {
        setFormationFieldsUiReadOnly( false );
    }
    else
    {
        setFormationFieldsUiReadOnly( true );
    }
    bool noAnnotations = m_regionAnnotationType() == RiuPlotAnnotationTool::NO_ANNOTATIONS;
    m_regionAnnotationDisplay.uiCapability()->setUiReadOnly( noAnnotations );
    m_showRegionLabels.uiCapability()->setUiReadOnly( noAnnotations );

    if ( m_wellLogTrackPlotWidget )
    {
        this->updateWellPathAttributesCollection();
        this->updateWellPathAttributesOnPlot();
        m_wellLogTrackPlotWidget->updateLegend();

        this->updateAxisScaleEngine();
        this->updateRegionAnnotationsOnPlot();
        this->applyXZoomFromVisibleRange();
    }

    this->updateAxisAndGridTickIntervals();
    m_majorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );
    m_minorTickInterval.uiCapability()->setUiHidden( !m_explicitTickIntervals() );

    bool emptyRange = std::abs( m_visibleXRangeMax() - m_visibleXRangeMin ) <
                      1.0e-6 * std::max( 1.0, std::max( m_visibleXRangeMax(), m_visibleXRangeMin() ) );
    m_explicitTickIntervals.uiCapability()->setUiReadOnly( emptyRange );
    m_xAxisGridVisibility.uiCapability()->setUiReadOnly( emptyRange );

    updateAllLegendItems();

    if ( updateParentPlotAndToolbars )
    {
        updateParentPlotLayout();
        RiuPlotMainWindowTools::refreshToolbars();
    }
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

    if ( m_regionAnnotationType != RiuPlotAnnotationTool::NO_ANNOTATIONS )
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

    if ( m_regionAnnotationType != RiuPlotAnnotationTool::NO_ANNOTATIONS )
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
void RimWellLogTrack::setXAxisTitle( const QString& text )
{
    m_xAxisTitle = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogTrack::depthPlotTitle() const
{
    RimWellLogPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted( parent );

    return parent->depthPlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogTrack::widthScaleFactor() const
{
    return static_cast<int>( m_widthScaleFactor() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setWidthScaleFactor( WidthScaleFactor scaleFactor )
{
    m_widthScaleFactor = scaleFactor;
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
void RimWellLogTrack::recreateViewer()
{
    if ( m_wellLogTrackPlotWidget == nullptr )
    {
        m_wellLogTrackPlotWidget = new RiuWellLogTrack( this );
        updateAxisScaleEngine();

        for ( size_t cIdx = 0; cIdx < curves.size(); ++cIdx )
        {
            curves[cIdx]->setParentQwtPlotNoReplot( this->m_wellLogTrackPlotWidget );
        }

        this->m_wellLogTrackPlotWidget->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::detachAllCurves()
{
    for ( RimPlotCurve* curve : curves )
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
void RimWellLogTrack::reattachAllCurves()
{
    for ( RimPlotCurve* curve : curves )
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
    if ( m_wellLogTrackPlotWidget )
    {
        RimWellLogPlot* wellLogPlot;
        firstAncestorOrThisOfType( wellLogPlot );
        if ( wellLogPlot )
        {
            wellLogPlot->updateDepthZoom();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::calculateXZoomRangeAndUpdateQwt()
{
    this->calculateXZoomRange();
    this->applyXZoomFromVisibleRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::applyXZoomFromVisibleRange()
{
    if ( !m_wellLogTrackPlotWidget ) return;

    m_wellLogTrackPlotWidget->setXRange( m_visibleXRangeMin, m_visibleXRangeMax );

    // Attribute range. Fixed range where well components are positioned [-1, 1].
    // Set an extended range here to allow for some label space.
    double componentRangeMax = 1.5 * ( 10.0 / ( m_widthScaleFactor() ) );
    double componentRangeMin = -0.25;
    if ( m_showWellPathComponentsBothSides )
    {
        componentRangeMin = -1.5;
    }

    m_wellLogTrackPlotWidget->setXRange( componentRangeMin, componentRangeMax, QwtPlot::xBottom );

    m_wellLogTrackPlotWidget->replot();
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

    if ( !m_isAutoScaleXEnabled() )
    {
        return;
    }

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    size_t visibleCurves = 0u;
    for ( auto curve : curves )
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
        if ( visibleCurves )
        {
            minValue = RI_LOGPLOTTRACK_MINX_DEFAULT;
            maxValue = RI_LOGPLOTTRACK_MAXX_DEFAULT;
        }
        else
        {
            // Empty axis when there are no curves
            minValue = 0;
            maxValue = 0;
        }
    }

    if ( m_minorTickInterval() != 0.0 )
    {
        std::tie( minValue, maxValue ) = adjustXRange( minValue, maxValue, m_minorTickInterval() );
    }

    m_visibleXRangeMin = minValue;
    m_visibleXRangeMax = maxValue;

    computeAndSetXRangeMinForLogarithmicScale();
    updateEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateEditors()
{
    this->updateConnectedEditors();

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );
    plot->updateConnectedEditors();

    RimWellRftPlot* rftPlot( nullptr );

    firstAncestorOrThisOfType( rftPlot );

    if ( rftPlot )
    {
        rftPlot->updateConnectedEditors();
    }
    else
    {
        RimWellPltPlot* pltPlot( nullptr );
        firstAncestorOrThisOfType( pltPlot );

        if ( pltPlot )
        {
            pltPlot->updateConnectedEditors();
        }
    }
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
    return m_regionAnnotationType() == RiuPlotAnnotationTool::FORMATION_ANNOTATIONS;
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
RimWellLogCurve* RimWellLogTrack::curveDefinitionFromCurve( const QwtPlotCurve* curve ) const
{
    for ( size_t idx = 0; idx < curves.size(); idx++ )
    {
        if ( curves[idx]->qwtPlotCurve() == curve )
        {
            return curves[idx];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_userName );
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

    caf::PdmUiGroup* componentGroup = uiOrdering.addNewGroup( "Well Path Components" );
    componentGroup->add( &m_showWellPathAttributes );
    componentGroup->add( &m_showWellPathCompletions );
    componentGroup->add( &m_wellPathAttributesInLegend );
    componentGroup->add( &m_wellPathCompletionsInLegend );
    componentGroup->add( &m_showWellPathComponentsBothSides );
    componentGroup->add( &m_showWellPathComponentLabels );

    componentGroup->add( &m_wellPathComponentSource );

    uiOrderingForXAxisSettings( uiOrdering );

    caf::PdmUiGroup* trackSettingsGroup = uiOrdering.addNewGroup( "Track Settings" );
    trackSettingsGroup->add( &m_widthScaleFactor );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::initAfterRead()
{
    if ( m_showFormations_OBSOLETE() && m_regionAnnotationType() == RiuPlotAnnotationTool::NO_ANNOTATIONS )
    {
        m_regionAnnotationType    = RiuPlotAnnotationTool::FORMATION_ANNOTATIONS;
        m_regionAnnotationDisplay = RiuPlotAnnotationTool::DARK_LINES;
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
size_t RimWellLogTrack::curveIndex( RimWellLogCurve* curve )
{
    return curves.index( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::isVisible()
{
    return m_show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::setVisible( bool visible )
{
    m_show = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateAxisScaleEngine()
{
    if ( m_isLogarithmicScaleEnabled )
    {
        m_wellLogTrackPlotWidget->setAxisScaleEngine( QwtPlot::xTop, new QwtLogScaleEngine );

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine );
    }
    else
    {
        m_wellLogTrackPlotWidget->setAxisScaleEngine( QwtPlot::xTop, new RiuQwtLinearScaleEngine );

        // NB! Must assign scale engine to bottom in order to make QwtPlotGrid work
        m_wellLogTrackPlotWidget->setAxisScaleEngine( QwtPlot::xBottom, new RiuQwtLinearScaleEngine );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogTrack::isFirstVisibleTrackInPlot() const
{
    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );
    size_t ownIndex = plot->trackIndex( this );
    return plot->firstVisibleTrackIndex() == ownIndex;
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
void RimWellLogTrack::computeAndSetXRangeMinForLogarithmicScale()
{
    if ( m_isAutoScaleXEnabled && m_isLogarithmicScaleEnabled )
    {
        double pos = HUGE_VAL;
        double neg = -HUGE_VAL;

        for ( size_t cIdx = 0; cIdx < curves.size(); cIdx++ )
        {
            if ( curves[cIdx]->isCurveVisible() && curves[cIdx]->curveData() )
            {
                RigStatisticsCalculator::posNegClosestToZero( curves[cIdx]->curveData()->xPlotValues(), pos, neg );
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
    for ( RimWellLogCurve* curve : curves )
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
QString RimWellLogTrack::description()
{
    return m_userName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RimWellLogTrack::curvesVector()
{
    std::vector<RimWellLogCurve*> curvesVector;

    for ( RimWellLogCurve* curve : curves )
    {
        curvesVector.push_back( curve );
    }

    return curvesVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RimWellLogTrack::visibleCurvesVector()
{
    std::vector<RimWellLogCurve*> curvesVector;

    for ( RimWellLogCurve* curve : curves )
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

    std::vector<const RigWellPath*> wellPaths = RiaSimWellBranchTools::simulationWellBranches( simWellName,
                                                                                               useBranchDetection );

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

    curveData.md  = extractor->cellIntersectionMDs();
    curveData.tvd = extractor->cellIntersectionTVDs();

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

    curveData.md  = extractor->cellIntersectionMDs();
    curveData.tvd = extractor->cellIntersectionTVDs();

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

    if ( depthType == RimWellLogPlot::MEASURED_DEPTH || depthType == RimWellLogPlot::PSEUDO_LENGTH )
    {
        depthVector = curveData.md;
    }
    else if ( depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH )
    {
        depthVector = curveData.tvd;
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
        if ( eclipseCase->eclipseCaseData()->activeFormationNames() )
        {
            return eclipseCase->eclipseCaseData()->activeFormationNames()->formationNames();
        }
    }
    else if ( geoMechCase )
    {
        if ( geoMechCase->geoMechData()->femPartResults()->activeFormationNames() )
        {
            return geoMechCase->geoMechData()->femPartResults()->activeFormationNames()->formationNames();
        }
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogTrack::updateRegionAnnotationsOnPlot()
{
    removeRegionAnnotations();

    if ( m_regionAnnotationType == RiuPlotAnnotationTool::NO_ANNOTATIONS ) return;

    if ( m_annotationTool == nullptr )
    {
        m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>( new RiuPlotAnnotationTool() );
    }

    if ( m_regionAnnotationType == RiuPlotAnnotationTool::FORMATION_ANNOTATIONS )
    {
        updateFormationNamesOnPlot();
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
    std::vector<QString> formationNamesToPlot;

    RimWellLogPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );

    if ( m_formationSource == CASE )
    {
        if ( ( m_formationSimWellName == QString( "None" ) && m_formationWellPathForSourceCase == nullptr ) ||
             m_formationCase == nullptr )
            return;

        RimMainPlotCollection* mainPlotCollection;
        this->firstAncestorOrThisOfTypeAsserted( mainPlotCollection );

        RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();

        CurveSamplingPointData curveData;

        RigEclipseWellLogExtractor* eclWellLogExtractor     = nullptr;
        RigGeoMechWellLogExtractor* geoMechWellLogExtractor = nullptr;

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
            eclWellLogExtractor = RiaExtractionTools::wellLogExtractorEclipseCase( m_formationWellPathForSourceCase,
                                                                                   dynamic_cast<RimEclipseCase*>(
                                                                                       m_formationCase() ) );
        }

        if ( eclWellLogExtractor )
        {
            RimEclipseCase*             eclipseCase    = dynamic_cast<RimEclipseCase*>( m_formationCase() );
            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::
                createFromResultAddress( eclipseCase->eclipseCaseData(),
                                         0,
                                         RiaDefines::PorosityModelType::MATRIX_MODEL,
                                         0,
                                         RigEclipseResultAddress( RiaDefines::FORMATION_NAMES,
                                                                  RiaDefines::activeFormationNamesResultName() ) );

            curveData = RimWellLogTrack::curveSamplingPointData( eclWellLogExtractor, resultAccessor.p() );
        }
        else
        {
            geoMechWellLogExtractor = RiaExtractionTools::wellLogExtractorGeoMechCase( m_formationWellPathForSourceCase,
                                                                                       dynamic_cast<RimGeoMechCase*>(
                                                                                           m_formationCase() ) );
            if ( !geoMechWellLogExtractor ) return;

            std::string activeFormationNamesResultName = RiaDefines::activeFormationNamesResultName().toStdString();
            curveData = RimWellLogTrack::curveSamplingPointData( geoMechWellLogExtractor,
                                                                 RigFemResultAddress( RIG_FORMATION_NAMES,
                                                                                      activeFormationNamesResultName,
                                                                                      "" ) );
        }

        std::vector<std::pair<double, double>> yValues;
        std::vector<QString> formationNamesVector = RimWellLogTrack::formationNamesVector( m_formationCase );

        RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                formationNamesVector,
                                                plot->depthType(),
                                                &formationNamesToPlot,
                                                &yValues );

        std::pair<double, double> xRange = std::make_pair( m_visibleXRangeMin(), m_visibleXRangeMax() );

        caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

        m_annotationTool->attachNamedRegions( this->viewer(),
                                              formationNamesToPlot,
                                              xRange,
                                              yValues,
                                              m_regionAnnotationDisplay(),
                                              colorTable,
                                              ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100,
                                              m_showRegionLabels() );
    }
    else if ( m_formationSource() == WELL_PICK_FILTER )
    {
        if ( m_formationWellPathForSourceWellPath == nullptr ) return;

        if ( !( plot->depthType() == RimWellLogPlot::MEASURED_DEPTH ||
                plot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH ) )
        {
            return;
        }

        std::vector<double> yValues;

        const RigWellPathFormations* formations = m_formationWellPathForSourceWellPath->formationsGeometry();
        if ( !formations ) return;

        formations->depthAndFormationNamesUpToLevel( m_formationLevel(),
                                                     &formationNamesToPlot,
                                                     &yValues,
                                                     m_showformationFluids(),
                                                     plot->depthType() );

        m_annotationTool->attachWellPicks( this->viewer(), formationNamesToPlot, yValues );
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
        RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(
            wellBoreStabilityPlot->commonDataSource()->caseToApply() );
        RimWellPath* wellPath = wellBoreStabilityPlot->commonDataSource()->wellPathToApply();
        int          timeStep = wellBoreStabilityPlot->commonDataSource()->timeStepToApply();
        if ( geoMechCase && wellPath && timeStep >= 0 )
        {
            RigGeoMechWellLogExtractor* geoMechWellLogExtractor = nullptr;
            geoMechWellLogExtractor = RiaExtractionTools::wellLogExtractorGeoMechCase( wellPath,
                                                                                       dynamic_cast<RimGeoMechCase*>(
                                                                                           geoMechCase ) );
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
                geoMechWellLogExtractor->setWbsParameters( wbsPlot->porePressureSource(),
                                                           wbsPlot->poissonRatioSource(),
                                                           wbsPlot->ucsSource(),
                                                           wbsPlot->userDefinedPoissonRatio(),
                                                           wbsPlot->userDefinedUcs() );
            }

            std::vector<double> ppValues      = geoMechWellLogExtractor->porePressureIntervals( timeStep );
            std::vector<double> poissonValues = geoMechWellLogExtractor->poissonIntervals( timeStep );
            std::vector<double> ucsValues     = geoMechWellLogExtractor->ucsIntervals( timeStep );

            {
                caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

                std::vector<QString> sourceNames =
                    {"", "PP=Grid", "PP=Las-File", "PP=Element Property Table", "", "PP=Hydrostatic"};
                curveData.data = ppValues;

                std::vector<QString>                   sourceNamesToPlot;
                std::vector<std::pair<double, double>> yValues;
                RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                        sourceNames,
                                                        wellBoreStabilityPlot->depthType(),
                                                        &sourceNamesToPlot,
                                                        &yValues );
                m_annotationTool->attachNamedRegions( this->viewer(),
                                                      sourceNamesToPlot,
                                                      xRange,
                                                      yValues,
                                                      m_regionAnnotationDisplay(),
                                                      colorTable,
                                                      ( ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100 ) / 3,
                                                      m_showRegionLabels(),
                                                      RiuPlotAnnotationTool::LEFT_COLUMN );
            }
            {
                caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

                std::vector<QString> sourceNames =
                    {"",
                     "",
                     "Poisson=Las-File",
                     "Poisson=Element Property Table",
                     QString( "Poisson=%1" ).arg( wellBoreStabilityPlot->userDefinedPoissonRatio() ),
                     ""};
                curveData.data = poissonValues;

                std::vector<QString>                   sourceNamesToPlot;
                std::vector<std::pair<double, double>> yValues;
                RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                        sourceNames,
                                                        wellBoreStabilityPlot->depthType(),
                                                        &sourceNamesToPlot,
                                                        &yValues );
                m_annotationTool->attachNamedRegions( this->viewer(),
                                                      sourceNamesToPlot,
                                                      xRange,
                                                      yValues,
                                                      m_regionAnnotationDisplay(),
                                                      colorTable,
                                                      ( ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100 ) / 3,
                                                      m_showRegionLabels(),
                                                      RiuPlotAnnotationTool::CENTRE_COLUMN );
            }
            {
                caf::ColorTable colorTable( RimRegularLegendConfig::colorArrayFromColorType( m_colorShadingPalette() ) );

                std::vector<QString> sourceNames = {"",
                                                    "",
                                                    "UCS=Las-File",
                                                    "UCS=Element Property Table",
                                                    QString( "UCS=%1" ).arg( wellBoreStabilityPlot->userDefinedUcs() ),
                                                    ""};

                curveData.data = ucsValues;

                std::vector<QString>                   sourceNamesToPlot;
                std::vector<std::pair<double, double>> yValues;
                RimWellLogTrack::findRegionNamesToPlot( curveData,
                                                        sourceNames,
                                                        wellBoreStabilityPlot->depthType(),
                                                        &sourceNamesToPlot,
                                                        &yValues );
                m_annotationTool->attachNamedRegions( this->viewer(),
                                                      sourceNamesToPlot,
                                                      xRange,
                                                      yValues,
                                                      m_regionAnnotationDisplay(),
                                                      colorTable,
                                                      ( ( ( 100 - m_colorShadingTransparency ) * 255 ) / 100 ) / 3,
                                                      m_showRegionLabels(),
                                                      RiuPlotAnnotationTool::RIGHT_COLUMN );
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

        if ( m_showWellPathAttributes || m_showWellPathCompletions )
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

        const std::map<RiaDefines::WellPathComponentType, int> sortIndices = {{RiaDefines::WELL_PATH, 0},
                                                                              {RiaDefines::CASING, 1},
                                                                              {RiaDefines::LINER, 2},
                                                                              {RiaDefines::PERFORATION_INTERVAL, 3},
                                                                              {RiaDefines::FISHBONES, 4},
                                                                              {RiaDefines::FRACTURE, 5},
                                                                              {RiaDefines::PACKER, 6},
                                                                              {RiaDefines::ICD, 7},
                                                                              {RiaDefines::AICD, 8},
                                                                              {RiaDefines::ICV, 9}};

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
            QString legendTitle        = plotItem->legendTitle();
            bool    contributeToLegend = m_wellPathCompletionsInLegend() &&
                                      !completionsAssignedToLegend.count( legendTitle );
            plotItem->setContributeToLegend( contributeToLegend );
            m_wellPathAttributePlotObjects.push_back( std::move( plotItem ) );
            completionsAssignedToLegend.insert( legendTitle );
        }

        RimWellLogPlot* wellLogPlot;
        this->firstAncestorOrThisOfTypeAsserted( wellLogPlot );
        RimWellLogPlot::DepthTypeEnum depthType = wellLogPlot->depthType();

        for ( auto& attributePlotObject : m_wellPathAttributePlotObjects )
        {
            attributePlotObject->setDepthType( depthType );
            attributePlotObject->setShowLabel( m_showWellPathComponentLabels() );
            attributePlotObject->loadDataAndUpdate( false );
            attributePlotObject->setParentQwtPlotNoReplot( m_wellLogTrackPlotWidget );
        }
    }
    applyXZoomFromVisibleRange();
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

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

#include "RimDepthTrackPlot.h"

#include "RiaGuiApplication.h"
#include "RiaOptionItemFactory.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaTextStringTools.h"

#include "Well/RigWellLogCurveData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleWellLogCurveSet.h"
#include "RimGeoMechCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimPlot.h"
#include "RimPlotAxisAnnotation.h"
#include "RimPlotWindow.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogCurveInfoTextProvider.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cvfAssert.h"

#include <QKeyEvent>

#include <cmath>

#define RI_LOGPLOT_MINDEPTH_DEFAULT 0.0
#define RI_LOGPLOT_MAXDEPTH_DEFAULT 1000.0

namespace caf
{
template <>
void RimDepthTrackPlot::AxisGridEnum::setUp()
{
    addItem( RimDepthTrackPlot::AXIS_GRID_NONE, "GRID_X_NONE", "No Grid Lines" );
    addItem( RimDepthTrackPlot::AXIS_GRID_MAJOR, "GRID_X_MAJOR", "Major Only" );
    addItem( RimDepthTrackPlot::AXIS_GRID_MAJOR_AND_MINOR, "GRID_X_MAJOR_AND_MINOR", "Major and Minor" );
    setDefault( RimDepthTrackPlot::AXIS_GRID_MAJOR );
}

template <>
void caf::AppEnum<RimDepthTrackPlot::DepthOrientation_OBSOLETE>::setUp()
{
    addItem( RimDepthTrackPlot::DepthOrientation_OBSOLETE::HORIZONTAL, "HORIZONTAL", "Horizontal" );
    addItem( RimDepthTrackPlot::DepthOrientation_OBSOLETE::VERTICAL, "VERTICAL", "Vertical" );
    setDefault( RimDepthTrackPlot::DepthOrientation_OBSOLETE::VERTICAL );
}

} // End namespace caf

CAF_PDM_SOURCE_INIT( RimDepthTrackPlot, "DepthTrackPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot::RimDepthTrackPlot()
{
    CAF_PDM_InitObject( "Depth Track Plot", "", "", "A Plot With a shared Depth Axis and Multiple Tracks" );

    CAF_PDM_InitFieldNoDefault( &m_commonDataSource, "CommonDataSource", "Data Source", "", "Change the Data Source of All Curves in the Plot", "" );
    m_commonDataSource.uiCapability()->setUiTreeChildrenHidden( true );
    m_commonDataSource.xmlCapability()->disableIO();
    m_commonDataSource = new RimWellLogCurveCommonDataSource;

    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name" );
    m_plotWindowTitle.xmlCapability()->setIOWritable( false );

    auto templateText = QString( "%1, %2" ).arg( RiaDefines::namingVariableCase() ).arg( RiaDefines::namingVariableWell() );
    CAF_PDM_InitField( &m_nameTemplateText, "TemplateText", templateText, "Template Text" );
    CAF_PDM_InitFieldNoDefault( &m_namingMethod, "PlotNamingMethod", "Plot Name" );

    caf::AppEnum<RimDepthTrackPlot::DepthTypeEnum> depthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    CAF_PDM_InitScriptableField( &m_depthType, "DepthType", depthType, "Type" );

    caf::AppEnum<RiaDefines::DepthUnitType> depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    CAF_PDM_InitScriptableField( &m_depthUnit, "DepthUnit", depthUnit, "Unit" );

    CAF_PDM_InitScriptableField( &m_minVisibleDepth, "MinimumDepth", 0.0, "Min" );
    CAF_PDM_InitScriptableField( &m_maxVisibleDepth, "MaximumDepth", 1000.0, "Max" );
    m_minVisibleDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    m_maxVisibleDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldNoDefault( &m_depthAxisGridVisibility, "ShowDepthGridLines", "Show Grid Lines" );
    CAF_PDM_InitScriptableField( &m_isAutoScaleDepthEnabled, "AutoScaleDepthEnabled", true, "Auto Scale" );
    m_isAutoScaleDepthEnabled.uiCapability()->setUiHidden( true );

    caf::AppEnum<RiaDefines::MultiPlotAxisVisibility> depthAxisVisibility = RiaDefines::MultiPlotAxisVisibility::ONE_VISIBLE;
    CAF_PDM_InitField( &m_depthAxisVisibility, "DepthAxisVisibility", depthAxisVisibility, "Axis Visibility" );

    CAF_PDM_InitScriptableField( &m_showDepthMarkerLine, "ShowDepthMarkerLine", false, "Show Depth Marker Line" );

    CAF_PDM_InitScriptableField( &m_autoZoomMinDepthFactor, "AutoZoomMinDepthFactor", 0.0, "Auto Zoom Minimum Factor" );
    CAF_PDM_InitScriptableField( &m_autoZoomMaxDepthFactor, "AutoZoomMaxDepthFactor", 0.0, "Auto Zoom Maximum Factor" );

    CAF_PDM_InitFieldNoDefault( &m_depthAnnotations, "DepthAnnotations", "Depth Annotations" );
    m_depthAnnotations.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_subTitleFontSize, "SubTitleFontSize", "Track Title Font Size" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "" );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );
    m_nameConfig = new RimWellLogPlotNameConfig();

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "FilterEnsembleCurveSet", "Filter by Ensemble Curve Set" );
    CAF_PDM_InitFieldNoDefault( &m_depthEqualization, "DepthEqualization", "Depth Equalization" );

    CAF_PDM_InitFieldNoDefault( &m_plots, "Tracks", "Tracks" );
    auto reorderability = caf::PdmFieldReorderCapability::addToField( &m_plots );
    reorderability->orderChanged.connect( this, &RimDepthTrackPlot::onPlotsReordered );

    CAF_PDM_InitFieldNoDefault( &m_depthOrientation, "DepthOrientation", "Orientation" );

    m_availableDepthUnits = { RiaDefines::DepthUnitType::UNIT_METER, RiaDefines::DepthUnitType::UNIT_FEET };
    m_availableDepthTypes = { RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                              RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH,
                              RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB };

    m_minAvailableDepth = HUGE_VAL;
    m_maxAvailableDepth = -HUGE_VAL;

    m_commonDataSourceEnabled = true;

    m_plotLegendsHorizontal = false;
    setPlotTitleVisible( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot::~RimDepthTrackPlot()
{
    delete m_commonDataSource;
    delete m_nameConfig;

    removeMdiWindowFromMdiArea();
    m_plots.deleteChildren();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot& RimDepthTrackPlot::operator=( RimDepthTrackPlot&& rhs )
{
    RimPlotWindow::operator=( std::move( rhs ) );

    // Move all tracks
    auto plots = rhs.m_plots.childrenByType();
    rhs.m_plots.clearWithoutDelete();
    for ( auto plot : plots )
    {
        m_plots.push_back( plot );
    }

    // Deliberately don't set m_plotWindowTitle and m_nameConfig. This operator is used for copying parameters from
    // children. This only happens for some plots that used to own a plot but now inherits the plot.
    // They had their own description at top level which we don't want to overwrite.

    auto dataSource = rhs.m_commonDataSource();
    rhs.m_commonDataSource.removeChild( dataSource );
    m_commonDataSource        = dataSource;
    m_commonDataSourceEnabled = rhs.m_commonDataSourceEnabled;

    m_depthType               = rhs.m_depthType();
    m_depthUnit               = rhs.m_depthUnit();
    m_minVisibleDepth         = rhs.m_minVisibleDepth();
    m_maxVisibleDepth         = rhs.m_maxVisibleDepth();
    m_depthAxisGridVisibility = rhs.m_depthAxisGridVisibility();
    m_isAutoScaleDepthEnabled = rhs.m_isAutoScaleDepthEnabled();
    m_depthAxisVisibility     = rhs.m_depthAxisVisibility();

    m_subTitleFontSize  = rhs.m_subTitleFontSize();
    m_axisTitleFontSize = rhs.m_axisTitleFontSize();
    m_axisValueFontSize = rhs.m_axisValueFontSize();

    m_minAvailableDepth = rhs.m_minAvailableDepth;
    m_maxAvailableDepth = rhs.m_maxAvailableDepth;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimDepthTrackPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimDepthTrackPlot::createPlotWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    return createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDepthTrackPlot::description() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimDepthTrackPlot::plotCount() const
{
    return m_plots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimDepthTrackPlot::plotIndex( const RimWellLogTrack* plot ) const
{
    return m_plots.indexOf( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RimDepthTrackPlot::plotByIndex( size_t index ) const
{
    if ( index < m_plots.size() )
    {
        return m_plots[index];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimDepthTrackPlot::plots() const
{
    std::vector<RimPlot*> baseClassPlots;

    for ( auto p : m_plots.childrenByType() )
    {
        baseClassPlots.push_back( p );
    }

    return baseClassPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogTrack*> RimDepthTrackPlot::visiblePlots() const
{
    std::vector<RimWellLogTrack*> allVisiblePlots;
    for ( auto plot : m_plots() )
    {
        if ( plot->showWindow() )
        {
            allVisiblePlots.push_back( plot );
        }
    }
    return allVisiblePlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDepthTrackPlot::columnCount() const
{
    if ( depthOrientation() == RiaDefines::Orientation::VERTICAL )
        return RimPlotWindow::columnCount();
    else
        return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::updateZoom()
{
    if ( m_isAutoScaleDepthEnabled )
    {
        calculateAvailableDepthRange();
        if ( m_minAvailableDepth < HUGE_VAL && m_maxAvailableDepth > -HUGE_VAL )
        {
            auto depthRange = m_maxAvailableDepth - m_minAvailableDepth;

            m_minVisibleDepth = m_minAvailableDepth - m_autoZoomMinDepthFactor * depthRange;
            m_maxVisibleDepth = m_maxAvailableDepth + ( 0.01 + m_autoZoomMaxDepthFactor ) * depthRange;
        }
    }

    for ( RimPlot* plot : plots() )
    {
        static_cast<RimWellLogTrack*>( plot )->setVisibleDepthRange( m_minVisibleDepth(), m_maxVisibleDepth() );
        plot->updatePlotWidgetFromAxisRanges();
    }

    if ( m_viewer )
    {
        m_viewer->updateVerticalScrollBar( m_minVisibleDepth(), m_maxVisibleDepth(), m_minAvailableDepth, m_maxAvailableDepth );
    }

    // Required to make sure the tracks are aligned correctly for vertical plots
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthAxisRangeByFactorAndCenter( double zoomFactor, double zoomCenter )
{
    double newMinimum = zoomCenter - ( zoomCenter - m_minVisibleDepth ) * zoomFactor;
    double newMaximum = zoomCenter + ( m_maxVisibleDepth - zoomCenter ) * zoomFactor;

    setDepthAxisRange( newMinimum, newMaximum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthAxisRangeByPanDepth( double panFactor )
{
    double delta = panFactor * ( m_maxVisibleDepth - m_minVisibleDepth );
    setDepthAxisRange( m_minVisibleDepth + delta, m_maxVisibleDepth + delta );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthAxisRange( double minimumDepth, double maximumDepth )
{
    m_minVisibleDepth = minimumDepth;
    m_maxVisibleDepth = maximumDepth;

    m_minVisibleDepth.uiCapability()->updateConnectedEditors();
    m_maxVisibleDepth.uiCapability()->updateConnectedEditors();

    setAutoScaleDepthValuesEnabled( false );
    updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::calculateAvailableDepthRange()
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    auto plots = this->plots();

    for ( size_t tIdx = 0; tIdx < plots.size(); tIdx++ )
    {
        double minTrackDepth = HUGE_VAL;
        double maxTrackDepth = -HUGE_VAL;

        if ( plots[tIdx]->showWindow() )
        {
            static_cast<RimWellLogTrack*>( plots[tIdx] )->availableDepthRange( &minTrackDepth, &maxTrackDepth );

            if ( minTrackDepth < minDepth )
            {
                minDepth = minTrackDepth;
            }

            if ( maxTrackDepth > maxDepth )
            {
                maxDepth = maxTrackDepth;
            }
        }
    }

    if ( minDepth < HUGE_VAL && maxDepth > -HUGE_VAL )
    {
        m_minAvailableDepth = minDepth;
        m_maxAvailableDepth = maxDepth;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::availableDepthRange( double* minimumDepth, double* maximumDepth ) const
{
    *minimumDepth = m_minAvailableDepth;
    *maximumDepth = m_maxAvailableDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::visibleDepthRange( double* minimumDepth, double* maximumDepth ) const
{
    *minimumDepth = m_minVisibleDepth;
    *maximumDepth = m_maxVisibleDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::enableDepthMarkerLine( bool enable )
{
    m_showDepthMarkerLine = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDepthTrackPlot::isDepthMarkerLineEnabled() const
{
    return m_showDepthMarkerLine();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthMarkerPosition( double depth )
{
    RimPlotAxisAnnotation* firstAnnotation = nullptr;
    if ( !m_depthAnnotations.empty() )
    {
        firstAnnotation = m_depthAnnotations[0];
    }

    if ( firstAnnotation == nullptr )
    {
        firstAnnotation = new RimPlotAxisAnnotation();
        firstAnnotation->setAnnotationType( RimPlotAxisAnnotation::AnnotationType::LINE );
        firstAnnotation->setPenStyle( Qt::DashLine );
        m_depthAnnotations.push_back( firstAnnotation );
    }

    firstAnnotation->setValue( depth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::clearDepthAnnotations()
{
    m_depthAnnotations.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisAnnotation*> RimDepthTrackPlot::depthAxisAnnotations() const
{
    return m_depthAnnotations.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAutoZoomMinimumDepthFactor( double factor )
{
    m_autoZoomMinDepthFactor = factor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAutoZoomMaximumDepthFactor( double factor )
{
    m_autoZoomMaxDepthFactor = factor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RimDepthTrackPlot::caseDepthUnit() const
{
    RimEclipseResultCase* thecase = dynamic_cast<RimEclipseResultCase*>( commonDataSource()->caseToApply() );
    if ( thecase == nullptr )
    {
        // no suitable case found, look in the project to see if there is a eclipse case with units defined loaded
        RimProject* p = RiaApplication::instance()->project();
        for ( RimEclipseCase* aCase : p->activeOilField()->analysisModels()->cases() )
        {
            thecase = dynamic_cast<RimEclipseResultCase*>( aCase );
            if ( thecase ) break;
        }
    }

    if ( thecase )
    {
        switch ( thecase->unitSystem() )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                return RiaDefines::DepthUnitType::UNIT_FEET;

            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                return RiaDefines::DepthUnitType::UNIT_METER;

            default:
                break;
        }
    }

    return RiaDefines::DepthUnitType::UNIT_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::uiOrderingForDepthAxis( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_availableDepthTypes.size() > 1u )
    {
        uiOrdering.add( &m_depthType );
    }

    if ( m_availableDepthUnits.size() > 1u )
    {
        uiOrdering.add( &m_depthUnit );
    }

    uiOrdering.add( &m_minVisibleDepth );
    uiOrdering.add( &m_maxVisibleDepth );

    auto group = uiOrdering.addNewGroup( "Advanced" );
    group->setCollapsedByDefault();
    group->add( &m_depthOrientation );
    group->add( &m_depthAxisGridVisibility );
    group->add( &m_depthAxisVisibility );
    group->add( &m_showDepthMarkerLine );

    group->add( &m_autoZoomMinDepthFactor );
    group->add( &m_autoZoomMaxDepthFactor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::uiOrderingForAutoName( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotTitle );
    uiOrdering.add( &m_namingMethod );
    uiOrdering.add( &m_nameTemplateText );

    m_nameConfig->uiOrdering( uiConfigName, uiOrdering );

    auto tooltipText = supportedPlotNameVariables().join( ", " );
    m_nameTemplateText.uiCapability()->setUiToolTip( tooltipText );
    m_nameTemplateText.uiCapability()->setUiHidden( m_namingMethod() != RiaDefines::ObjectNamingMethod::TEMPLATE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDepthTrackPlot::createPlotNameFromTemplate( const QString& templateText ) const
{
    return RiaTextStringTools::replaceTemplateTextWithValues( templateText, createNameKeyValueMap() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimDepthTrackPlot::supportedPlotNameVariables() const
{
    return { RiaDefines::namingVariableCase(),
             RiaDefines::namingVariableWell(),
             RiaDefines::namingVariableRefWell(),
             RiaDefines::namingVariableWellBranch(),
             RiaDefines::namingVariableTime(),
             RiaDefines::namingVariableAirGap() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimDepthTrackPlot::createNameKeyValueMap() const
{
    std::map<QString, QString> variableValueMap;

    RimCase*     commonCase     = m_commonDataSource->caseToApply();
    RimWellPath* commonWellPath = m_commonDataSource->wellPathToApply();

    if ( commonCase )
    {
        variableValueMap[RiaDefines::namingVariableCase()] = commonCase->caseUserDescription();

        if ( m_commonDataSource->timeStepToApply() != -1 )
        {
            variableValueMap[RiaDefines::namingVariableTime()] = commonCase->timeStepName( m_commonDataSource->timeStepToApply() );
        }
    }
    else
    {
        auto summaryCase = m_commonDataSource->summaryCaseToApply();
        if ( summaryCase )
        {
            variableValueMap[RiaDefines::namingVariableCase()] = summaryCase->displayCaseName();

            auto wellName = m_commonDataSource->rftWellName();
            if ( !wellName.isEmpty() ) variableValueMap[RiaDefines::namingVariableWell()] = wellName;

            auto dateTime = m_commonDataSource->rftTime();
            if ( dateTime.isValid() )
            {
                variableValueMap[RiaDefines::namingVariableTime()] = dateTime.toString( RiaQDateTimeTools::dateFormatString() );
            }

            auto branchIndex = m_commonDataSource->rftBranchIndex();
            if ( branchIndex >= 0 )
            {
                variableValueMap[RiaDefines::namingVariableWellBranch()] = QString::number( branchIndex );
            }
        }
    }

    if ( commonWellPath && !commonWellPath->name().isEmpty() )
    {
        variableValueMap[RiaDefines::namingVariableWell()] = commonWellPath->name();
    }
    else if ( !m_commonDataSource->simWellNameToApply().isEmpty() )
    {
        variableValueMap[RiaDefines::namingVariableWell()] = m_commonDataSource->simWellNameToApply();
    }

    if ( commonWellPath )
    {
        RigWellPath* wellPathGeometry = commonWellPath->wellPathGeometry();
        if ( wellPathGeometry )
        {
            double rkb = wellPathGeometry->rkbDiff();

            variableValueMap[RiaDefines::namingVariableAirGap()] = QString( "Air Gap = %1 m" ).arg( rkb );
        }
    }

    return variableValueMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDepthTrackPlot::createAutoName() const
{
    if ( m_namingMethod() == RiaDefines::ObjectNamingMethod::AUTO )
    {
        // Use the ordering of the supported variables to create a name
        auto candidateNames = supportedPlotNameVariables();

        auto variableValues = createNameKeyValueMap();

        QStringList variablesWithValue;
        for ( const auto& name : candidateNames )
        {
            if ( variableValues.count( name ) )
            {
                variablesWithValue.push_back( name );
            }
        }

        QString templateText = variablesWithValue.join( ", " );
        return createPlotNameFromTemplate( templateText );
    }

    if ( m_namingMethod() == RiaDefines::ObjectNamingMethod::TEMPLATE )
    {
        return createPlotNameFromTemplate( m_nameTemplateText );
    }

    return m_nameConfig->customName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotNameConfig* RimDepthTrackPlot::nameConfig() const
{
    return m_nameConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setNameTemplateText( const QString& templateText )
{
    m_nameTemplateText = templateText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setNamingMethod( RiaDefines::ObjectNamingMethod namingMethod )
{
    m_namingMethod = namingMethod;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimDepthTrackPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        QPixmap pix( m_viewer->size() );
        m_viewer->renderTo( &pix );
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimDepthTrackPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new RiuWellLogPlot( this, mainWindowParent );
    recreatePlotWidgets();
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::performAutoNameUpdate()
{
    updateCommonDataSource();
    m_plotWindowTitle = createAutoName();
    if ( m_viewer )
    {
        m_viewer->setTitleVisible( m_showPlotTitle() );
        m_viewer->setPlotTitle( m_plotWindowTitle );
    }
    updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::recreatePlotWidgets()
{
    CVF_ASSERT( m_viewer );

    m_viewer->removeAllPlots();

    for ( auto plot : m_plots )
    {
        RimDepthTrackPlot::createPlotWidgetAndAttachCurveTextProvider( plot );

        m_viewer->addPlot( plot->plotWidget() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::handleKeyPressEvent( QKeyEvent* keyEvent )
{
    if ( keyEvent->key() == Qt::Key_PageUp )
    {
        if ( keyEvent->modifiers() & Qt::ShiftModifier )
        {
            m_commonDataSource->applyPrevCase();
            keyEvent->accept();
        }
        else if ( keyEvent->modifiers() & Qt::ControlModifier )
        {
            m_commonDataSource->applyPrevWell();
            keyEvent->accept();
        }
        else
        {
            m_commonDataSource->applyPrevTimeStep();
            keyEvent->accept();
        }
    }
    else if ( keyEvent->key() == Qt::Key_PageDown )
    {
        if ( keyEvent->modifiers() & Qt::ShiftModifier )
        {
            m_commonDataSource->applyNextCase();
            keyEvent->accept();
        }
        else if ( keyEvent->modifiers() & Qt::ControlModifier )
        {
            m_commonDataSource->applyNextWell();
            keyEvent->accept();
        }
        else
        {
            m_commonDataSource->applyNextTimeStep();
            keyEvent->accept();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurveCommonDataSource* RimDepthTrackPlot::commonDataSource() const
{
    return m_commonDataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::updateCommonDataSource()
{
    m_commonDataSource->analyseCurvesAndTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setCommonDataSourceEnabled( bool enable )
{
    m_commonDataSourceEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAvailableDepthUnits( const std::set<RiaDefines::DepthUnitType>& depthUnits )
{
    m_availableDepthUnits = depthUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAvailableDepthTypes( const std::set<DepthTypeEnum>& depthTypes )
{
    m_availableDepthTypes = depthTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDepthTrackPlot::asciiDataForPlotExport() const
{
    QString plotContentAsText;

    for ( RimPlot* plot : plots() )
    {
        if ( plot->showWindow() )
        {
            plotContentAsText += plot->asciiDataForPlotExport();
        }
    }

    return plotContentAsText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::onPlotAdditionOrRemoval()
{
    calculateAvailableDepthRange();
    updateZoom();
    updateSubPlotNames();
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        m_viewer->renderTo( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::doUpdateLayout()
{
    if ( m_viewer )
    {
        m_viewer->setTitleFontSizes( titleFontSize(), subTitleFontSize() );
        m_viewer->setLegendFontSize( legendFontSize() );
        m_viewer->setAxisFontSizes( axisTitleFontSize(), axisValueFontSize() );
        m_viewer->scheduleUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::onPlotsReordered( const SignalEmitter* emitter )
{
    updateSubPlotNames();
    recreatePlotWidgets();
    loadDataAndUpdate();

    RiaPlotWindowRedrawScheduler::instance()->performScheduledUpdates();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::updateDepthAxisVisibility()
{
    auto plots = visiblePlots();

    for ( auto p : plots )
    {
        auto plotWidget = p->plotWidget();
        if ( !plotWidget ) continue;

        bool isFirstTrack = ( p == plots.front() );
        bool isLastTrack  = ( p == plots.back() );

        p->updateAxesVisibility( depthOrientation(), isFirstTrack, isLastTrack );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::cleanupBeforeClose()
{
    auto plotVector = plots();
    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        plotVector[tIdx]->detachAllCurves();
    }

    if ( m_viewer )
    {
        m_viewer->setParent( nullptr );
        delete m_viewer;
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::updateSubPlotNames()
{
    auto plotVector = plots();
    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        RimWellLogTrack* track = dynamic_cast<RimWellLogTrack*>( plotVector[tIdx] );
        CAF_ASSERT( track );
        if ( track )
        {
            QString            description = track->description();
            QRegularExpression regexp( "Track \\d+" );
            description.replace( regexp, QString( "Track %1" ).arg( tIdx + 1 ) );
            track->setDescription( description );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_minVisibleDepth || changedField == &m_maxVisibleDepth )
    {
        m_isAutoScaleDepthEnabled = false;
        updateZoom();
    }
    else if ( changedField == &m_depthAxisGridVisibility || changedField == &m_autoZoomMaxDepthFactor ||
              changedField == &m_autoZoomMinDepthFactor )
    {
        updateZoom();
    }
    else if ( changedField == &m_isAutoScaleDepthEnabled )
    {
        if ( m_isAutoScaleDepthEnabled )
        {
            updateZoom();
        }
    }
    else if ( changedField == &m_depthType )
    {
        m_isAutoScaleDepthEnabled = true;

        RimWellAllocationPlot* parentWellAllocation = firstAncestorOrThisOfType<RimWellAllocationPlot>();
        if ( parentWellAllocation )
        {
            parentWellAllocation->loadDataAndUpdate();
        }
        else
        {
            loadDataAndUpdate();
        }
    }
    else if ( changedField == &m_depthUnit )
    {
        m_isAutoScaleDepthEnabled = true;
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_depthOrientation || changedField == &m_depthAxisVisibility )
    {
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_subTitleFontSize || changedField == &m_axisTitleFontSize || changedField == &m_axisValueFontSize )
    {
        updateFonts();
    }
    else if ( changedField == &m_showPlotTitle || changedField == &m_namingMethod || changedField == &m_nameTemplateText )
    {
        performAutoNameUpdate();
    }
    else if ( changedField == &m_depthEqualization )
    {
        std::vector<RimEnsembleWellLogCurveSet*> ensembleWellLogCurveSets = descendantsOfType<RimEnsembleWellLogCurveSet>();
        for ( auto ensembleWellLogCurveSet : ensembleWellLogCurveSets )
        {
            ensembleWellLogCurveSet->setDepthEqualization( m_depthEqualization() );
            ensembleWellLogCurveSet->loadDataAndUpdate( true );
        }
    }
    else if ( changedField == &m_ensembleCurveSet )
    {
        std::vector<RimEnsembleWellLogCurveSet*> ensembleWellLogCurveSets = descendantsOfType<RimEnsembleWellLogCurveSet>();
        for ( auto ensembleWellLogCurveSet : ensembleWellLogCurveSets )
        {
            ensembleWellLogCurveSet->setFilterByEnsembleCurveSet( m_ensembleCurveSet() );
            ensembleWellLogCurveSet->loadDataAndUpdate( true );
        }
    }
    else if ( changedField == &m_showDepthMarkerLine )
    {
        if ( !m_showDepthMarkerLine )
        {
            clearDepthAnnotations();
            for ( auto p : plots() )
            {
                p->updateAxes();
            }
        }
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_commonDataSourceEnabled )
    {
        m_commonDataSource->uiOrdering( uiConfigName, uiOrdering );
    }

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "Depth Axis" );
    uiOrderingForDepthAxis( uiConfigName, *gridGroup );

    caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup( "Plot Title" );
    uiOrderingForAutoName( uiConfigName, *titleGroup );

    caf::PdmUiGroup* legendGroup = uiOrdering.addNewGroup( "Legends" );
    legendGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForLegends( uiConfigName, *legendGroup, true );

    uiOrderingForFonts( uiConfigName, uiOrdering );

    std::vector<RimEnsembleWellLogCurveSet*> ensembleWellLogCurveSets = descendantsOfType<RimEnsembleWellLogCurveSet>();
    if ( !ensembleWellLogCurveSets.empty() )
    {
        caf::PdmUiGroup* ensembleWellLogGroup = uiOrdering.addNewGroup( "Ensemble Well Log" );
        ensembleWellLogGroup->add( &m_depthEqualization );
        ensembleWellLogGroup->add( &m_ensembleCurveSet );

        // Disable depth equalization if any of the ensemble is missing k-layer info
        bool hasKLayerIndex = true;
        for ( auto wellLogCurveSet : ensembleWellLogCurveSets )
            if ( !wellLogCurveSet->hasPropertyInFile( RiaResultNames::indexKResultName() ) ) hasKLayerIndex = false;

        m_depthEqualization.uiCapability()->setUiReadOnly( !hasKLayerIndex );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimDepthTrackPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_depthType )
    {
        using DepthAppEnum = caf::AppEnum<DepthTypeEnum>;
        for ( size_t i = 0; i < DepthAppEnum::size(); ++i )
        {
            DepthTypeEnum enumVal = DepthAppEnum::fromIndex( i );

            if ( m_availableDepthTypes.count( enumVal ) )
            {
                options.push_back( caf::PdmOptionItemInfo( DepthAppEnum::uiText( enumVal ), enumVal ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_depthUnit )
    {
        using UnitAppEnum = caf::AppEnum<RiaDefines::DepthUnitType>;
        for ( auto depthUnit : m_availableDepthUnits )
        {
            options.push_back( caf::PdmOptionItemInfo( UnitAppEnum::uiText( depthUnit ), depthUnit ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ensembleCurveSet )
    {
        RiaOptionItemFactory::appendOptionItemsForEnsembleCurveSets( &options );
    }
    else if ( fieldNeedingOptions == &m_namingMethod )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ObjectNamingMethod>::uiText( RiaDefines::ObjectNamingMethod::AUTO ),
                                                   RiaDefines::ObjectNamingMethod::AUTO ) );

        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ObjectNamingMethod>::uiText( RiaDefines::ObjectNamingMethod::CUSTOM ),
                                                   RiaDefines::ObjectNamingMethod::CUSTOM ) );

        if ( !supportedPlotNameVariables().isEmpty() )
        {
            options.push_back(
                caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ObjectNamingMethod>::uiText( RiaDefines::ObjectNamingMethod::TEMPLATE ),
                                        RiaDefines::ObjectNamingMethod::TEMPLATE ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::initAfterRead()
{
    RimPlotWindow::initAfterRead();

    if ( m_depthAxisGridVisibility() == AXIS_GRID_MINOR )
    {
        m_depthAxisGridVisibility = AXIS_GRID_MAJOR_AND_MINOR;
    }

    if ( !m_plotWindowTitle().isEmpty() )
    {
        m_nameConfig->setCustomName( m_plotWindowTitle );
    }

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2022.06.2" ) && !m_nameConfig->customName().isEmpty() )
    {
        m_namingMethod = RiaDefines::ObjectNamingMethod::CUSTOM;
    }

    performAutoNameUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_minVisibleDepth || field == &m_maxVisibleDepth )
    {
        caf::PdmUiDoubleValueEditorAttribute::testAndSetFixedWithTwoDecimals( attribute );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    performAutoNameUpdate();
    updatePlots();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::updatePlots()
{
    if ( m_showWindow )
    {
        updateDepthAxisVisibility();

        for ( RimPlot* plot : plots() )
        {
            plot->loadDataAndUpdate();
        }
        updateZoom();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimDepthTrackPlot::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::uiOrderingForFonts( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Fonts" );
    fontGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForFonts( uiConfigName, *fontGroup );
    fontGroup->add( &m_subTitleFontSize );
    fontGroup->add( &m_axisTitleFontSize );
    fontGroup->add( &m_axisValueFontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::createPlotWidgetAndAttachCurveTextProvider( RimWellLogTrack* track )
{
    if ( !track ) return;
    track->createPlotWidget();

    auto* qwtPlotWidget = dynamic_cast<RiuQwtPlotWidget*>( track->plotWidget() );
    if ( !qwtPlotWidget ) return;

    new RiuWellLogCurvePointTracker( qwtPlotWidget->qwtPlot(), curveTextProvider(), track );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveInfoTextProvider* RimDepthTrackPlot::curveTextProvider()
{
    static auto textProvider = RimWellLogCurveInfoTextProvider();
    return &textProvider;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::insertPlot( RimPlot* plot, size_t index )
{
    auto wellLogTrack = dynamic_cast<RimWellLogTrack*>( plot );
    CVF_ASSERT( wellLogTrack && "Only type RimWellLogTrack is supported in RimDepthTrackPlot" );

    if ( wellLogTrack )
    {
        m_plots.insert( index, wellLogTrack );

        if ( m_viewer )
        {
            RimDepthTrackPlot::createPlotWidgetAndAttachCurveTextProvider( wellLogTrack );

            m_viewer->insertPlot( wellLogTrack->plotWidget(), index );
        }
        wellLogTrack->setShowWindow( true );
        onPlotAdditionOrRemoval();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::removePlot( RimPlot* plot )
{
    if ( plot )
    {
        if ( m_viewer )
        {
            m_viewer->removePlot( plot->plotWidget() );
        }
        m_plots.removeChild( plot );

        onPlotAdditionOrRemoval();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot::DepthTypeEnum RimDepthTrackPlot::depthType() const
{
    return m_depthType.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthType( DepthTypeEnum depthType )
{
    m_depthType = depthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RimDepthTrackPlot::depthUnit() const
{
    return m_depthUnit.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDepthTrackPlot::depthAxisTitle() const
{
    QString depthTitle = "Depth";

    switch ( m_depthType.value() )
    {
        case RiaDefines::DepthTypeEnum::MEASURED_DEPTH:
            depthTitle = "MD";
            break;

        case RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH:
            depthTitle = "TVDMSL";
            break;

        case RiaDefines::DepthTypeEnum::PSEUDO_LENGTH:
            depthTitle = "PL";
            break;

        case RiaDefines::DepthTypeEnum::CONNECTION_NUMBER:
            depthTitle = "Connection";
            break;

        case RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB:
            depthTitle = "TVDRKB";
            break;
    }

    if ( m_depthType() == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER ) return depthTitle;

    if ( m_depthUnit == RiaDefines::DepthUnitType::UNIT_METER )
    {
        depthTitle += " [m]";
    }
    else if ( m_depthUnit == RiaDefines::DepthUnitType::UNIT_FEET )
    {
        depthTitle += " [ft]";
    }
    else if ( m_depthUnit == RiaDefines::DepthUnitType::UNIT_NONE )
    {
        depthTitle += "";
    }

    return depthTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::enableDepthAxisGridLines( AxisGridVisibility gridVisibility )
{
    m_depthAxisGridVisibility = gridVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot::AxisGridVisibility RimDepthTrackPlot::depthAxisGridLinesEnabled() const
{
    return m_depthAxisGridVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::Orientation RimDepthTrackPlot::depthOrientation() const
{
    return m_depthOrientation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthOrientation( RiaDefines::Orientation depthOrientation )
{
    m_depthOrientation = depthOrientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::MultiPlotAxisVisibility RimDepthTrackPlot::depthAxisVisibility() const
{
    return m_depthAxisVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthAxisVisibility( RiaDefines::MultiPlotAxisVisibility axisVisibility )
{
    m_depthAxisVisibility = axisVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimDepthTrackPlot::depthAxis() const
{
    return depthAxis( m_depthOrientation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimDepthTrackPlot::depthAxis( RiaDefines::Orientation depthOrientation )
{
    if ( depthOrientation == RiaDefines::Orientation::VERTICAL ) return RiuPlotAxis::defaultLeft();

    return RiuPlotAxis::defaultBottom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimDepthTrackPlot::valueAxis() const
{
    return valueAxis( m_depthOrientation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimDepthTrackPlot::valueAxis( RiaDefines::Orientation depthOrientation )
{
    if ( depthOrientation == RiaDefines::Orientation::VERTICAL ) return RiuPlotAxis::defaultTop();

    return RiuPlotAxis::defaultLeft();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimDepthTrackPlot::annotationAxis() const
{
    return annotationAxis( m_depthOrientation() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimDepthTrackPlot::annotationAxis( RiaDefines::Orientation depthOrientation )
{
    auto riuAxis = valueAxis( depthOrientation );

    auto oppositeAxis = RiaDefines::opposite( riuAxis.axis() );

    return RiuPlotAxis( oppositeAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::updateTrackVisibility()
{
    for ( auto& track : m_plots )
    {
        track->updateCheckStateBasedOnCurveData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAutoScalePropertyValuesEnabled( bool enabled )
{
    for ( auto plot : m_plots.childrenByType() )
    {
        plot->setAutoScalePropertyValuesEnabled( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAutoScaleDepthValuesEnabled( bool enabled )
{
    m_isAutoScaleDepthEnabled = enabled;
    m_isAutoScaleDepthEnabled.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::zoomAll()
{
    setAutoScalePropertyValuesEnabled( true );
    setAutoScaleDepthValuesEnabled( true );
    updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setDepthUnit( RiaDefines::DepthUnitType depthUnit )
{
    m_depthUnit = depthUnit;

    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    calculateAvailableDepthRange();
    updateZoom();
    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateWellLogPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDepthTrackPlot::subTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_subTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDepthTrackPlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimDepthTrackPlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

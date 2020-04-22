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

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimPlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogTrack.h"

#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellLogPlot.h"

#include "cafPdmFieldIOScriptability.h"
#include "cafPdmObjectScriptability.h"
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

} // End namespace caf

CAF_PDM_SOURCE_INIT( RimDepthTrackPlot, "DepthTrackPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDepthTrackPlot::RimDepthTrackPlot()
{
    CAF_PDM_InitObject( "Depth Track Plot", "", "", "A Plot With a shared Depth Axis and Multiple Tracks" );

    CAF_PDM_InitFieldNoDefault( &m_commonDataSource,
                                "CommonDataSource",
                                "Data Source",
                                "",
                                "Change the Data Source of All Curves in the Plot",
                                "" );
    m_commonDataSource.uiCapability()->setUiTreeHidden( true );
    m_commonDataSource.uiCapability()->setUiTreeChildrenHidden( true );
    m_commonDataSource.xmlCapability()->disableIO();
    m_commonDataSource = new RimWellLogCurveCommonDataSource;

    CAF_PDM_InitScriptableFieldWithIO( &m_showPlotWindowTitle, "ShowTitleInPlot", true, "Show Title", "", "", "" );

    CAF_PDM_InitField( &m_plotWindowTitle, "PlotDescription", QString( "" ), "Name", "", "", "" );
    m_plotWindowTitle.xmlCapability()->setIOWritable( false );

    caf::AppEnum<RimDepthTrackPlot::DepthTypeEnum> depthType = RiaDefines::MEASURED_DEPTH;
    CAF_PDM_InitScriptableFieldWithIO( &m_depthType, "DepthType", depthType, "Type", "", "", "" );

    caf::AppEnum<RiaDefines::DepthUnitType> depthUnit = RiaDefines::UNIT_METER;
    CAF_PDM_InitScriptableFieldWithIO( &m_depthUnit, "DepthUnit", depthUnit, "Unit", "", "", "" );

    CAF_PDM_InitScriptableFieldWithIO( &m_minVisibleDepth, "MinimumDepth", 0.0, "Min", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIO( &m_maxVisibleDepth, "MaximumDepth", 1000.0, "Max", "", "", "" );
    m_minVisibleDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    m_maxVisibleDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_depthAxisGridVisibility, "ShowDepthGridLines", "Show Grid Lines", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIO( &m_isAutoScaleDepthEnabled, "AutoScaleDepthEnabled", true, "Auto Scale", "", "", "" );
    m_isAutoScaleDepthEnabled.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "", "", "", "" );
    m_nameConfig.uiCapability()->setUiTreeHidden( true );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );
    m_nameConfig = new RimWellLogPlotNameConfig();

    CAF_PDM_InitFieldNoDefault( &m_plots, "Tracks", "", "", "", "" );
    m_plots.uiCapability()->setUiHidden( true );

    m_availableDepthUnits = {RiaDefines::UNIT_METER, RiaDefines::UNIT_FEET};
    m_availableDepthTypes = {RiaDefines::MEASURED_DEPTH, RiaDefines::TRUE_VERTICAL_DEPTH, RiaDefines::TRUE_VERTICAL_DEPTH_RKB};

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
    m_plots.deleteAllChildObjects();

    cleanupBeforeClose();
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
bool RimDepthTrackPlot::isPlotTitleVisible() const
{
    return m_showPlotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setPlotTitleVisible( bool visible )
{
    m_showPlotWindowTitle = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::addPlot( RimPlot* plot )
{
    insertPlot( plot, m_plots.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::insertPlot( RimPlot* plot, size_t index )
{
    if ( plot )
    {
        m_plots.insert( index, plot );

        if ( m_viewer )
        {
            plot->createPlotWidget();
            m_viewer->insertPlot( plot->viewer(), index );
        }
        plot->setShowWindow( true );
        plot->setLegendsVisible( false );

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
            m_viewer->removePlot( plot->viewer() );
        }
        m_plots.removeChildObject( plot );

        onPlotAdditionOrRemoval();
    }
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
size_t RimDepthTrackPlot::plotIndex( const RimPlot* plot ) const
{
    return m_plots.index( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot* RimDepthTrackPlot::plotByIndex( size_t index ) const
{
    return m_plots[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimDepthTrackPlot::plots() const
{
    return m_plots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimDepthTrackPlot::visiblePlots() const
{
    std::vector<RimPlot*> allVisiblePlots;
    for ( RimPlot* plot : m_plots() )
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
void RimDepthTrackPlot::updateZoom()
{
    if ( m_isAutoScaleDepthEnabled )
    {
        calculateAvailableDepthRange();
        if ( m_minAvailableDepth < HUGE_VAL && m_maxAvailableDepth > -HUGE_VAL )
        {
            m_minVisibleDepth = m_minAvailableDepth;
            m_maxVisibleDepth = m_maxAvailableDepth + 0.01 * ( m_maxAvailableDepth - m_minAvailableDepth );
        }
    }

    for ( RimPlot* plot : plots() )
    {
        static_cast<RimWellLogTrack*>( plot )->setVisibleYRange( m_minVisibleDepth(), m_maxVisibleDepth() );
        plot->updateZoomInQwt();
    }

    if ( m_viewer )
    {
        m_viewer->updateVerticalScrollBar( m_minVisibleDepth(), m_maxVisibleDepth(), m_minAvailableDepth, m_maxAvailableDepth );
    }
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

    setAutoScaleDepthEnabled( false );
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
    uiOrdering.add( &m_depthAxisGridVisibility );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::uiOrderingForAutoName( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotWindowTitle );
    m_nameConfig->uiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDepthTrackPlot::createAutoName() const
{
    QStringList generatedCurveName;

    if ( !m_nameConfig->customName().isEmpty() )
    {
        generatedCurveName.push_back( m_nameConfig->customName() );
    }

    RimCase*     commonCase     = m_commonDataSource->caseToApply();
    RimWellPath* commonWellPath = m_commonDataSource->wellPathToApply();

    QStringList generatedAutoTags;
    if ( m_nameConfig->addCaseName() && commonCase )
    {
        generatedAutoTags.push_back( commonCase->caseUserDescription() );
    }

    if ( m_nameConfig->addWellName() )
    {
        if ( commonWellPath && !commonWellPath->name().isEmpty() )
        {
            generatedAutoTags.push_back( commonWellPath->name() );
        }
        else if ( !m_commonDataSource->simWellNameToApply().isEmpty() )
        {
            generatedAutoTags.push_back( m_commonDataSource->simWellNameToApply() );
        }
    }

    if ( m_nameConfig->addTimeStep() )
    {
        if ( commonCase && m_commonDataSource->timeStepToApply() != -1 )
        {
            generatedAutoTags.push_back( commonCase->timeStepName( m_commonDataSource->timeStepToApply() ) );
        }
    }

    if ( m_nameConfig->addAirGap() )
    {
        if ( commonWellPath )
        {
            RigWellPath* wellPathGeometry = commonWellPath->wellPathGeometry();
            if ( wellPathGeometry )
            {
                double rkb = wellPathGeometry->rkbDiff();
                generatedAutoTags.push_back( QString( "Air Gap = %1 m" ).arg( rkb ) );
            }
        }
    }

    if ( m_nameConfig->addWaterDepth() )
    {
        if ( commonWellPath )
        {
            RigWellPath* wellPathGeometry = commonWellPath->wellPathGeometry();
            if ( wellPathGeometry )
            {
                const std::vector<cvf::Vec3d>& wellPathPoints = wellPathGeometry->wellPathPoints();
                if ( !wellPathPoints.empty() )
                {
                    double tvdmsl = std::abs( wellPathPoints.front()[2] );
                    generatedAutoTags.push_back( QString( "Water Depth = %1 m" ).arg( tvdmsl ) );
                }
            }
        }
    }

    if ( !generatedAutoTags.empty() )
    {
        generatedCurveName.push_back( generatedAutoTags.join( ", " ) );
    }
    return generatedCurveName.join( ": " );
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
        m_viewer->setTitleVisible( m_showPlotWindowTitle() );
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

    auto plotVector = plots();

    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        plotVector[tIdx]->createPlotWidget();
        m_viewer->addPlot( plotVector[tIdx]->viewer() );
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
    m_commonDataSource->updateDefaultOptions();
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
    QString out = description() + "\n";

    for ( RimPlot* plot : plots() )
    {
        if ( plot->showWindow() )
        {
            out += plot->asciiDataForPlotExport();
        }
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::onPlotAdditionOrRemoval()
{
    calculateAvailableDepthRange();
    updateZoom();
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
        m_viewer->scheduleUpdate();
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
void RimDepthTrackPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_minVisibleDepth || changedField == &m_maxVisibleDepth )
    {
        m_isAutoScaleDepthEnabled = false;
        updateZoom();
    }
    else if ( changedField == &m_depthAxisGridVisibility )
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

        bool isTVDRKB = m_depthType == RiaDefines::TRUE_VERTICAL_DEPTH_RKB;
        m_nameConfig->setAutoNameTags( m_nameConfig->addCaseName(),
                                       m_nameConfig->addWellName(),
                                       m_nameConfig->addTimeStep(),
                                       isTVDRKB,
                                       m_nameConfig->addWaterDepth() );

        RimWellAllocationPlot* parentWellAllocation = nullptr;
        this->firstAncestorOrThisOfType( parentWellAllocation );
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
        updateZoom();
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

    caf::PdmUiGroup* plotLayoutGroup = uiOrdering.addNewGroup( "Plot Layout" );
    RimPlotWindow::uiOrderingForPlotLayout( uiConfigName, *plotLayoutGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimDepthTrackPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                        bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

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

    ( *useOptionsOnly ) = true;
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

    performAutoNameUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_minVisibleDepth || field == &m_maxVisibleDepth )
    {
        auto doubleAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( doubleAttr )
        {
            doubleAttr->m_decimals     = 2;
            doubleAttr->m_numberFormat = caf::PdmUiDoubleValueEditorAttribute::NumberFormat::FIXED;
        }
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
        for ( RimPlot* plot : plots() )
        {
            plot->loadDataAndUpdate();
        }
        this->updateZoom();
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
        case RiaDefines::MEASURED_DEPTH:
            depthTitle = "MD";
            break;

        case RiaDefines::TRUE_VERTICAL_DEPTH:
            depthTitle = "TVDMSL";
            break;

        case RiaDefines::PSEUDO_LENGTH:
            depthTitle = "PL";
            break;

        case RiaDefines::CONNECTION_NUMBER:
            depthTitle = "Connection";
            break;

        case RiaDefines::TRUE_VERTICAL_DEPTH_RKB:
            depthTitle = "TVDRKB";
            break;
    }

    if ( m_depthType() == RiaDefines::CONNECTION_NUMBER ) return depthTitle;

    if ( m_depthUnit == RiaDefines::UNIT_METER )
    {
        depthTitle += " [m]";
    }
    else if ( m_depthUnit == RiaDefines::UNIT_FEET )
    {
        depthTitle += " [ft]";
    }
    else if ( m_depthUnit == RiaDefines::UNIT_NONE )
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
void RimDepthTrackPlot::setAutoScaleXEnabled( bool enabled )
{
    for ( RimPlot* plot : plots() )
    {
        plot->setAutoScaleXEnabled( enabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::setAutoScaleDepthEnabled( bool enabled )
{
    m_isAutoScaleDepthEnabled = enabled;
    m_isAutoScaleDepthEnabled.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDepthTrackPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleDepthEnabled( true );
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

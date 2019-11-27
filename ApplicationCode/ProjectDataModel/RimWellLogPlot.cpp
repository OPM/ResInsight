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

#include "RimWellLogPlot.h"

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

#include "RiuMultiPlotWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellLogPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cvfAssert.h"

#include <QKeyEvent>

#include <cmath>

#define RI_LOGPLOT_MINDEPTH_DEFAULT 0.0
#define RI_LOGPLOT_MAXDEPTH_DEFAULT 1000.0

namespace caf
{
template <>
void RimWellLogPlot::AxisGridEnum::setUp()
{
    addItem( RimWellLogPlot::AXIS_GRID_NONE, "GRID_X_NONE", "No Grid Lines" );
    addItem( RimWellLogPlot::AXIS_GRID_MAJOR, "GRID_X_MAJOR", "Major Only" );
    addItem( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR, "GRID_X_MAJOR_AND_MINOR", "Major and Minor" );
    setDefault( RimWellLogPlot::AXIS_GRID_MAJOR );
}

} // End namespace caf

CAF_PDM_SOURCE_INIT( RimWellLogPlot, "WellLogPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::RimWellLogPlot()
{
    CAF_PDM_InitObject( "Well Log Plot", ":/WellLogPlot16x16.png", "", "" );

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

    caf::AppEnum<RimWellLogPlot::DepthTypeEnum> depthType = RiaDefines::MEASURED_DEPTH;
    CAF_PDM_InitField( &m_depthType, "DepthType", depthType, "Type", "", "", "" );

    caf::AppEnum<RiaDefines::DepthUnitType> depthUnit = RiaDefines::UNIT_METER;
    CAF_PDM_InitField( &m_depthUnit, "DepthUnit", depthUnit, "Unit", "", "", "" );

    CAF_PDM_InitField( &m_minVisibleDepth, "MinimumDepth", 0.0, "Min", "", "", "" );
    CAF_PDM_InitField( &m_maxVisibleDepth, "MaximumDepth", 1000.0, "Max", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_depthAxisGridVisibility, "ShowDepthGridLines", "Show Grid Lines", "", "", "" );
    CAF_PDM_InitField( &m_isAutoScaleDepthEnabled, "AutoScaleDepthEnabled", true, "Auto Scale", "", "", "" );
    m_isAutoScaleDepthEnabled.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "", "", "", "" );
    m_nameConfig.uiCapability()->setUiTreeHidden( true );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );
    m_nameConfig = new RimWellLogPlotNameConfig();

    m_availableDepthUnits = {RiaDefines::UNIT_METER, RiaDefines::UNIT_FEET};
    m_availableDepthTypes = {RiaDefines::MEASURED_DEPTH, RiaDefines::TRUE_VERTICAL_DEPTH};

    m_minAvailableDepth = HUGE_VAL;
    m_maxAvailableDepth = -HUGE_VAL;

    m_commonDataSourceEnabled = true;
    m_columnCountEnum         = RimMultiPlotWindow::COLUMNS_UNLIMITED;

    m_plotLegendsHorizontal = false;
    setMultiPlotTitleVisible( false );
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimWellLogPlot& RimWellLogPlot::operator=( RimWellLogPlot&& rhs )
{
    RimMultiPlotWindow::operator=( std::move( rhs ) );

    auto dataSource = rhs.m_commonDataSource();
    rhs.m_commonDataSource.removeChildObject( dataSource );
    m_commonDataSource = dataSource;

    m_depthType               = rhs.m_depthType();
    m_depthUnit               = rhs.m_depthUnit();
    m_minVisibleDepth         = rhs.m_minVisibleDepth();
    m_maxVisibleDepth         = rhs.m_maxVisibleDepth();
    m_depthAxisGridVisibility = rhs.m_depthAxisGridVisibility();
    m_isAutoScaleDepthEnabled = rhs.m_isAutoScaleDepthEnabled();

    auto nameConfig = rhs.m_nameConfig();
    rhs.m_nameConfig.removeChildObject( nameConfig );
    m_nameConfig = nameConfig;

    m_minAvailableDepth = rhs.m_minAvailableDepth;
    m_maxAvailableDepth = rhs.m_maxAvailableDepth;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::~RimWellLogPlot()
{
    delete m_commonDataSource;
    delete m_nameConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellLogPlot::createPlotWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    return createViewWidget( mainWindowParent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateZoom()
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
    }

    if ( m_viewer )
    {
        m_viewer->updateVerticalScrollBar( m_minVisibleDepth(),
                                           m_maxVisibleDepth(),
                                           m_minAvailableDepth,
                                           m_maxAvailableDepth );
    }

    RimMultiPlotWindow::updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthAxisRangeByFactorAndCenter( double zoomFactor, double zoomCenter )
{
    double newMinimum = zoomCenter - ( zoomCenter - m_minVisibleDepth ) * zoomFactor;
    double newMaximum = zoomCenter + ( m_maxVisibleDepth - zoomCenter ) * zoomFactor;

    setDepthAxisRange( newMinimum, newMaximum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthAxisRangeByPanDepth( double panFactor )
{
    double delta = panFactor * ( m_maxVisibleDepth - m_minVisibleDepth );
    setDepthAxisRange( m_minVisibleDepth + delta, m_maxVisibleDepth + delta );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthAxisRange( double minimumDepth, double maximumDepth )
{
    m_minVisibleDepth = minimumDepth;
    m_maxVisibleDepth = maximumDepth;

    m_minVisibleDepth.uiCapability()->updateConnectedEditors();
    m_maxVisibleDepth.uiCapability()->updateConnectedEditors();

    setAutoScaleYEnabled( false );
    updateZoom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::calculateAvailableDepthRange()
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
void RimWellLogPlot::availableDepthRange( double* minimumDepth, double* maximumDepth ) const
{
    *minimumDepth = m_minAvailableDepth;
    *maximumDepth = m_maxAvailableDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::visibleDepthRange( double* minimumDepth, double* maximumDepth ) const
{
    *minimumDepth = m_minVisibleDepth;
    *maximumDepth = m_maxVisibleDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setAutoScaleYEnabled( bool on )
{
    m_isAutoScaleDepthEnabled = on;
    m_isAutoScaleDepthEnabled.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::enableAllAutoNameTags( bool enable )
{
    m_nameConfig->enableAllAutoNameTags( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::uiOrderingForDepthAxis( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
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
void RimWellLogPlot::uiOrderingForAutoName( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotWindowTitle );
    m_nameConfig->uiOrdering( uiConfigName, uiOrdering );
    uiOrdering.add( &m_showIndividualPlotTitles );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot::createAutoName() const
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
RimWellLogPlotNameConfig* RimWellLogPlot::nameConfig() const
{
    return m_nameConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellLogPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new RiuWellLogPlot( this, mainWindowParent );
    recreatePlotWidgets();
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::performAutoNameUpdate()
{
    updateCommonDataSource();
    setMultiPlotTitle( createAutoName() );
    applyPlotWindowTitleToWidgets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::handleKeyPressEvent( QKeyEvent* keyEvent )
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
RimWellLogCurveCommonDataSource* RimWellLogPlot::commonDataSource() const
{
    return m_commonDataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateCommonDataSource()
{
    m_commonDataSource->updateDefaultOptions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setCommonDataSourceEnabled( bool enable )
{
    m_commonDataSourceEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setAvailableDepthUnits( const std::set<RiaDefines::DepthUnitType>& depthUnits )
{
    m_availableDepthUnits = depthUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setAvailableDepthTypes( const std::set<DepthTypeEnum>& depthTypes )
{
    m_availableDepthTypes = depthTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::onPlotAdditionOrRemoval()
{
    calculateAvailableDepthRange();
    updateZoom();
    RimMultiPlotWindow::onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateSubPlotNames()
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
void RimWellLogPlot::updatePlotWindowTitle()
{
    performAutoNameUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimMultiPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

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
        m_isAutoScaleDepthEnabled                   = true;
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
void RimWellLogPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_commonDataSourceEnabled )
    {
        m_commonDataSource->uiOrdering( uiConfigName, uiOrdering );
    }

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "Depth Axis" );
    uiOrderingForDepthAxis( uiConfigName, *gridGroup );

    caf::PdmUiGroup* titleAndLegendsGroup = uiOrdering.addNewGroup( "Plot Layout" );
    uiOrderingForAutoName( uiConfigName, *titleAndLegendsGroup );
    RimPlotWindow::uiOrderingForLegendSettings( uiConfigName, *titleAndLegendsGroup );
    titleAndLegendsGroup->add( &m_columnCountEnum );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                     bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimMultiPlotWindow::calculateValueOptions( fieldNeedingOptions,
                                                                                       useOptionsOnly );

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
void RimWellLogPlot::initAfterRead()
{
    RimMultiPlotWindow::initAfterRead();

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
void RimWellLogPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                            QString                    uiConfigName,
                                            caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_columnCountEnum )
    {
        auto comboAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( comboAttr )
        {
            comboAttr->iconSize = QSize( 24, 14 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellLogPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        RiuWellLogPlot* wellLogViewer = dynamic_cast<RiuWellLogPlot*>( m_viewer.data() );
        CAF_ASSERT( wellLogViewer );
        bool isScrollbarVisible = wellLogViewer->isScrollbarVisible();
        wellLogViewer->setScrollbarVisible( false );
        image = RimMultiPlotWindow::snapshotWindowContent();
        wellLogViewer->setScrollbarVisible( isScrollbarVisible );
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::DepthTypeEnum RimWellLogPlot::depthType() const
{
    return m_depthType.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthType( DepthTypeEnum depthType )
{
    m_depthType = depthType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RimWellLogPlot::depthUnit() const
{
    return m_depthUnit.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot::depthAxisTitle() const
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
void RimWellLogPlot::enableDepthAxisGridLines( AxisGridVisibility gridVisibility )
{
    m_depthAxisGridVisibility = gridVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::AxisGridVisibility RimWellLogPlot::depthAxisGridLinesEnabled() const
{
    return m_depthAxisGridVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthUnit( RiaDefines::DepthUnitType depthUnit )
{
    m_depthUnit = depthUnit;

    updateLayout();
}

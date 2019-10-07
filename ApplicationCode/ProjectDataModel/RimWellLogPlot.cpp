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
#include "RimWellLogCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindow.h"
#include "RiuWellLogPlot.h"
#include "RiuWellLogTrack.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cvfAssert.h"

#include <QKeyEvent>

#include <cmath>

#define RI_LOGPLOT_MINDEPTH_DEFAULT 0.0
#define RI_LOGPLOT_MAXDEPTH_DEFAULT 1000.0

namespace caf
{
template <>
void caf::AppEnum<RimWellLogPlot::DepthTypeEnum>::setUp()
{
    addItem( RimWellLogPlot::MEASURED_DEPTH, "MEASURED_DEPTH", "Measured Depth" );
    addItem( RimWellLogPlot::TRUE_VERTICAL_DEPTH, "TRUE_VERTICAL_DEPTH", "True Vertical Depth (MSL)" );
    addItem( RimWellLogPlot::PSEUDO_LENGTH, "PSEUDO_LENGTH", "Pseudo Length" );
    addItem( RimWellLogPlot::CONNECTION_NUMBER, "CONNECTION_NUMBER", "Connection Number" );
    setDefault( RimWellLogPlot::MEASURED_DEPTH );
}

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

    m_viewer = nullptr;

    CAF_PDM_InitField( &m_userName_OBSOLETE, "PlotDescription", QString( "" ), "Name", "", "", "" );
    m_userName_OBSOLETE.xmlCapability()->setIOWritable( false );

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

    caf::AppEnum<RimWellLogPlot::DepthTypeEnum> depthType = MEASURED_DEPTH;
    CAF_PDM_InitField( &m_depthType, "DepthType", depthType, "Type", "", "", "" );

    caf::AppEnum<RiaDefines::DepthUnitType> depthUnit = RiaDefines::UNIT_METER;
    CAF_PDM_InitField( &m_depthUnit, "DepthUnit", depthUnit, "Unit", "", "", "" );

    CAF_PDM_InitField( &m_minVisibleDepth, "MinimumDepth", 0.0, "Min", "", "", "" );
    CAF_PDM_InitField( &m_maxVisibleDepth, "MaximumDepth", 1000.0, "Max", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_depthAxisGridVisibility, "ShowDepthGridLines", "Show Grid Lines", "", "", "" );
    CAF_PDM_InitField( &m_isAutoScaleDepthEnabled, "AutoScaleDepthEnabled", true, "Auto Scale", "", "", "" );
    m_isAutoScaleDepthEnabled.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_showTitleInPlot, "ShowTitleInPlot", false, "Show Title", "", "", "" );
    CAF_PDM_InitField( &m_showTrackLegends, "ShowTrackLegends", true, "Show Legends", "", "", "" );
    CAF_PDM_InitField( &m_trackLegendsHorizontal, "TrackLegendsHorizontal", false, "Legend Orientation", "", "", "" );
    m_trackLegendsHorizontal.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_tracks, "Tracks", "", "", "", "" );
    m_tracks.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_nameConfig, "NameConfig", "", "", "", "" );
    m_nameConfig.uiCapability()->setUiTreeHidden( true );
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden( true );
    m_nameConfig = new RimWellLogPlotNameConfig();

    m_availableDepthUnits = {RiaDefines::UNIT_METER, RiaDefines::UNIT_FEET};
    m_availableDepthTypes = {MEASURED_DEPTH, TRUE_VERTICAL_DEPTH};

    m_minAvailableDepth = HUGE_VAL;
    m_maxAvailableDepth = -HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
/// Move-assignment operator. Argument has to be passed with std::move()
//--------------------------------------------------------------------------------------------------
RimWellLogPlot& RimWellLogPlot::operator=( RimWellLogPlot&& rhs )
{
    // Don't copy the obsoleted m_userName_OBSOLETE
    // It had its own implementation in the RFT/PLT/WAP plots

    auto dataSource = rhs.m_commonDataSource();
    rhs.m_commonDataSource.removeChildObject( dataSource );
    m_commonDataSource = dataSource;

    m_depthType               = rhs.m_depthType();
    m_depthUnit               = rhs.m_depthUnit();
    m_minVisibleDepth         = rhs.m_minVisibleDepth();
    m_maxVisibleDepth         = rhs.m_maxVisibleDepth();
    m_depthAxisGridVisibility = rhs.m_depthAxisGridVisibility();
    m_isAutoScaleDepthEnabled = rhs.m_isAutoScaleDepthEnabled();
    m_showTitleInPlot         = rhs.m_showTitleInPlot();
    m_showTrackLegends        = rhs.m_showTrackLegends();
    m_trackLegendsHorizontal  = rhs.m_trackLegendsHorizontal();

    // Move all tracks
    std::vector<RimWellLogTrack*> tracks = rhs.m_tracks.childObjects();
    rhs.m_tracks.clear();
    for ( RimWellLogTrack* track : tracks )
    {
        m_tracks.push_back( track );
    }

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
    removeMdiWindowFromMdiArea();

    m_tracks.deleteAllChildObjects();

    deleteViewWidget();
    delete m_commonDataSource;
    delete m_nameConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellLogPlot::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_minVisibleDepth || changedField == &m_maxVisibleDepth )
    {
        applyDepthZoomFromVisibleDepth();

        m_isAutoScaleDepthEnabled = false;
    }
    else if ( changedField == &m_showTrackLegends || changedField == &m_trackLegendsHorizontal ||
              changedField == &m_depthAxisGridVisibility )
    {
        updateTracks();
    }
    else if ( changedField == &m_showTitleInPlot )
    {
        m_viewer->setTitleVisible( m_showTitleInPlot() );
    }
    else if ( changedField == &m_isAutoScaleDepthEnabled )
    {
        updateDepthZoom();
    }
    else if ( changedField == &m_depthType )
    {
        onDepthTypeChanged();
        updateTracks();
        updateDepthZoom();
    }
    else if ( changedField == &m_depthUnit )
    {
        updateTracks();
        updateDepthZoom();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                     bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

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
    else if ( fieldNeedingOptions == &m_trackLegendsHorizontal )
    {
        options.push_back( caf::PdmOptionItemInfo( "Vertical", QVariant::fromValue( false ) ) );
        options.push_back( caf::PdmOptionItemInfo( "Horizontal", QVariant::fromValue( true ) ) );
    }

    ( *useOptionsOnly ) = true;
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellLogPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        m_viewer->setScrollbarVisible( false );
        QPixmap pix = m_viewer->grab();
        image       = pix.toImage();
        m_viewer->setScrollbarVisible( true );
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::addTrack( RimWellLogTrack* track )
{
    m_tracks.push_back( track );
    if ( m_viewer )
    {
        track->recreateViewer();
        m_viewer->addTrackPlot( track->viewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::insertTrack( RimWellLogTrack* track, size_t index )
{
    m_tracks.insert( index, track );

    if ( m_viewer )
    {
        track->recreateViewer();
        m_viewer->insertTrackPlot( track->viewer(), index );
    }

    updateTrackNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::removeTrack( RimWellLogTrack* track )
{
    if ( track )
    {
        if ( m_viewer ) m_viewer->removeTrackPlot( track->viewer() );
        m_tracks.removeChildObject( track );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RimWellLogPlot::trackByIndex( size_t index ) const
{
    return m_tracks[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellLogPlot::firstVisibleTrackIndex() const
{
    for ( size_t i = 0; i < m_tracks.size(); ++i )
    {
        if ( m_tracks[i]->isVisible() )
        {
            return i;
        }
    }
    return std::numeric_limits<size_t>::max();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogTrack*> RimWellLogPlot::tracks() const
{
    return m_tracks.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogTrack*> RimWellLogPlot::visibleTracks() const
{
    std::vector<RimWellLogTrack*> tracks;
    for ( RimWellLogTrack* track : m_tracks() )
    {
        if ( track->isVisible() )
        {
            tracks.push_back( track );
        }
    }
    return tracks;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthZoomByFactorAndCenter( double zoomFactor, double zoomCenter )
{
    double newMinimum = zoomCenter - ( zoomCenter - m_minVisibleDepth ) * zoomFactor;
    double newMaximum = zoomCenter + ( m_maxVisibleDepth - zoomCenter ) * zoomFactor;

    setDepthZoomMinMax( newMinimum, newMaximum );
    setDepthAutoZoom( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::panDepth( double panFactor )
{
    double delta = panFactor * ( m_maxVisibleDepth - m_minVisibleDepth );
    setDepthZoomMinMax( m_minVisibleDepth + delta, m_maxVisibleDepth + delta );
    setDepthAutoZoom( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthZoomMinMax( double minimumDepth, double maximumDepth )
{
    m_minVisibleDepth = minimumDepth;
    m_maxVisibleDepth = maximumDepth;

    m_minVisibleDepth.uiCapability()->updateConnectedEditors();
    m_maxVisibleDepth.uiCapability()->updateConnectedEditors();

    applyDepthZoomFromVisibleDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::calculateAvailableDepthRange()
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    for ( size_t tIdx = 0; tIdx < m_tracks.size(); tIdx++ )
    {
        double minTrackDepth = HUGE_VAL;
        double maxTrackDepth = -HUGE_VAL;

        if ( m_tracks[tIdx]->isVisible() )
        {
            m_tracks[tIdx]->availableDepthRange( &minTrackDepth, &maxTrackDepth );

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

    m_minAvailableDepth = minDepth;
    m_maxAvailableDepth = maxDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::availableDepthRange( double* minimumDepth, double* maximumDepth ) const
{
    if ( hasAvailableDepthRange() )
    {
        *minimumDepth = m_minAvailableDepth;
        *maximumDepth = m_maxAvailableDepth;
    }
    else
    {
        *minimumDepth = RI_LOGPLOT_MINDEPTH_DEFAULT;
        *maximumDepth = RI_LOGPLOT_MAXDEPTH_DEFAULT;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::hasAvailableDepthRange() const
{
    return m_minAvailableDepth < HUGE_VAL && m_maxAvailableDepth > -HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::zoomAll()
{
    setDepthAutoZoom( true );
    updateDepthZoom();
    updateTracks( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthAutoZoom( bool on )
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
QString RimWellLogPlot::asciiDataForPlotExport() const
{
    QString out = description() + "\n";

    for ( RimWellLogTrack* track : m_tracks )
    {
        if ( !track->isVisible() ) continue;

        out += "\n" + track->description() + "\n";

        std::vector<RimWellLogCurve*> curves = track->curvesVector();

        std::vector<QString>             curveNames;
        std::vector<double>              curveDepths;
        std::vector<std::vector<double>> curvesPlotXValues;

        for ( RimWellLogCurve* curve : curves )
        {
            if ( !curve->isCurveVisible() ) continue;

            const RigWellLogCurveData* curveData = curve->curveData();
            if ( !curveData ) continue;
            curveNames.push_back( curve->curveName() );

            if ( curveNames.size() == 1 )
            {
                if ( depthType() == TRUE_VERTICAL_DEPTH )
                {
                    curveDepths = curveData->trueDepthPlotValues( depthUnit() );
                }
                else
                {
                    curveDepths = curveData->measuredDepthPlotValues( depthUnit() );
                }
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

        for ( size_t i = 0; i < curveDepths.size(); ++i )
        {
            if ( i == 0 )
            {
                if ( depthType() == CONNECTION_NUMBER )
                    out += "Connection";
                else if ( depthType() == MEASURED_DEPTH )
                    out += "MD   ";
                else if ( depthType() == PSEUDO_LENGTH )
                    out += "PL   ";
                else if ( depthType() == TRUE_VERTICAL_DEPTH )
                    out += "TVD  ";
                for ( QString name : curveNames )
                    out += "  \t" + name;
                out += "\n";
            }
            else if ( curveDepths[i] == curveDepths[i - 1] )
            {
                continue;
            }

            out += QString::number( curveDepths[i], 'f', 3 );
            for ( std::vector<double> plotVector : curvesPlotXValues )
            {
                out += " \t" + QString::number( plotVector[i], 'g' );
            }
            out += "\n";
        }
    }

    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::uiOrderingForDepthAxis( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "Depth Axis" );

    if ( m_availableDepthTypes.size() > 1u )
    {
        gridGroup->add( &m_depthType );
    }

    if ( m_availableDepthUnits.size() > 1u )
    {
        gridGroup->add( &m_depthUnit );
    }

    gridGroup->add( &m_minVisibleDepth );
    gridGroup->add( &m_maxVisibleDepth );
    gridGroup->add( &m_depthAxisGridVisibility );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::uiOrderingForPlotSettings( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* titleAndLegendsGroup = uiOrdering.addNewGroup( "Title and Legends" );
    titleAndLegendsGroup->add( &m_showTrackLegends );
    titleAndLegendsGroup->add( &m_trackLegendsHorizontal );
    titleAndLegendsGroup->add( &m_showTitleInPlot );
    m_nameConfig->uiOrdering( "", *titleAndLegendsGroup );
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
void RimWellLogPlot::performAutoNameUpdate()
{
    updateCommonDataSource();
    this->updatePlotTitle();
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
void RimWellLogPlot::depthZoomMinMax( double* minimumDepth, double* maximumDepth ) const
{
    *minimumDepth = m_minVisibleDepth;
    *maximumDepth = m_maxVisibleDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimViewWindow::defineUiOrdering( uiConfigName, uiOrdering );

    m_commonDataSource->uiOrdering( uiConfigName, uiOrdering );
    uiOrderingForDepthAxis( uiOrdering );
    uiOrderingForPlotSettings( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlot::userDescriptionField()
{
    return m_nameConfig->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();
    updatePlotTitle();
    updateTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateTracks( bool autoScaleXAxis )
{
    if ( m_showWindow )
    {
        for ( size_t tIdx = 0; tIdx < m_tracks.size(); ++tIdx )
        {
            m_tracks[tIdx]->loadDataAndUpdate();
            if ( autoScaleXAxis )
            {
                m_tracks[tIdx]->setAutoScaleXEnabled( true );
                m_tracks[tIdx]->calculateXZoomRangeAndUpdateQwt();
                m_tracks[tIdx]->updateAxisAndGridTickIntervals();
            }
        }

        calculateAvailableDepthRange();
        applyDepthZoomFromVisibleDepth();
        if ( m_viewer ) m_viewer->updateChildrenLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateTrackNames()
{
    for ( size_t tIdx = 0; tIdx < m_tracks.size(); tIdx++ )
    {
        QString            description = m_tracks[tIdx]->description();
        QRegularExpression regexp( "Track \\d+" );
        description.replace( regexp, QString( "Track %1" ).arg( tIdx + 1 ) );
        m_tracks[tIdx]->setDescription( description );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updateDepthZoom()
{
    if ( m_isAutoScaleDepthEnabled )
    {
        applyZoomAllDepths();
    }
    else
    {
        applyDepthZoomFromVisibleDepth();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::applyDepthZoomFromVisibleDepth()
{
    if ( m_viewer )
    {
        double minDepth = m_minVisibleDepth < HUGE_VAL ? m_minVisibleDepth : RI_LOGPLOT_MINDEPTH_DEFAULT;
        double maxDepth = m_maxVisibleDepth > -HUGE_VAL ? m_maxVisibleDepth : RI_LOGPLOT_MAXDEPTH_DEFAULT;

        m_viewer->setDepthZoomAndReplot( minDepth, maxDepth );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::applyZoomAllDepths()
{
    calculateAvailableDepthRange();

    if ( hasAvailableDepthRange() )
    {
        setDepthZoomMinMax( m_minAvailableDepth,
                            m_maxAvailableDepth + 0.01 * ( m_maxAvailableDepth - m_minAvailableDepth ) );
    }
    else
    {
        setDepthZoomMinMax( RI_LOGPLOT_MINDEPTH_DEFAULT, RI_LOGPLOT_MAXDEPTH_DEFAULT );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::recreateTrackPlots()
{
    CVF_ASSERT( m_viewer );

    for ( size_t tIdx = 0; tIdx < m_tracks.size(); ++tIdx )
    {
        m_tracks[tIdx]->recreateViewer();
        m_viewer->addTrackPlot( m_tracks[tIdx]->viewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::detachAllCurves()
{
    for ( size_t tIdx = 0; tIdx < m_tracks.size(); ++tIdx )
    {
        m_tracks[tIdx]->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDescription( const QString& description )
{
    m_nameConfig->setCustomName( description );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogPlot::description() const
{
    return m_nameConfig->customName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellLogPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new RiuWellLogPlot( this, mainWindowParent );
    recreateTrackPlots();
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::deleteViewWidget()
{
    detachAllCurves();

    if ( m_viewer )
    {
        m_viewer->deleteLater();
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::initAfterRead()
{
    RimViewWindow::initAfterRead();

    updateCommonDataSource();
    if ( !m_userName_OBSOLETE().isEmpty() )
    {
        m_nameConfig->setCustomName( m_userName_OBSOLETE() );
    }
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
QString RimWellLogPlot::depthPlotTitle() const
{
    QString depthTitle = "Depth";

    switch ( m_depthType.value() )
    {
        case MEASURED_DEPTH:
            depthTitle = "MD";
            break;

        case TRUE_VERTICAL_DEPTH:
            depthTitle = "TVDMSL";
            break;

        case PSEUDO_LENGTH:
            depthTitle = "PL";
            break;

        case CONNECTION_NUMBER:
            depthTitle = "Connection";
            break;
    }

    if ( m_depthType() == CONNECTION_NUMBER ) return depthTitle;

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
void RimWellLogPlot::enableDepthGridLines( AxisGridVisibility gridVisibility )
{
    m_depthAxisGridVisibility = gridVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot::AxisGridVisibility RimWellLogPlot::depthGridLinesVisibility() const
{
    return m_depthAxisGridVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::isPlotTitleVisible() const
{
    return m_showTitleInPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setPlotTitleVisible( bool visible )
{
    m_showTitleInPlot = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::areTrackLegendsVisible() const
{
    return m_showTrackLegends();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setTrackLegendsVisible( bool doShow )
{
    m_showTrackLegends = doShow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlot::areTrackLegendsHorizontal() const
{
    return m_trackLegendsHorizontal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setTrackLegendsHorizontal( bool horizontal )
{
    m_trackLegendsHorizontal = horizontal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimWellLogPlot::trackIndex( const RimWellLogTrack* track ) const
{
    return m_tracks.index( track );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::setDepthUnit( RiaDefines::DepthUnitType depthUnit )
{
    m_depthUnit = depthUnit;

    updateTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::updatePlotTitle()
{
    if ( m_viewer )
    {
        m_viewer->setPlotTitle( this->createAutoName() );
    }
    updateMdiWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// Default empty implementation
//--------------------------------------------------------------------------------------------------
void RimWellLogPlot::onDepthTypeChanged() {}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later versio<n.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimStreamlineInViewCollection.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigTracerPoint.h"

#include "RimEclipseCase.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimStreamline.h"
#include "RimStreamlineDataAccess.h"
#include "RimStreamlineGenerator.h"

#include "RiaLogging.h"

#include "RiuViewer.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"

#include "cvfCollection.h"

#include <cmath>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimStreamlineInViewCollection::VisualizationMode>::setUp()
{
    addItem( RimStreamlineInViewCollection::VisualizationMode::ANIMATION, "ANIMATION", "Animation" );
    addItem( RimStreamlineInViewCollection::VisualizationMode::MANUAL, "MANUAL", "Manual control" );
    setDefault( RimStreamlineInViewCollection::VisualizationMode::ANIMATION );
}

template <>
void AppEnum<RimStreamlineInViewCollection::StreamlinePhaseType>::setUp()
{
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::OIL, "OIL", "Oil" );
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::GAS, "GAS", "Gas" );
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::WATER, "WATER", "Water" );
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::COMBINED, "COMBINED", "Combined" );
    setDefault( RimStreamlineInViewCollection::StreamlinePhaseType::COMBINED );
}

template <>
void AppEnum<RimStreamlineInViewCollection::ColorMode>::setUp()
{
    addItem( RimStreamlineInViewCollection::ColorMode::PHASE_COLORS, "PHASE_COLORS", "Dominant Phase" );
    addItem( RimStreamlineInViewCollection::ColorMode::VELOCITY, "VELOCITY", "Velocity" );
    setDefault( RimStreamlineInViewCollection::ColorMode::PHASE_COLORS );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimStreamlineInViewCollection, "StreamlineInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::RimStreamlineInViewCollection()
    : m_shouldGenerateTracers( false )
    , m_currentTimestep( -1 )
{
    CAF_PDM_InitObject( "Streamlines", ":/Erase.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LOG10_CONTINUOUS );
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name", "", "", "" );
    m_collectionName = "Streamlines";
    m_collectionName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowThreshold, "FlowThreshold", "Flow Threshold [m/day]", "", "", "" );
    m_flowThreshold = 0.01;

    CAF_PDM_InitFieldNoDefault( &m_lengthThreshold, "LengthThreshold", "Minimum Length [m]", "", "", "" );
    m_lengthThreshold = 100.0;

    CAF_PDM_InitFieldNoDefault( &m_resolution, "Resolution", "Resolution [days]", "", "", "" );
    m_resolution = 20.0;

    CAF_PDM_InitFieldNoDefault( &m_maxDays, "MaxDays", "Max Days", "", "", "" );
    m_maxDays = 5000;

    CAF_PDM_InitFieldNoDefault( &m_useProducers, "UseProducers", "Producer Wells", "", "", "" );
    m_useProducers = true;

    CAF_PDM_InitFieldNoDefault( &m_useInjectors, "UseInjectors", "Injector Wells", "", "", "" );
    m_useInjectors = true;

    CAF_PDM_InitFieldNoDefault( &m_phases, "Phase", "Phase", "", "", "" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_visualizationMode, "VisualizationMode", "Visualization Mode", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_colorMode, "ColorMode", "Colors", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_animationSpeed, "AnimationSpeed", "Animation Speed", "", "", "" );
    m_animationSpeed.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_animationSpeed = 10;

    CAF_PDM_InitFieldNoDefault( &m_animationIndex, "AnimationIndex", "Animation Index", "", "", "" );
    m_animationIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_animationIndex    = 0;
    m_maxAnimationIndex = 0;

    CAF_PDM_InitFieldNoDefault( &m_scaleFactor, "ScaleFactor", "Scale Factor", "", "", "" );
    m_scaleFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_scaleFactor = 100.0;

    CAF_PDM_InitFieldNoDefault( &m_tracerLength, "TracerLength", "Tracer Length", "", "", "" );
    m_tracerLength.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_tracerLength = 100;

    CAF_PDM_InitFieldNoDefault( &m_streamlines, "Streamlines", "Streamlines", "", "", "" );
    m_streamlines.uiCapability()->setUiTreeHidden( true );
    m_streamlines.xmlCapability()->disableIO();

    m_eclipseCase = nullptr;

    // we are a topmost folder, do not delete us
    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::~RimStreamlineInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimStreamlineInViewCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::setEclipseCase( RimEclipseCase* reservoir )
{
    m_shouldGenerateTracers = true;
    m_eclipseCase           = reservoir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStreamlineInViewCollection::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStreamlineInViewCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStreamlineInViewCollection::shouldBeAvailable() const
{
    if ( ( eclipseCase() == nullptr ) || ( eclipseCase()->eclipseCaseData() == nullptr ) ) return false;

    return ( dynamic_cast<RimEclipseInputCase*>( eclipseCase() ) == nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<RiaDefines::PhaseType> RimStreamlineInViewCollection::phases() const
{
    std::list<RiaDefines::PhaseType> retval;

    switch ( m_phases() )
    {
        case StreamlinePhaseType::OIL:
            retval.push_back( RiaDefines::PhaseType::OIL_PHASE );
            break;
        case StreamlinePhaseType::GAS:
            retval.push_back( RiaDefines::PhaseType::GAS_PHASE );
            break;
        case StreamlinePhaseType::WATER:
            retval.push_back( RiaDefines::PhaseType::WATER_PHASE );
            break;

        case StreamlinePhaseType::COMBINED:
            retval.push_back( RiaDefines::PhaseType::OIL_PHASE );
            retval.push_back( RiaDefines::PhaseType::GAS_PHASE );
            retval.push_back( RiaDefines::PhaseType::WATER_PHASE );
            break;

        default:
            break;
    }
    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::VisualizationMode RimStreamlineInViewCollection::visualizationMode() const
{
    return m_visualizationMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::ColorMode RimStreamlineInViewCollection::colorMode() const
{
    return m_colorMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<RigTracer>& RimStreamlineInViewCollection::tracers()
{
    m_activeTracers.clear();

    for ( auto& streamline : m_streamlines() )
    {
        if ( streamline->tracer().size() > 1 )
        {
            m_activeTracers.push_back( streamline->tracer() );
        }
    }
    return m_activeTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimRegularLegendConfig* RimStreamlineInViewCollection::legendConfig() const
{
    return m_legendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::mappingRange( double& min, double& max ) const
{
    min = HUGE_VAL;
    max = -HUGE_VAL;

    for ( auto& streamline : m_streamlines() )
    {
        const RigTracer& tracer = streamline->tracer();
        for ( size_t i = 0; i < tracer.tracerPoints().size() - 1; i++ )
        {
            min = std::min( min, tracer.tracerPoints()[i].absValue() );
            max = std::max( max, tracer.tracerPoints()[i].absValue() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                         bool       isUsingOverrideViewer )
{
    if ( m_isActive() && ( m_streamlines.size() > 0 ) && m_legendConfig->showLegend() )
    {
        QString title = "Streamlines: \n";
        title += m_phases().uiText() + " flow\n";
        m_legendConfig->setTitle( title );

        double minResultValue;
        double maxResultValue;
        mappingRange( minResultValue, maxResultValue );
        m_legendConfig->setAutomaticRanges( minResultValue, maxResultValue, minResultValue, maxResultValue );

        double posClosestToZero = HUGE_VAL;
        double negClosestToZero = -HUGE_VAL;
        m_legendConfig->setClosestToZeroValues( posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero );

        if ( colorMode() == ColorMode::VELOCITY )
        {
            nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( m_legendConfig->titledOverlayFrame(),
                                                                      isUsingOverrideViewer );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Main entry point for triggering a streamline update
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::updateFromCurrentTimeStep( int timeStep )
{
    if ( timeStep != m_currentTimestep )
    {
        m_shouldGenerateTracers = true;
        m_currentTimestep       = timeStep;
    }

    updateStreamlines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimStreamlineInViewCollection::animationSpeed() const
{
    return m_animationSpeed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimStreamlineInViewCollection::animationIndex() const
{
    return m_animationIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::scaleFactor() const
{
    return m_scaleFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimStreamlineInViewCollection::tracerLength() const
{
    return m_tracerLength();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::refresh()
{
    m_shouldGenerateTracers = true;
    updateStreamlines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::findStartCells( int                                       timeIdx,
                                                    std::vector<std::pair<QString, RigCell>>& outInjectorCells,
                                                    std::vector<std::pair<QString, RigCell>>& outProducerCells )
{
    // get the simulation wells
    const cvf::Collection<RigSimWellData>& simWellData = eclipseCase()->eclipseCaseData()->wellResults();

    std::vector<const RigGridBase*> grids;
    eclipseCase()->eclipseCaseData()->allGrids( &grids );

    // go through all sim wells and find all open producer and injector cells for the given timestep
    for ( auto& swdata : simWellData )
    {
        if ( !swdata->hasWellResult( timeIdx ) || !swdata->hasAnyValidCells( timeIdx ) ) continue;

        RigWellResultFrame frame = swdata->wellResultFrame( timeIdx );

        for ( auto& branch : frame.m_wellResultBranches )
        {
            for ( const auto& point : branch.m_branchResultPoints )
            {
                if ( point.isValid() && point.m_isOpen )
                {
                    RigCell cell = grids[point.m_gridIndex]->cell( point.m_gridCellIndex );
                    if ( frame.m_productionType == RiaDefines::WellProductionType::PRODUCER )
                    {
                        outProducerCells.push_back( std::pair<QString, RigCell>( swdata->m_wellName, cell ) );
                    }
                    else if ( frame.m_productionType != RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
                    {
                        outInjectorCells.push_back( std::pair<QString, RigCell>( swdata->m_wellName, cell ) );
                    }
                    m_wellCellIds.insert( cell.gridLocalCellIndex() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Main entry point for generating streamlines/tracers
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::updateStreamlines()
{
    bool bNeedRedraw = ( m_streamlines.size() > 0 );

    // get the view
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );
    if ( !eclView ) return;

    if ( m_shouldGenerateTracers && isActive() )
    {
        // reset generated streamlines
        m_streamlines().clear();
        m_wellCellIds.clear();

        // get current simulation timestep
        int timeIdx = eclView->currentTimeStep();

        // get the well cells we should start growing from
        std::vector<std::pair<QString, RigCell>> seedCellsInjector;
        std::vector<std::pair<QString, RigCell>> seedCellsProducer;
        findStartCells( timeIdx, seedCellsInjector, seedCellsProducer );

        // set up the data access helper
        RimStreamlineDataAccess dataAccess;
        bool                    accessOk = dataAccess.setupDataAccess( eclipseCase()->eclipseCaseData()->mainGrid(),
                                                    eclipseCase()->eclipseCaseData(),
                                                    phases(),
                                                    timeIdx );

        // did we find the data we needed?
        if ( accessOk )
        {
            // setup the streamline generator to use
            RimStreamlineGenerator generator( m_wellCellIds );
            generator.setLimits( m_flowThreshold, m_maxDays, m_resolution, m_lengthThreshold );
            generator.initGenerator( &dataAccess, phases() );

            const int reverseDirection = -1.0;
            const int normalDirection  = 1.0;

            std::list<RimStreamline*> streamlines;

            size_t seedsCount = 0;
            if ( m_useInjectors() ) seedsCount += seedCellsInjector.size();
            if ( m_useProducers() ) seedsCount += seedCellsProducer.size();

            caf::ProgressInfo streamlineProgress( seedsCount, "Generating streamlines, please wait..." );

            // generate tracers for all injectors
            if ( m_useInjectors() )
            {
                for ( auto& cellinfo : seedCellsInjector )
                {
                    generator.generateTracer( cellinfo.second, normalDirection, cellinfo.first, streamlines );
                    streamlineProgress.incrementProgress();
                }
            }

            // generate tracers for all producers, make sure to invert the direction to backtrack the traces
            if ( m_useProducers() )
            {
                for ( auto& cellinfo : seedCellsProducer )
                {
                    generator.generateTracer( cellinfo.second, reverseDirection, cellinfo.first, streamlines );
                    streamlineProgress.incrementProgress();
                }
            }

            // get rid of empty and too short streamlines
            m_maxAnimationIndex = 0;
            for ( auto& sline : streamlines )
            {
                if ( sline && sline->size() > 1 )
                {
                    m_maxAnimationIndex = std::max( sline->size(), m_maxAnimationIndex );
                    m_streamlines.push_back( sline );
                    sline = nullptr;
                }
            }

            bNeedRedraw = true;
        }
        m_shouldGenerateTracers = false;
    }

    if ( bNeedRedraw )
    {
        eclView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
// debug output
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::outputSummary() const
{
    qDebug() << "Generated" << m_streamlines.size() << " tracers";

    for ( auto s : m_streamlines )
    {
        QString debStr( "Tracer for well " );
        debStr += s->simWellName();
        debStr += " of length ";
        debStr += QString::number( s->tracer().totalDistance(), 'f', 2 );
        debStr += " meters and ";
        debStr += QString::number( s->tracer().size() );
        debStr += " points.";
        qDebug() << debStr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::initAfterRead()
{
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );

    if ( eclView && m_isActive() ) eclView->requestAnimationTimer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );

    caf::PdmUiGroup* dataGroup = uiOrdering.addNewGroup( "Data Selection" );
    dataGroup->add( &m_phases );
    dataGroup->add( &m_flowThreshold );
    dataGroup->add( &m_lengthThreshold );
    dataGroup->add( &m_resolution );
    dataGroup->add( &m_maxDays );

    caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup( "Well Selection" );
    wellGroup->add( &m_useInjectors );
    wellGroup->add( &m_useProducers );

    caf::PdmUiGroup* visualizationGroup = uiOrdering.addNewGroup( "Visualization Settings" );
    visualizationGroup->add( &m_visualizationMode );
    visualizationGroup->add( &m_colorMode );

    if ( m_visualizationMode() == VisualizationMode::ANIMATION )
    {
        visualizationGroup->add( &m_animationSpeed );
        visualizationGroup->add( &m_tracerLength );
    }
    else if ( m_visualizationMode() == VisualizationMode::MANUAL )
    {
        visualizationGroup->add( &m_animationIndex );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                          QString                 uiConfigName /*= "" */ )
{
    uiTreeOrdering.add( &m_legendConfig );
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_animationSpeed )
    {
        caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 1;
            myAttr->m_maximum = 100;
        }
    }
    else if ( field == &m_animationIndex )
    {
        caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = static_cast<int>( m_maxAnimationIndex );
        }
    }
    else if ( field == &m_tracerLength )
    {
        caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 1;
            myAttr->m_maximum = static_cast<int>( 1000 );
        }
    }
    else if ( field == &m_scaleFactor )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 0.1;
            myAttr->m_maximum = 10000.0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField == &m_animationSpeed || changedField == &m_animationIndex || changedField == &m_tracerLength ||
         changedField == &m_visualizationMode )
    {
        return;
    }

    if ( changedField == &m_lengthThreshold || changedField == &m_flowThreshold || changedField == &m_resolution ||
         changedField == &m_maxDays || changedField == &m_useProducers || changedField == &m_useInjectors ||
         changedField == &m_phases )
    {
        m_shouldGenerateTracers = true;
    }

    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );

    if ( changedField == &m_isActive )
    {
        if ( eclView )
        {
            if ( m_isActive() )
                eclView->requestAnimationTimer();
            else
                eclView->releaseAnimationTimer();
        }
    }

    if ( eclView ) eclView->scheduleCreateDisplayModelAndRedraw();
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimStreamline.h"

#include "RiaLogging.h"

#include "RiuViewer.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfCollection.h"

#include <math.h>
#include <qdebug.h>

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
    addItem( RimStreamlineInViewCollection::VisualizationMode::VECTORS, "VECTORS", "Vectors" );
    setDefault( RimStreamlineInViewCollection::VisualizationMode::ANIMATION );
}

template <>
void AppEnum<RimStreamlineInViewCollection::StreamlinePhaseType>::setUp()
{
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::OIL, "OIL", "Oil" );
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::GAS, "GAS", "Gas" );
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::WATER, "WATER", "Water" );
    addItem( RimStreamlineInViewCollection::StreamlinePhaseType::COMBINED, "COMBINED", "Combined" );
    setDefault( RimStreamlineInViewCollection::StreamlinePhaseType::OIL );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimStreamlineInViewCollection, "StreamlineInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::RimStreamlineInViewCollection()
{
    CAF_PDM_InitScriptableObject( "Streamlines", ":/Erase.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendDefinition", "Color Legend", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_collectionName, "Name", "Name", "", "", "" );
    m_collectionName = "Streamlines";
    m_collectionName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_flowThreshold, "FlowThreshold", "Flow Threshold [m/day]", "", "", "" );
    m_flowThreshold = 0.000001;

    CAF_PDM_InitScriptableFieldNoDefault( &m_lengthThreshold, "LengthThreshold", "Minimum Length [m]", "", "", "" );
    m_lengthThreshold = 20.0;

    CAF_PDM_InitScriptableFieldNoDefault( &m_resolution, "Resolution", "Resolution [days]", "", "", "" );
    m_resolution = 10.0;

    CAF_PDM_InitScriptableFieldNoDefault( &m_density, "Density", "Density", "", "", "" );
    m_density.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_density = 2;

    CAF_PDM_InitScriptableFieldNoDefault( &m_maxDays, "MaxDays", "Max Days ", "", "", "" );
    m_maxDays = 50000;

    CAF_PDM_InitScriptableField( &m_phases, "Phase", StreamlinePhaseTypeEnum( StreamlinePhaseType::OIL ), "Phase", "", "", "" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );
    m_isActive.xmlCapability()->setIOReadable( false );

    CAF_PDM_InitFieldNoDefault( &m_visualizationMode, "VisualizationMode", "Visualization Mode", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_distanceBetweenTracerPoints,
                                "DistanceBetweenTracerPoints",
                                "Distance Between Tracer Points",
                                "",
                                "",
                                "" );
    m_distanceBetweenTracerPoints = 10.0;

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

    CAF_PDM_InitFieldNoDefault( &m_injectionDeltaTime, "InjectionDeltaTime", "Pause between injections", "", "", "" );
    m_injectionDeltaTime.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_injectionDeltaTime = 500;

    CAF_PDM_InitScriptableFieldNoDefault( &m_streamlines, "Streamlines", "Streamlines", "", "", "" );
    m_streamlines.uiCapability()->setUiTreeHidden( true );
    m_streamlines.xmlCapability()->disableIO();

    uiCapability()->setUiTreeChildrenHidden( true );

    m_eclipseCase = nullptr;

    // we are a topmost folder, do not delete us
    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::~RimStreamlineInViewCollection()
{
    delete m_legendConfig;
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
    m_eclipseCase = reservoir;
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
QString RimStreamlineInViewCollection::gridResultNameFromPhase( RiaDefines::PhaseType              phase,
                                                                cvf::StructGridInterface::FaceType faceIdx ) const
{
    QString retval = "";
    switch ( phase )
    {
        case RiaDefines::PhaseType::GAS_PHASE:
            retval += "FLRGAS";
            break;
        case RiaDefines::PhaseType::OIL_PHASE:
            retval += "FLROIL";
            break;
        case RiaDefines::PhaseType::WATER_PHASE:
            retval += "FLRWAT";
            break;
        default:
            CAF_ASSERT( false );
            break;
    }

    switch ( faceIdx )
    {
        case cvf::StructGridInterface::FaceType::POS_I:
            retval += "I+";
            break;
        case cvf::StructGridInterface::FaceType::POS_J:
            retval += "J+";
            break;
        case cvf::StructGridInterface::FaceType::POS_K:
            retval += "K+";
            break;
        default:
            CAF_ASSERT( false );
            break;
    }

    return retval;
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
            // TODO - add filter for active simulation wells here
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
    if ( m_isActive() )
    {
        m_legendConfig->setTitle( QString( "Streamlines: \n" ) );
        double minResultValue;
        double maxResultValue;
        mappingRange( minResultValue, maxResultValue );
        m_legendConfig->setAutomaticRanges( minResultValue, maxResultValue, minResultValue, maxResultValue );
        m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );

        double posClosestToZero = HUGE_VAL;
        double negClosestToZero = -HUGE_VAL;
        m_legendConfig->setClosestToZeroValues( posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero );
        nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( m_legendConfig->titledOverlayFrame(),
                                                                  isUsingOverrideViewer );
    }
}

void RimStreamlineInViewCollection::updateFromCurrentTimeStep()
{
    updateStreamlines();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::distanceBetweenTracerPoints() const
{
    return m_distanceBetweenTracerPoints();
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
size_t RimStreamlineInViewCollection::injectionDeltaTime() const
{
    return m_injectionDeltaTime();
}

//--------------------------------------------------------------------------------------------------
/// Main entry point for generating streamlines/tracers
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::updateStreamlines()
{
    // reset generated streamlines
    m_streamlines().clear();
    m_wellCellIds.clear();

    // get the view
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );
    if ( !eclView ) return;

    if ( !m_isActive() )
    {
        eclView->loadDataAndUpdate();
        return;
    }

    // get current simulation timestep
    int timeIdx = eclView->currentTimeStep();

    // get the simulation wells
    const cvf::Collection<RigSimWellData>& simWellData = eclipseCase()->eclipseCaseData()->wellResults();

    std::vector<std::pair<QString, RigCell>> seedCellsInjector;
    std::vector<std::pair<QString, RigCell>> seedCellsProducer;

    std::vector<const RigGridBase*> grids;
    eclipseCase()->eclipseCaseData()->allGrids( &grids );

    // go through all sim wells and find all open producer and injector cells for the selected phase
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
                    if ( frame.m_productionType == RigWellResultFrame::WellProductionType::PRODUCER )
                    {
                        seedCellsProducer.push_back( std::pair<QString, RigCell>( swdata->m_wellName, cell ) );
                    }
                    else if ( frame.m_productionType != RigWellResultFrame::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
                    {
                        seedCellsInjector.push_back( std::pair<QString, RigCell>( swdata->m_wellName, cell ) );
                    }
                    m_wellCellIds.insert( cell.mainGridCellIndex() );
                }
            }
        }
    }

    qDebug() << "Found " << seedCellsProducer.size() << " producing cells";
    qDebug() << "Found " << seedCellsInjector.size() << " injector cells";

    // make sure we have the data we need loaded, and set up data accessors to access the data later
    for ( auto phase : phases() )
        loadDataIfMissing( phase, timeIdx );

    if ( !setupDataAccessors( timeIdx ) ) return;

    const int reverseDirection = -1.0;
    const int normalDirection  = 1.0;

    // generate tracers for all injectors
    for ( auto& cell : seedCellsInjector )
    {
        generateTracer( cell.second, normalDirection, cell.first );
    }
    // generate tracers for all producers, make sure to invert the direction to backtrack the traces
    for ( auto& cell : seedCellsProducer )
    {
        generateTracer( cell.second, reverseDirection, cell.first );
    }

    outputSummary();

    m_maxAnimationIndex = 0;
    for ( auto& s : m_streamlines )
    {
        if ( s->tracer().tracerPoints().size() > 0 )
        {
            m_maxAnimationIndex = std::max( s->tracer().tracerPoints().size(), m_maxAnimationIndex );
        }
    }
    eclView->loadDataAndUpdate();
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
/// Make sure we have the data we need loaded for the given phase and time step index
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::loadDataIfMissing( RiaDefines::PhaseType phase, int timeIdx )
{
    RigCaseCellResultsData* data = m_eclipseCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    std::vector<cvf::StructGridInterface::FaceType> faces = {
        cvf::StructGridInterface::FaceType::POS_I,
        cvf::StructGridInterface::FaceType::POS_J,
        cvf::StructGridInterface::FaceType::POS_K,
    };

    for ( auto cubeFaceIdx : faces )
    {
        QString                 resultname = gridResultNameFromPhase( phase, cubeFaceIdx );
        RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultname );

        data->ensureKnownResultLoadedForTimeStep( address, timeIdx );
    }
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
    dataGroup->add( &m_density );

    caf::PdmUiGroup* visualizationGroup = uiOrdering.addNewGroup( "Visualization Settings" );
    visualizationGroup->add( &m_visualizationMode );
    // visualizationGroup->add( &m_distanceBetweenTracerPoints );
    if ( m_visualizationMode() == VisualizationMode::ANIMATION )
    {
        visualizationGroup->add( &m_animationSpeed );
        visualizationGroup->add( &m_tracerLength );
        visualizationGroup->add( &m_injectionDeltaTime );
    }
    if ( m_visualizationMode() == VisualizationMode::MANUAL )
    {
        visualizationGroup->add( &m_animationIndex );
    }
    if ( m_visualizationMode() == VisualizationMode::VECTORS )
    {
        visualizationGroup->add( &m_scaleFactor );
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
    else if ( field == &m_injectionDeltaTime )
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
    else if ( field == &m_density )
    {
        caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 2;
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
    if ( changedField == &m_animationSpeed || changedField == &m_animationIndex ||
         changedField == &m_injectionDeltaTime || changedField == &m_tracerLength )
    {
        return;
    }

    if ( changedField == &m_visualizationMode &&
         qvariant_cast<int>( newValue ) != static_cast<int>( VisualizationMode::VECTORS ) &&
         qvariant_cast<int>( oldValue ) != static_cast<int>( VisualizationMode::VECTORS ) )
    {
        return;
    }

    updateStreamlines();
}

//--------------------------------------------------------------------------------------------------
/// Return a data accessor for the given phase and face and time step
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RimStreamlineInViewCollection::getDataAccessor( cvf::StructGridInterface::FaceType faceIdx,
                                                                            RiaDefines::PhaseType              phase,
                                                                            int                                timeIdx )
{
    QString resultname = gridResultNameFromPhase( phase, faceIdx );

    RiaDefines::PorosityModelType porModel = RiaDefines::PorosityModelType::MATRIX_MODEL;
    int                           gridIdx  = 0;

    RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultname );

    return RigResultAccessorFactory::createFromResultAddress( eclipseCase()->eclipseCaseData(),
                                                              gridIdx,
                                                              porModel,
                                                              timeIdx,
                                                              address );
}

//--------------------------------------------------------------------------------------------------
/// Set up data accessors to access the flroil/gas/wat data for all faces
//--------------------------------------------------------------------------------------------------
bool RimStreamlineInViewCollection::setupDataAccessors( int timeIdx )
{
    m_dataAccess.clear();

    for ( auto phase : phases() )
    {
        m_dataAccess[phase] = std::vector<cvf::ref<RigResultAccessor>>();

        // Note: NEG_? accessors are set to POS_? accessors, but will be referring the neighbor cell when used
        m_dataAccess[phase].push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_I, phase, timeIdx ) );
        m_dataAccess[phase].push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_I, phase, timeIdx ) );
        m_dataAccess[phase].push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_J, phase, timeIdx ) );
        m_dataAccess[phase].push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_J, phase, timeIdx ) );
        m_dataAccess[phase].push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_K, phase, timeIdx ) );
        m_dataAccess[phase].push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_K, phase, timeIdx ) );
    }

    for ( auto& pair : m_dataAccess )
    {
        for ( auto& access : pair.second )
        {
            if ( access.isNull() ) return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Return the neighbor cell of the given cell and face
//--------------------------------------------------------------------------------------------------
RigCell* RimStreamlineInViewCollection::findNeighborCell( RigCell                            cell,
                                                          RigGridBase*                       grid,
                                                          cvf::StructGridInterface::FaceType face ) const
{
    size_t i, j, k;

    grid->ijkFromCellIndexUnguarded( cell.mainGridCellIndex(), &i, &j, &k );

    size_t neighborIdx;
    if ( grid->cellIJKNeighbor( i, j, k, face, &neighborIdx ) )
    {
        return &grid->cell( neighborIdx );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Find and return the local grid cell index of all up to 26 neighbor cells to the given cell
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimStreamlineInViewCollection::findNeighborCellIndexes( RigCell* cell, RigGridBase* grid ) const
{
    std::vector<size_t> neighbors;

    size_t ni, nj, nk;

    grid->ijkFromCellIndexUnguarded( cell->mainGridCellIndex(), &ni, &nj, &nk );

    for ( size_t i = ni - 1; i <= ni + 1; i++ )
        for ( size_t j = nj - 1; j <= nj + 1; j++ )
            for ( size_t k = nk - 1; k <= nk + 1; k++ )
            {
                if ( grid->isCellValid( i, j, k ) )
                {
                    size_t cellIndex = grid->cellIndexFromIJK( i, j, k );
                    if ( ( ni == i ) && ( nj == j ) && ( nk == k ) ) continue;
                    neighbors.push_back( cellIndex );
                }
            }

    return neighbors;
}

//--------------------------------------------------------------------------------------------------
/// Return the face scalar value for the given cell and NEG_? face, by using the neighbor cell
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::negFaceValue( RigCell                            cell,
                                                    cvf::StructGridInterface::FaceType faceIdx,
                                                    RigGridBase*                       grid,
                                                    RiaDefines::PhaseType              phase ) const
{
    double retval = 0.0;

    RigCell* neighborCell = findNeighborCell( cell, grid, faceIdx );
    if ( neighborCell && !neighborCell->isInvalid() )
    {
        std::vector<cvf::ref<RigResultAccessor>> access = m_dataAccess.at( phase );
        retval      = access[faceIdx]->cellScalar( neighborCell->mainGridCellIndex() );
        double area = cell.faceNormalWithAreaLength( faceIdx ).length();
        if ( area != 0.0 )
            retval /= area;
        else
            retval = 0.0;

        if ( isinf( retval ) ) retval = 0.0;
    }

    return retval;
}

//--------------------------------------------------------------------------------------------------
/// Return the face scalar value for the given cell and POS_? face
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::posFaceValue( RigCell                            cell,
                                                    cvf::StructGridInterface::FaceType faceIdx,
                                                    RiaDefines::PhaseType              phase ) const
{
    std::vector<cvf::ref<RigResultAccessor>> access = m_dataAccess.at( phase );
    double                                   retval = access[faceIdx]->cellScalar( cell.mainGridCellIndex() );
    double                                   length = cell.faceNormalWithAreaLength( faceIdx ).length();
    if ( length != 0.0 )
        retval /= length;
    else
        retval = 0.0;

    if ( isinf( retval ) ) retval = 0.0;

    return retval;
}

//--------------------------------------------------------------------------------------------------
/// Return the face scalar value for the given cell and face
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::faceValue( RigCell                            cell,
                                                 cvf::StructGridInterface::FaceType faceIdx,
                                                 RigGridBase*                       grid,
                                                 RiaDefines::PhaseType              phase ) const
{
    if ( faceIdx % 2 == 0 ) return posFaceValue( cell, faceIdx, phase );

    // NEG_? face values must be read from the neighbor cells
    return negFaceValue( cell, faceIdx, grid, phase );
}

//--------------------------------------------------------------------------------------------------
/// Calculate the average direction inside the cell by adding the scaled face normals of all faces
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStreamlineInViewCollection::cellDirection( RigCell                cell,
                                                         RigGridBase*           grid,
                                                         RiaDefines::PhaseType& dominantPhaseOut ) const
{
    cvf::Vec3d direction( 0, 0, 0 );

    std::vector<cvf::StructGridInterface::FaceType> faces = { cvf::StructGridInterface::FaceType::POS_I,
                                                              cvf::StructGridInterface::FaceType::NEG_I,
                                                              cvf::StructGridInterface::FaceType::POS_J,
                                                              cvf::StructGridInterface::FaceType::NEG_J,
                                                              cvf::StructGridInterface::FaceType::POS_K,
                                                              cvf::StructGridInterface::FaceType::NEG_K };

    double maxval    = 0.0;
    dominantPhaseOut = phases().front();

    for ( auto face : faces )
    {
        cvf::Vec3d faceNorm = cell.faceNormalWithAreaLength( face );
        faceNorm.normalize();
        double faceval = 0.0;
        for ( auto phase : phases() )
        {
            double tmpval = faceValue( cell, face, grid, phase );
            if ( abs( tmpval ) > maxval )
            {
                maxval           = abs( tmpval );
                dominantPhaseOut = phase;
            }
            faceval += tmpval;
        }
        faceNorm *= faceval;
        if ( face % 2 != 0 ) faceNorm *= -1.0;

        direction += faceNorm;
    }
    return direction;
}

//--------------------------------------------------------------------------------------------------
/// Return the cell bounding box for the given cell
/// TODO - use the BB feature from RigCell once cell filters have been merged in
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimStreamlineInViewCollection::cellBoundingBox( RigCell* cell, RigGridBase* grid ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;
    grid->cellCornerVertices( cell->mainGridCellIndex(), hexCorners.data() );
    cvf::BoundingBox bb;
    for ( const auto& corner : hexCorners )
    {
        bb.add( corner );
    }
    return bb;
}

//--------------------------------------------------------------------------------------------------
/// Generate multiple start posisions for the given cell face, using the user specified tracer density
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::generateStartPositions( RigCell                            cell,
                                                            cvf::StructGridInterface::FaceType faceIdx,
                                                            std::list<cvf::Vec3d>&             positions )
{
    cvf::Vec3d center = cell.faceCenter( faceIdx );

    std::array<cvf::Vec3d, 4> corners;
    cell.faceCorners( faceIdx, &corners );

    positions.push_back( center );

    // if density is zero, just return face center
    if ( m_density() == 0 )
    {
        return;
    }

    for ( const auto& pos : corners )
        positions.push_back( pos );

    // if density is 1, return face center and corners
    if ( m_density() == 1 )
    {
        return;
    }

    // if density is 2, add some more points in-between
    for ( size_t cornerIdx = 0; cornerIdx < 4; cornerIdx++ )
    {
        cvf::Vec3d xa = corners[cornerIdx] - center;
        positions.push_back( center + xa / 2.0 );
        cvf::Vec3d xab  = corners[( cornerIdx + 1 ) % 4] - corners[cornerIdx];
        cvf::Vec3d ab_2 = corners[cornerIdx] + xab / 2.0;
        positions.push_back( ab_2 );
        cvf::Vec3d xab_2 = ab_2 - center;
        positions.push_back( center + xab_2 / 2.0 );
    }
}

//--------------------------------------------------------------------------------------------------
/// Grow tracers for all faces of the input cell, possibly reversing the direction for producers
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::generateTracer( RigCell cell, double direction, QString simWellName )
{
    RigMainGrid* grid = eclipseCase()->eclipseCaseData()->mainGrid();

    std::vector<cvf::StructGridInterface::FaceType> faces = { cvf::StructGridInterface::FaceType::POS_I,
                                                              cvf::StructGridInterface::FaceType::NEG_I,
                                                              cvf::StructGridInterface::FaceType::POS_J,
                                                              cvf::StructGridInterface::FaceType::NEG_J,
                                                              cvf::StructGridInterface::FaceType::POS_K,
                                                              cvf::StructGridInterface::FaceType::NEG_K };

    size_t ni, nj, nk;
    grid->ijkFromCellIndexUnguarded( cell.mainGridCellIndex(), &ni, &nj, &nk );

    cvf::Vec3d cellCenter = cell.center();

    RiaDefines::PhaseType dominantStartPhase = phases().front();

    // try to generate a tracer for all faces in the selected cell
    for ( auto faceIdx : faces )
    {
        // get the face normal for the current face, scale it with the flow, and check that it is still valid
        cvf::Vec3d startDirection = cell.faceNormalWithAreaLength( faceIdx );
        startDirection.normalize();
        double faceval = 0.0;
        double maxval  = 0.0;

        for ( auto phase : phases() )
        {
            double tmpval = faceValue( cell, faceIdx, grid, phase );
            if ( abs( tmpval ) > maxval )
            {
                dominantStartPhase = phase;
                maxval             = abs( tmpval );
            }
            faceval += tmpval;
        }
        startDirection *= faceval;
        startDirection *= direction;
        // skip vectors with inf values
        if ( startDirection.isUndefined() ) continue;
        // if too little flow, skip making tracer for this face
        if ( startDirection.length() <= m_flowThreshold ) continue;

        // generate a set of start positions starting in the face
        std::list<cvf::Vec3d> positions;
        generateStartPositions( cell, faceIdx, positions );

        // calculate the max number of steps based on user settings for length and resolution
        const int maxSteps = (int)( m_maxDays / m_resolution );

        // get the neighbour cell for this face, this is where the tracer should start growing
        RigCell* startCell = findNeighborCell( cell, grid, faceIdx );
        if ( startCell == nullptr ) continue;

        for ( const cvf::Vec3d& startPosition : positions )
        {
            if ( startPosition.isUndefined() ) continue;

            RigCell*   curCell = startCell;
            cvf::Vec3d curPos  = startPosition;
            int        curStep = 0;

            std::set<size_t> visitedCellsIdx;

            grid->ijkFromCellIndexUnguarded( curCell->mainGridCellIndex(), &ni, &nj, &nk );

            // create the streamline we should store the tracer points in
            RimStreamline* streamLine = new RimStreamline( simWellName );
            streamLine->addTracerPoint( cellCenter, startDirection, dominantStartPhase );

            // get the current cell bounding box and average direction movement vector
            cvf::BoundingBox      bb            = cellBoundingBox( curCell, grid );
            RiaDefines::PhaseType dominantPhase = dominantStartPhase;
            cvf::Vec3d            curDirection  = cellDirection( *curCell, grid, dominantPhase ) * direction;

            while ( curStep < maxSteps )
            {
                // keep track of where we have been to avoid loops
                visitedCellsIdx.insert( curCell->mainGridCellIndex() );

                grid->ijkFromCellIndexUnguarded( curCell->mainGridCellIndex(), &ni, &nj, &nk );

                // is this a well cell, if so, stop growing
                if ( m_wellCellIds.count( curCell->mainGridCellIndex() ) > 0 ) break;

                // while we stay in the cell, keep moving in the same direction
                bool stop = false;
                while ( bb.contains( curPos ) )
                {
                    streamLine->addTracerPoint( curPos, curDirection, dominantPhase );
                    curPos += curDirection * m_resolution;
                    curStep++;
                    // TODO - calculate new direction here based on how close we are to the various cell faces
                    //        To simplify things - keep size of direction the same as the cell direction, just update
                    //        where we are heading
                    stop = ( curStep >= maxSteps ) || ( curDirection.length() < m_flowThreshold );
                    if ( stop ) break;
                }
                if ( stop ) break;

                // we have exited the cell we were in, find the next cell (should be one of our neighbours)
                RigCell             tmpCell;
                RigCell*            nextCell  = nullptr;
                std::vector<size_t> neighbors = findNeighborCellIndexes( curCell, grid );
                for ( auto cellIdx : neighbors )
                {
                    tmpCell = grid->cell( cellIdx );

                    bb = cellBoundingBox( &tmpCell, grid );
                    if ( bb.contains( curPos ) )
                    {
                        nextCell = &tmpCell;
                        break;
                    }
                }

                // no neighbour found, stop this tracer
                if ( nextCell == nullptr ) break;

                // have we been here, if so stop?
                if ( visitedCellsIdx.count( nextCell->mainGridCellIndex() ) > 0 ) break;

                // update our current cell and direction
                curCell      = nextCell;
                curDirection = cellDirection( *curCell, grid, dominantPhase ) * direction;

                // stop if too little flow
                if ( curDirection.length() < m_flowThreshold ) break;
            }

            if ( streamLine->tracer().size() > 1 )
            {
                // make sure the streamline points with the flow towards the producer
                if ( direction < 0.0 ) streamLine->reverse();

                double distance = streamLine->tracer().totalDistance();

                if ( distance >= m_lengthThreshold )
                {
                    streamLine->generateStatistics();
                    m_streamlines.push_back( streamLine );
                    streamLine = nullptr;
                }
            }
            if ( streamLine ) delete streamLine;
        }
    }

    return;
}

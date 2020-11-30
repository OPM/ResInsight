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
    addItem( RimStreamlineInViewCollection::VisualizationMode::CURVES, "CURVES", "Curves" );
    addItem( RimStreamlineInViewCollection::VisualizationMode::VECTORS, "VECTORS", "Vectors" );
    addItem( RimStreamlineInViewCollection::VisualizationMode::DEBUG, "DEBUG", "Debug" );
    setDefault( RimStreamlineInViewCollection::VisualizationMode::CURVES );
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

    CAF_PDM_InitScriptableFieldNoDefault( &m_flowThreshold, "FlowThreshold", "Flow Threshold [m/day]", "", "", "" );
    m_flowThreshold = 0.000001;

    CAF_PDM_InitScriptableFieldNoDefault( &m_lengthThreshold, "LengthThreshold", "Minimum Length [m]", "", "", "" );
    m_lengthThreshold = 20.0;

    CAF_PDM_InitScriptableFieldNoDefault( &m_resolution, "Resolution", "Resolution [days]", "", "", "" );
    m_resolution = 1.0;

    CAF_PDM_InitScriptableFieldNoDefault( &m_maxDays, "MaxDays", "Max Days ", "", "", "" );
    m_maxDays = 50000;

    CAF_PDM_InitScriptableField( &m_phase,
                                 "Phase",
                                 caf::AppEnum<RiaDefines::PhaseType>( RiaDefines::PhaseType::OIL_PHASE ),
                                 "Phase",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_visualizationMode, "VisualizationMode", "Visualization Mode", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_distanceBetweenTracerPoints,
                                "DistanceBetweenTracerPoints",
                                "Distance Between Tracer Points",
                                "",
                                "",
                                "" );
    m_distanceBetweenTracerPoints = 10.0;

    CAF_PDM_InitFieldNoDefault( &m_animationSpeed, "AnimationSpeed", "Animation Speed", "", "", "" );
    m_animationSpeed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_animationSpeed = 1.0;

    CAF_PDM_InitFieldNoDefault( &m_scaleFactor, "ScaleFactor", "Scale Factor", "", "", "" );
    m_scaleFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_scaleFactor = 1.0;

    CAF_PDM_InitFieldNoDefault( &m_tracerLength, "TracerLength", "Tracer Length", "", "", "" );
    m_tracerLength.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_tracerLength = 1.0;

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
RiaDefines::PhaseType RimStreamlineInViewCollection::phase() const
{
    return m_phase();
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

    for ( auto streamline : m_streamlines() )
    {
        // TODO - add filter for active simulation wells here
        m_activeTracers.push_back( streamline->tracer() );
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

    for ( auto streamline : m_streamlines() )
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
    goForIt();
    m_legendConfig->setTitle( QString( "Streamlines: \n" ) );
    double minResultValue;
    double maxResultValue;
    mappingRange( minResultValue, maxResultValue );
    m_legendConfig->setAutomaticRanges( minResultValue, maxResultValue, minResultValue, maxResultValue );
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );

    double posClosestToZero = HUGE_VAL;
    double negClosestToZero = -HUGE_VAL;
    m_legendConfig->setClosestToZeroValues( posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero );
    nativeOrOverrideViewer->addColorLegendToBottomLeftCorner( m_legendConfig->titledOverlayFrame(), isUsingOverrideViewer );
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
double RimStreamlineInViewCollection::animationSpeed() const
{
    return m_animationSpeed();
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
double RimStreamlineInViewCollection::tracerLength() const
{
    return m_tracerLength();
}

//--------------------------------------------------------------------------------------------------
/// Main entry point for generating streamlines/tracers
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::goForIt()
{
    // reset generated streamlines
    m_streamlines().clear();
    m_wellCellIds.clear();

    // get the view
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );
    if ( !eclView ) return;

    // get current simulation timestep
    int timeIdx = eclView->currentTimeStep();

    // get the simulation wells
    const cvf::Collection<RigSimWellData>& simWellData = eclipseCase()->eclipseCaseData()->wellResults();

    std::vector<std::pair<QString, RigCell>> seedCellsInjector;
    std::vector<std::pair<QString, RigCell>> seedCellsProducer;

    std::vector<const RigGridBase*> grids;
    eclipseCase()->eclipseCaseData()->allGrids( &grids );

    // go through all sim wells and find all open producer and injector cells for the selected phase
    for ( auto swdata : simWellData )
    {
        if ( !swdata->hasWellResult( timeIdx ) || !swdata->hasAnyValidCells( timeIdx ) ) continue;

        RigWellResultFrame frame = swdata->wellResultFrame( timeIdx );

        for ( auto branch : frame.m_wellResultBranches )
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
    loadDataIfMissing( phase(), timeIdx );
    if ( !setupDataAccessors( m_phase(), timeIdx ) ) return;

    const int reverseDirection = -1.0;
    const int normalDirection  = 1.0;

    // generate tracers for all injectors
    for ( auto cell : seedCellsInjector )
    {
        generateTracer( cell.second, normalDirection, cell.first );
    }
    // generate tracers for all producers, make sure to invert the direction to backtrack the traces
    for ( auto cell : seedCellsProducer )
    {
        generateTracer( cell.second, reverseDirection, cell.first );
    }

    outputSummary();

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
    {
        caf::PdmUiGroup* dataGroup = uiOrdering.addNewGroup( "Data Selection" );
        dataGroup->add( &m_collectionName );
        dataGroup->add( &m_phase );
        dataGroup->add( &m_flowThreshold );
        dataGroup->add( &m_lengthThreshold );
        dataGroup->add( &m_resolution );
        dataGroup->add( &m_maxDays );
    }
    {
        caf::PdmUiGroup* visualizationGroup = uiOrdering.addNewGroup( "Visualization Settings" );
        visualizationGroup->add( &m_visualizationMode );
        visualizationGroup->add( &m_distanceBetweenTracerPoints );
        visualizationGroup->add( &m_animationSpeed );
        visualizationGroup->add( &m_scaleFactor );
        visualizationGroup->add( &m_tracerLength );
    }
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
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum = 0.1;
        myAttr->m_maximum = 100.0;
    }
    else if ( field == &m_scaleFactor )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum = 0.1;
        myAttr->m_maximum = 1000.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField != &m_animationSpeed )
    {
        goForIt();
    }
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
    // RigCaseCellResultsData* data = m_eclipseCase->eclipseCaseData()->results( porModel );

    return RigResultAccessorFactory::createFromResultAddress( eclipseCase()->eclipseCaseData(),
                                                              gridIdx,
                                                              porModel,
                                                              timeIdx,
                                                              address );
}

//--------------------------------------------------------------------------------------------------
/// Set up data accessors to access the flroil/gas/wat data for all faces
//--------------------------------------------------------------------------------------------------
bool RimStreamlineInViewCollection::setupDataAccessors( RiaDefines::PhaseType phase, int timeIdx )
{
    m_dataAccess.clear();

    // NEG_? accessors are set to POS_? accessors, but will be referring the neighbor cell when used
    m_dataAccess.push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_I, phase, timeIdx ) );
    m_dataAccess.push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_I, phase, timeIdx ) );
    m_dataAccess.push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_J, phase, timeIdx ) );
    m_dataAccess.push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_J, phase, timeIdx ) );
    m_dataAccess.push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_K, phase, timeIdx ) );
    m_dataAccess.push_back( getDataAccessor( cvf::StructGridInterface::FaceType::POS_K, phase, timeIdx ) );

    for ( auto access : m_dataAccess )
    {
        if ( access.isNull() ) return false;
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
                                                    RigGridBase*                       grid ) const
{
    double retval = 0.0;

    RigCell* neighborCell = findNeighborCell( cell, grid, faceIdx );
    if ( neighborCell && !neighborCell->isInvalid() )
    {
        retval      = m_dataAccess[faceIdx]->cellScalar( neighborCell->mainGridCellIndex() );
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
double RimStreamlineInViewCollection::posFaceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx ) const
{
    double retval = m_dataAccess[faceIdx]->cellScalar( cell.mainGridCellIndex() );
    double length = cell.faceNormalWithAreaLength( faceIdx ).length();
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
                                                 RigGridBase*                       grid ) const
{
    if ( faceIdx % 2 == 0 ) return posFaceValue( cell, faceIdx );

    // NEG_? face values must be read from the neighbor cells
    return negFaceValue( cell, faceIdx, grid );
}

//--------------------------------------------------------------------------------------------------
/// Return a vector with the the 6 face scalars of the given cell
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStreamlineInViewCollection::faceValues( RigCell cell, RigGridBase* grid )
{
    double valPosI = posFaceValue( cell, cvf::StructGridInterface::FaceType::POS_I );
    double valPosJ = posFaceValue( cell, cvf::StructGridInterface::FaceType::POS_J );
    double valPosK = posFaceValue( cell, cvf::StructGridInterface::FaceType::POS_K );
    double valNegI = negFaceValue( cell, cvf::StructGridInterface::FaceType::NEG_I, grid );
    double valNegJ = negFaceValue( cell, cvf::StructGridInterface::FaceType::NEG_J, grid );
    double valNegK = negFaceValue( cell, cvf::StructGridInterface::FaceType::NEG_K, grid );

    std::vector<double> retval = { valPosI, valNegI, valPosJ, valNegJ, valPosK, valNegK };
    return retval;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the average direction inside the cell by adding the scaled face normals of all faces
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStreamlineInViewCollection::cellDirection( RigCell cell, RigGridBase* grid ) const
{
    cvf::Vec3d direction( 0, 0, 0 );

    std::vector<cvf::StructGridInterface::FaceType> faces = { cvf::StructGridInterface::FaceType::POS_I,
                                                              cvf::StructGridInterface::FaceType::NEG_I,
                                                              cvf::StructGridInterface::FaceType::POS_J,
                                                              cvf::StructGridInterface::FaceType::NEG_J,
                                                              cvf::StructGridInterface::FaceType::POS_K,
                                                              cvf::StructGridInterface::FaceType::NEG_K };

    for ( auto face : faces )
    {
        cvf::Vec3d faceNorm = cell.faceNormalWithAreaLength( face );
        faceNorm.normalize();
        faceNorm *= faceValue( cell, face, grid );
        if ( face % 2 != 0 ) faceNorm *= -1.0;

        direction += faceNorm;
    }
    return direction;
}

//--------------------------------------------------------------------------------------------------
/// Return the cell bounding box for the given cell
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
/// Grow tracers for all faces of the input cell, possibly reversing the direction for producers
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::generateTracer( RigCell cell, double direction, QString simWellName )
{
    RigMainGrid*        grid     = eclipseCase()->eclipseCaseData()->mainGrid();
    std::vector<double> faceVals = faceValues( cell, grid );

    std::vector<cvf::StructGridInterface::FaceType> faces = { cvf::StructGridInterface::FaceType::POS_I,
                                                              cvf::StructGridInterface::FaceType::NEG_I,
                                                              cvf::StructGridInterface::FaceType::POS_J,
                                                              cvf::StructGridInterface::FaceType::NEG_J,
                                                              cvf::StructGridInterface::FaceType::POS_K,
                                                              cvf::StructGridInterface::FaceType::NEG_K };

    // if ( cell.mainGridCellIndex() != 65613 ) return;
    size_t ni, nj, nk;
    grid->ijkFromCellIndexUnguarded( cell.mainGridCellIndex(), &ni, &nj, &nk );

    // qDebug() << "Tracing from cell " << cell.mainGridCellIndex();
    // qDebug() << "Simwell cell IJK: " << ni << nj << nk;

    // try to generate a tracer for all faces in the selected cell
    for ( auto faceIdx : faces )
    {
        // if too little flow, skip making tracer for this face
        if ( faceVals[faceIdx] <= m_flowThreshold )
        {
            // qDebug() << "Skipping cell " << cell.mainGridCellIndex() << "for face direction " << faceIdx;
            continue;
        }

        // get the face normal for the current face, scale it with the flow, and check that it is still valid
        cvf::Vec3d startDirection = cell.faceNormalWithAreaLength( faceIdx );
        startDirection.normalize();
        startDirection *= faceVals[faceIdx];
        startDirection *= direction;
        // skip vectors with inf values
        if ( startDirection.isUndefined() ) continue;

        // start the tracer in the face center of the cell
        cvf::Vec3d startPosition = cell.faceCenter( faceIdx );
        if ( startPosition.isUndefined() ) continue;

        // get the neighbour cell for this face, this is where the tracer should start growing
        RigCell* startCell = findNeighborCell( cell, grid, faceIdx );
        if ( startCell == nullptr ) continue;

        RigCell*   curCell = startCell;
        cvf::Vec3d curPos  = startPosition;
        // size_t     startIdx = startCell->mainGridCellIndex();
        // if ( startIdx != 43717 ) continue;

        std::set<size_t> visitedCellsIdx;

        grid->ijkFromCellIndexUnguarded( curCell->mainGridCellIndex(), &ni, &nj, &nk );

        // qDebug() << "Starting tracer from cell" << startCell->mainGridCellIndex() << "in direction"
        //         << startDirection.x() << startDirection.y() << startDirection.z() << "for face" << faceIdx;
        // qDebug() << "Start cell IJK: " << ni << nj << nk;

        // create the streamline we should store the tracer points in
        RimStreamline* streamLine = new RimStreamline( simWellName );

        // calculate the max number of steps based on user settings for length and resolution
        int maxSteps = (int)( m_maxDays / m_resolution );
        int curStep  = 0;

        // get the current cell bounding box and average direction movement vector
        cvf::BoundingBox bb           = cellBoundingBox( curCell, grid );
        cvf::Vec3d       curDirection = cellDirection( *curCell, grid ) * direction;

        RigCell cell;

        while ( curStep < maxSteps )
        {
            // keep track of where we have been to avoid loops
            visitedCellsIdx.insert( curCell->mainGridCellIndex() );

            bool stop = false;

            grid->ijkFromCellIndexUnguarded( curCell->mainGridCellIndex(), &ni, &nj, &nk );
            // qDebug() << "In cell " << curCell->mainGridCellIndex() << "(" << ni << nj << nk << ") with direction "
            //         << curDirection.x() << curDirection.y() << curDirection.z();

            // is this a well cell, if so, stop growing
            if ( m_wellCellIds.count( curCell->mainGridCellIndex() ) > 0 )
            {
                // qDebug() << "Tracer stopped. Found well in cell " << curCell->mainGridCellIndex();
                break;
            }

            // while we stay in the cell, keep moving in the same direction
            while ( bb.contains( curPos ) )
            {
                streamLine->addTracerPoint( curPos, curDirection );
                curPos += curDirection * m_resolution;
                curStep++;
                stop = ( curStep >= maxSteps ) || ( curDirection.length() < m_flowThreshold );
                if ( stop ) break;
            }

            if ( stop ) break;

            // we have exited the cell we were in, find the next cell (should be one of our neighbours)
            RigCell*            nextCell  = nullptr;
            std::vector<size_t> neighbors = findNeighborCellIndexes( curCell, grid );
            for ( auto cellIdx : neighbors )
            {
                cell = grid->cell( cellIdx );
                bb   = cellBoundingBox( &cell, grid );
                if ( bb.contains( curPos ) )
                {
                    nextCell = &cell;
                    break;
                }
            }

            // no neighbour found, stop this tracer
            if ( nextCell == nullptr )
            {
                // qDebug() << "Tracer stopped. No neighbor found.";
                break;
            }

            if ( visitedCellsIdx.count( nextCell->mainGridCellIndex() ) > 0 )
            {
                // qDebug() << "Tracer stopped. Loop detected.";
                break;
            }

            // update our current cell and direction
            curCell      = nextCell;
            curDirection = cellDirection( *curCell, grid ) * direction;

            // stop if too little flow
            if ( curDirection.length() < m_flowThreshold ) break;
        }

        // qDebug() << "Tracer length: " << streamLine->tracer().size() << "points";

        if ( streamLine->tracer().size() > 1 )
        {
            // make sure the streamline points with the flow towards the producer
            if ( direction < 0.0 ) streamLine->reverse();

            double distance = streamLine->tracer().totalDistance();

            // auto firstPos = streamLine->tracer().tracerPoints().front();
            // auto lastPos  = streamLine->tracer().tracerPoints().back();

            // double distance = lastPos.position().pointDistance( firstPos.position() );

            // qDebug() << "Tracer distance:" << distance << "meters";

            if ( distance >= m_lengthThreshold )
            {
                streamLine->generateStatistics();
                m_streamlines.push_back( streamLine );
                streamLine = nullptr;
            }
            // else
            //{
            //    qDebug() << "Skipping too short tracer";
            //}
        }
        if ( streamLine ) delete streamLine;
    }
    return;
}

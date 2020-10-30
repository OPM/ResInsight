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
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimStreamline.h"

#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include "cvfCollection.h"

#include <qdebug.h>

CAF_PDM_SOURCE_INIT( RimStreamlineInViewCollection, "StreamlineInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::RimStreamlineInViewCollection()
{
    CAF_PDM_InitScriptableObject( "Streamlines", ":/Erase.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_collectionName, "Name", "Name", "", "", "" );
    m_collectionName = "Streamlines";

    CAF_PDM_InitScriptableFieldNoDefault( &m_flowThreshold, "FlowThreshold", "Flow Threshold [m/day]", "", "", "" );
    m_flowThreshold = 0.001;

    CAF_PDM_InitScriptableFieldNoDefault( &m_resolution, "Resolution", "Resolution [days]", "", "", "" );
    m_resolution = 0.2;

    CAF_PDM_InitScriptableFieldNoDefault( &m_maxDays, "MaxDays", "Max. days ", "", "", "" );
    m_maxDays = 100;

    CAF_PDM_InitScriptableField( &m_phase,
                                 "Phase",
                                 caf::AppEnum<RiaDefines::PhaseType>( RiaDefines::PhaseType::OIL_PHASE ),
                                 "Phase",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_streamlines, "Streamlines", "Streamlines", "", "", "" );
    m_streamlines.uiCapability()->setUiTreeHidden( true );

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
        // TODO - add filter for active simulation wells here?
        m_activeTracers.push_back( streamline->tracer() );
    }
    return m_activeTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::goForIt()
{
    // reset generated streamlines
    m_streamlines().clear();

    // get the view
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );
    if ( !eclView ) return;

    // get current simulation timestep
    int timeIdx = eclView->currentTimeStep();

    // get the simulation wells
    const cvf::Collection<RigSimWellData>& simWellData = eclipseCase()->eclipseCaseData()->wellResults();

    std::vector<RigCell> seedCellsInjector;
    std::vector<RigCell> seedCellsProducer;

    std::vector<const RigGridBase*> grids;
    eclipseCase()->eclipseCaseData()->allGrids( &grids );

    // TODO - add filter to select subset of simwells here? Depends on speed - use a view filter or a calculation filter?
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
                        seedCellsProducer.push_back( cell );
                    }
                    else if ( frame.m_productionType != RigWellResultFrame::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
                    {
                        seedCellsInjector.push_back( cell );
                    }
                }
            }
        }
    }

    qDebug() << "Found " << seedCellsProducer.size() << " producing cells";
    qDebug() << "Found " << seedCellsInjector.size() << " injector cells";

    loadDataIfMissing( phase(), timeIdx );
    if ( !setupDataAccessors( m_phase(), timeIdx ) ) return;

    const int reverseDirection = -1.0;
    const int normalDirection  = 1.0;

    for ( auto cell : seedCellsInjector )
    {
        generateTracer( cell, normalDirection );
    }
    // for ( auto cell : seedCellsProducer )
    //{
    //    generateTracer( cell, reverseDirection );
    //}

    qDebug() << "Generated" << m_streamlines.childObjects().size() << " tracers";
}

//--------------------------------------------------------------------------------------------------
///
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
cvf::ref<RigResultAccessor> RimStreamlineInViewCollection::getDataAccessor( cvf::StructGridInterface::FaceType faceIdx,
                                                                            RiaDefines::PhaseType              phase,
                                                                            int                                timeIdx )
{
    QString resultname = gridResultNameFromPhase( phase, faceIdx );

    // TODO - should probably get these from somewhere
    RiaDefines::PorosityModelType porModel = RiaDefines::PorosityModelType::MATRIX_MODEL;
    int                           gridIdx  = 0;

    RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultname );
    RigCaseCellResultsData* data = m_eclipseCase->eclipseCaseData()->results( porModel );

    return RigResultAccessorFactory::createFromResultAddress( eclipseCase()->eclipseCaseData(),
                                                              gridIdx,
                                                              porModel,
                                                              timeIdx,
                                                              address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStreamlineInViewCollection::setupDataAccessors( RiaDefines::PhaseType phase, int timeIdx )
{
    // m_dataAccess.clear();

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
///
//--------------------------------------------------------------------------------------------------
RigCell* RimStreamlineInViewCollection::findNeighborCell( RigCell                            cell,
                                                          RigGridBase*                       grid,
                                                          cvf::StructGridInterface::FaceType face ) const
{
    size_t i, j, k;

    grid->ijkFromCellIndexUnguarded( cell.gridLocalCellIndex(), &i, &j, &k );

    size_t neighborIdx;
    if ( grid->cellIJKNeighbor( i, j, k, face, &neighborIdx ) )
    {
        return &grid->cell( neighborIdx );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCell*> RimStreamlineInViewCollection::findNeighborCells( RigCell* cell, RigGridBase* grid ) const
{
    std::vector<RigCell*> neighbors;

    size_t ni, nj, nk;

    grid->ijkFromCellIndexUnguarded( cell->gridLocalCellIndex(), &ni, &nj, &nk );

    for ( size_t i = ni - 1; i <= ni + 1; i++ )
        for ( size_t j = nj - 1; j <= nj + 1; j++ )
            for ( size_t k = nk - 1; k <= nk + 1; k++ )
            {
                if ( grid->isCellValid( i, j, k ) )
                {
                    RigCell cell = grid->cell( grid->cellIndexFromIJK( i, j, k ) );
                    if ( ( ni != i ) && ( nj != j ) && ( nk != k ) ) neighbors.push_back( &cell );
                }
            }

    return neighbors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::negFaceValue( RigCell                            cell,
                                                    cvf::StructGridInterface::FaceType faceIdx,
                                                    RigGridBase*                       grid ) const
{
    double retval = 0.0;

    RigCell* neighborCell = findNeighborCell( cell, grid, faceIdx );
    if ( neighborCell )
    {
        retval      = m_dataAccess[faceIdx]->cellScalar( neighborCell->gridLocalCellIndex() );
        double area = cell.faceNormalWithAreaLength( faceIdx ).length();
        if ( area != 0.0 )
            retval /= area;
        else
            retval = 0.0;
    }

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::posFaceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx ) const
{
    double retval = m_dataAccess[faceIdx]->cellScalar( cell.gridLocalCellIndex() );
    double length = cell.faceNormalWithAreaLength( faceIdx ).length();
    if ( length != 0.0 )
        retval /= length;
    else
        retval = 0.0;

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStreamlineInViewCollection::faceValue( RigCell                            cell,
                                                 cvf::StructGridInterface::FaceType faceIdx,
                                                 RigGridBase*                       grid ) const
{
    if ( faceIdx % 2 == 0 ) return posFaceValue( cell, faceIdx );

    return negFaceValue( cell, faceIdx, grid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStreamlineInViewCollection::faceValues( RigCell cell, RigGridBase* grid )
{
    double valPosI = posFaceValue( cell, cvf::StructGridInterface::FaceType::POS_I );
    double valPosJ = posFaceValue( cell, cvf::StructGridInterface::FaceType::POS_J );
    double valPosK = posFaceValue( cell, cvf::StructGridInterface::FaceType::POS_K );
    double valNegI = negFaceValue( cell, cvf::StructGridInterface::FaceType::NEG_I, grid );
    double valNegJ = negFaceValue( cell, cvf::StructGridInterface::FaceType::NEG_J, grid );
    double valNegK = negFaceValue( cell, cvf::StructGridInterface::FaceType::NEG_K, grid );

    std::vector<double> retval = {valPosI, valNegI, valPosJ, valNegJ, valPosK, valNegK};
    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStreamlineInViewCollection::cellDirection( RigCell cell, RigGridBase* grid ) const
{
    cvf::Vec3d direction( 0, 0, 0 );

    std::vector<cvf::StructGridInterface::FaceType> faces = {cvf::StructGridInterface::FaceType::POS_I,
                                                             cvf::StructGridInterface::FaceType::NEG_I,
                                                             cvf::StructGridInterface::FaceType::POS_J,
                                                             cvf::StructGridInterface::FaceType::NEG_J,
                                                             cvf::StructGridInterface::FaceType::POS_K,
                                                             cvf::StructGridInterface::FaceType::NEG_K};

    for ( auto face : faces )
    {
        cvf::Vec3d faceNorm = cell.faceNormalWithAreaLength( face );
        faceNorm.normalize();
        faceNorm *= faceValue( cell, face, grid );

        direction += faceNorm;
    }
    return direction;
}

cvf::BoundingBox RimStreamlineInViewCollection::cellBoundingBox( RigCell* cell, RigGridBase* grid ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;
    grid->cellCornerVertices( cell->gridLocalCellIndex(), hexCorners.data() );
    cvf::BoundingBox bb;
    for ( const auto& corner : hexCorners )
    {
        bb.add( corner );
    }
    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::generateTracer( RigCell cell, double direction )
{
    RigMainGrid*        grid     = eclipseCase()->eclipseCaseData()->mainGrid();
    std::vector<double> faceVals = faceValues( cell, grid );

    std::vector<cvf::StructGridInterface::FaceType> faces = {cvf::StructGridInterface::FaceType::POS_I,
                                                             cvf::StructGridInterface::FaceType::NEG_I,
                                                             cvf::StructGridInterface::FaceType::POS_J,
                                                             cvf::StructGridInterface::FaceType::NEG_J,
                                                             cvf::StructGridInterface::FaceType::POS_K,
                                                             cvf::StructGridInterface::FaceType::NEG_K};

    for ( auto faceIdx : faces )
    {
        if ( faceVals[faceIdx] <= m_flowThreshold )
        {
            qDebug() << "Skipping cell " << cell.gridLocalCellIndex() << "for face direction " << faceIdx;
            continue;
        }

        cvf::Vec3d startDirection = cell.faceNormalWithAreaLength( faceIdx );
        startDirection.normalize();
        startDirection *= faceVals[faceIdx];
        // skip vectors with inf values
        if ( startDirection.isUndefined() ) continue;

        cvf::Vec3d startPosition = cell.faceCenter( faceIdx );
        if ( startPosition.isUndefined() ) continue;

        RigCell* startCell = findNeighborCell( cell, grid, faceIdx );
        if ( startCell == nullptr ) continue;

        qDebug() << "Starting tracer from cell" << startCell->gridLocalCellIndex() << "in direction"
                 << startDirection.x() << startDirection.y() << startDirection.z();

        RigCell*   curCell = startCell;
        cvf::Vec3d curPos  = startPosition;

        RimStreamline* streamLine = new RimStreamline( "" );
        m_streamlines.childObjects().push_back( streamLine );

        int maxSteps = (int)( m_maxDays / m_resolution );
        int curStep  = 0;

        cvf::BoundingBox bb = cellBoundingBox( curCell, grid );

        while ( curStep < maxSteps )
        {
            cvf::Vec3d curDirection = cellDirection( *curCell, grid );

            while ( bb.contains( curPos ) )
            {
                streamLine->addTracerPoint( curPos, curDirection );
                curPos += curDirection * m_resolution;
                curStep++;
                if ( curStep >= maxSteps ) break;
            }

            if ( curStep >= maxSteps ) break;

            RigCell*              nextCell  = nullptr;
            std::vector<RigCell*> neighbors = findNeighborCells( curCell, grid );
            for ( auto cell : neighbors )
            {
                bb = cellBoundingBox( cell, grid );
                if ( bb.contains( curPos ) )
                {
                    nextCell = cell;
                    break;
                }
            }

            if ( nextCell == nullptr ) break;

            curCell = nextCell;

            if ( curDirection.length() < m_flowThreshold ) break;
        }
    }
    return;
}

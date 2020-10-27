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
    m_resolution = 0.1;

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

QString RimStreamlineInViewCollection::gridResultNameFromPhase( RiaDefines::PhaseType phase, int faceIdx ) const
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
        case 0:
            retval += "I+";
            break;
        case 2:
            retval += "J+";
            break;
        case 4:
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
    for ( auto cell : seedCellsProducer )
    {
        generateTracer( cell, reverseDirection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::loadDataIfMissing( RiaDefines::PhaseType phase, int timeIdx )
{
    RigCaseCellResultsData* data = m_eclipseCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    for ( int cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx += 2 )
    {
        QString                 resultname = gridResultNameFromPhase( phase, cubeFaceIdx );
        RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultname );

        data->ensureKnownResultLoadedForTimeStep( address, timeIdx );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor>
    RimStreamlineInViewCollection::getDataAccessor( int faceIdx, RiaDefines::PhaseType phase, int timeIdx )
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
    m_dataI = getDataAccessor( 0, phase, timeIdx );
    m_dataJ = getDataAccessor( 2, phase, timeIdx );
    m_dataK = getDataAccessor( 4, phase, timeIdx );

    return ( m_dataI.notNull() && m_dataJ.notNull() && m_dataK.notNull() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCell* RimStreamlineInViewCollection::findNeighborCell( RigCell                            cell,
                                                          RigGridBase*                       grid,
                                                          cvf::StructGridInterface::FaceType face )
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
std::vector<double> RimStreamlineInViewCollection::getFaceValues( RigCell cell, RigGridBase* grid )
{
    const size_t cellIdx = cell.gridLocalCellIndex();

    double valPosI = m_dataI->cellScalar( cellIdx );
    valPosI /= cell.faceNormalWithAreaLength( cvf::StructGridInterface::FaceType::POS_I ).length();

    double valPosJ = m_dataJ->cellScalar( cellIdx );
    valPosJ /= cell.faceNormalWithAreaLength( cvf::StructGridInterface::FaceType::POS_J ).length();

    double valPosK = m_dataK->cellScalar( cellIdx );
    valPosK /= cell.faceNormalWithAreaLength( cvf::StructGridInterface::FaceType::POS_K ).length();

    double valNegI = 0.0;
    double valNegJ = 0.0;
    double valNegK = 0.0;
    double area    = 1.0;

    RigCell* neighborCell = findNeighborCell( cell, grid, cvf::StructGridInterface::FaceType::NEG_I );
    if ( neighborCell )
    {
        valNegI = m_dataI->cellScalar( neighborCell->gridLocalCellIndex() );
        area    = neighborCell->faceNormalWithAreaLength( cvf::StructGridInterface::FaceType::POS_I ).length();
        if ( area != 0.0 )
            valNegI /= area;
        else
            valNegI = 0.0;
    }

    neighborCell = findNeighborCell( cell, grid, cvf::StructGridInterface::FaceType::NEG_J );
    if ( neighborCell )
    {
        valNegJ = m_dataI->cellScalar( neighborCell->gridLocalCellIndex() );
        area    = neighborCell->faceNormalWithAreaLength( cvf::StructGridInterface::FaceType::POS_J ).length();
        if ( area != 0.0 )
            valNegJ /= area;
        else
            valNegJ = 0.0;
    }

    neighborCell = findNeighborCell( cell, grid, cvf::StructGridInterface::FaceType::NEG_K );
    if ( neighborCell )
    {
        valNegK = m_dataI->cellScalar( neighborCell->gridLocalCellIndex() );
        area    = neighborCell->faceNormalWithAreaLength( cvf::StructGridInterface::FaceType::POS_K ).length();
        if ( area != 0.0 )
            valNegK /= area;
        else
            valNegK = 0.0;
    }

    std::vector<double> retval = {valPosI, valNegI, valPosJ, valNegJ, valPosK, valNegK};
    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::generateTracer( RigCell cell, double direction )
{
    RigGridBase*        grid       = eclipseCase()->eclipseCaseData()->grid( 0 );
    std::vector<double> faceValues = getFaceValues( cell, grid );

    for ( int faceIdx = 0; faceIdx < 6; faceIdx++ )
    {
        if ( faceValues[faceIdx] <= m_flowThreshold )
        {
            qDebug() << "Skipping cell " << cell.gridLocalCellIndex() << "for face direction " << faceIdx;
            continue;
        }

        qDebug() << "Starting tracer " << cell.gridLocalCellIndex() << faceValues;
    }
    return;
}

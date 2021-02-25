/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimStreamlineDataAccess.h"

#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineDataAccess::RimStreamlineDataAccess()
    : m_data( nullptr )
    , m_grid( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineDataAccess::~RimStreamlineDataAccess()
{
}

//--------------------------------------------------------------------------------------------------
/// Set up data accessors to access the selected flroil/gas/wat data for all faces
//--------------------------------------------------------------------------------------------------
bool RimStreamlineDataAccess::setupDataAccess( RigMainGrid*                     grid,
                                               RigEclipseCaseData*              data,
                                               std::list<RiaDefines::PhaseType> phases,
                                               int                              timeIdx )
{
    m_grid = grid;
    m_data = data;

    m_dataAccess.clear();

    for ( auto phase : phases )
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
/// Return a data accessor for the given phase and face and time step
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RimStreamlineDataAccess::getDataAccessor( cvf::StructGridInterface::FaceType faceIdx,
                                                                      RiaDefines::PhaseType              phase,
                                                                      int                                timeIdx )
{
    RiaDefines::PorosityModelType porModel = RiaDefines::PorosityModelType::MATRIX_MODEL;

    RigCaseCellResultsData* data = m_data->results( porModel );

    QString resultname = gridResultNameFromPhase( phase, faceIdx );
    int     gridIdx    = 0;

    RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultname );

    // Make sure we have the data we need loaded for the given phase and time step index
    data->ensureKnownResultLoadedForTimeStep( address, timeIdx );

    return RigResultAccessorFactory::createFromResultAddress( m_data, gridIdx, porModel, timeIdx, address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStreamlineDataAccess::gridResultNameFromPhase( RiaDefines::PhaseType              phase,
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
/// Return the face scalar value for the given cell and NEG_? face, by using the neighbor cell
//--------------------------------------------------------------------------------------------------
double RimStreamlineDataAccess::negFaceValueDividedByArea( RigCell                            cell,
                                                           cvf::StructGridInterface::FaceType faceIdx,
                                                           RiaDefines::PhaseType              phase ) const
{
    double retval = 0.0;

    RigCell neighborCell = cell.neighborCell( faceIdx );
    if ( neighborCell.isInvalid() ) return retval;

    std::vector<cvf::ref<RigResultAccessor>> access = m_dataAccess.at( phase );
    retval                                          = access[faceIdx]->cellScalar( neighborCell.mainGridCellIndex() );
    double area                                     = cell.faceNormalWithAreaLength( faceIdx ).length();
    if ( area != 0.0 )
        retval /= area;
    else
        retval = 0.0;

    if ( std::isinf( retval ) ) retval = 0.0;

    return retval;
}

//--------------------------------------------------------------------------------------------------
/// Return the face scalar value for the given cell and POS_? face
//--------------------------------------------------------------------------------------------------
double RimStreamlineDataAccess::posFaceValueDividedByArea( RigCell                            cell,
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

    if ( std::isinf( retval ) ) retval = 0.0;

    return retval;
}

//--------------------------------------------------------------------------------------------------
/// Return the face scalar value for the given cell and face
//--------------------------------------------------------------------------------------------------
double RimStreamlineDataAccess::faceValueDividedByArea( RigCell                            cell,
                                                        cvf::StructGridInterface::FaceType faceIdx,
                                                        RiaDefines::PhaseType              phase ) const
{
    if ( faceIdx % 2 == 0 ) return posFaceValueDividedByArea( cell, faceIdx, phase );

    // NEG_? face values must be read from the neighbor cells
    return negFaceValueDividedByArea( cell, faceIdx, phase );
}

//--------------------------------------------------------------------------------------------------
/// Return the face scalar value for the given cell and face, by combining flow for all specified phases
//--------------------------------------------------------------------------------------------------
double RimStreamlineDataAccess::combinedFaceValueByArea( RigCell                            cell,
                                                         cvf::StructGridInterface::FaceType faceIdx,
                                                         std::list<RiaDefines::PhaseType>   phases,
                                                         RiaDefines::PhaseType&             outDominantPhase ) const
{
    double retValue  = 0.0;
    outDominantPhase = phases.front();

    double max = 0.0;

    for ( auto phase : phases )
    {
        double tmp = 0.0;
        if ( faceIdx % 2 == 0 )
            tmp = posFaceValueDividedByArea( cell, faceIdx, phase );
        else
            tmp = negFaceValueDividedByArea( cell, faceIdx, phase );
        if ( abs( tmp ) > max )
        {
            outDominantPhase = phase;
            max              = abs( tmp );
        }
        retValue += tmp;
    }

    return retValue;
}

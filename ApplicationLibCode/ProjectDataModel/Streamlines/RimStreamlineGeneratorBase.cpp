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

#include "RimStreamlineGeneratorBase.h"

#include "RigCell.h"
#include "RigMainGrid.h"

#include "RimStreamline.h"
#include "RimStreamlineDataAccess.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineGeneratorBase::RimStreamlineGeneratorBase( std::set<size_t>& wellCells )
    : m_maxDays( 10000 )
    , m_flowThreshold( 0.0 )
    , m_resolution( 10.0 )
    , m_wellCells( wellCells )
    , m_dataAccess( nullptr )
    , m_maxPoints( 1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineGeneratorBase::~RimStreamlineGeneratorBase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineGeneratorBase::setLimits( double flowThreshold, int maxDays, double resolutionInDays )
{
    m_flowThreshold = flowThreshold;
    m_maxDays       = maxDays;
    m_resolution    = resolutionInDays;
    m_maxPoints     = maxDays / resolutionInDays;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineGeneratorBase::initGenerator( RimStreamlineDataAccess* dataAccess, std::list<RiaDefines::PhaseType> phases )
{
    m_dataAccess = dataAccess;

    m_phases.clear();
    for ( auto phase : phases )
        m_phases.push_back( phase );
}

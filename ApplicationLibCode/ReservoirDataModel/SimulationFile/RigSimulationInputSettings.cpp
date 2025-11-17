/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RigSimulationInputSettings.h"

#include "opm/input/eclipse/Deck/DeckRecord.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSimulationInputSettings::RigSimulationInputSettings()
    : m_min( caf::VecIjk0::ZERO )
    , m_max( caf::VecIjk0::ZERO )
    , m_refinement( 1, 1, 1 )
    , m_boundaryCondition( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RigSimulationInputSettings::min() const
{
    return m_min;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RigSimulationInputSettings::max() const
{
    return m_max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setMin( const caf::VecIjk0& min )
{
    m_min = min;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setMax( const caf::VecIjk0& max )
{
    m_max = max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RigSimulationInputSettings::refinement() const
{
    return m_refinement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setRefinement( const cvf::Vec3st& refinement )
{
    m_refinement = refinement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::DeckRecord> RigSimulationInputSettings::bcpropKeywords() const
{
    return m_bcpropKeywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setBcpropKeywords( const std::vector<Opm::DeckRecord>& keywords )
{
    m_bcpropKeywords = keywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigSimulationInputSettings::boundaryCondition() const
{
    return m_boundaryCondition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setBoundaryCondition( int value )
{
    m_boundaryCondition = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigSimulationInputSettings::inputDeckFileName() const
{
    return m_inputDeckFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setInputDeckFileName( const QString& fileName )
{
    m_inputDeckFileName = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigSimulationInputSettings::outputDeckFileName() const
{
    return m_outputDeckFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setOutputDeckFileName( const QString& fileName )
{
    m_outputDeckFileName = fileName;
}

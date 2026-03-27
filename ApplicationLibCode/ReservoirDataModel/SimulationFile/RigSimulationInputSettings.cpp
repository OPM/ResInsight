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
    , m_boundaryCondition( RiaModelExportDefines::BoundaryCondition::OPERNUM_OPERATER )
    , m_porvMultiplier( 1.0e6 )
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
std::expected<void, QString> RigSimulationInputSettings::validateBox() const
{
    if ( m_min.i() > m_max.i() || m_min.j() > m_max.j() || m_min.k() > m_max.k() )
    {
        return std::unexpected(
            "Invalid cell box: Min should be less than or equal to Max in all dimensions. Please check export settings." );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNonUniformRefinement RigSimulationInputSettings::effectiveRefinement() const
{
    if ( hasNonUniformRefinement() ) return m_nonUniformRefinement;

    auto sectorSize =
        cvf::Vec3st( m_max.x() - m_min.x() + 1, m_max.y() - m_min.y() + 1, m_max.z() - m_min.z() + 1 );

    if ( m_refinement.x() > 1 || m_refinement.y() > 1 || m_refinement.z() > 1 )
        return RigNonUniformRefinement::fromUniform( m_refinement, sectorSize );

    return RigNonUniformRefinement( sectorSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setEffectiveRefinement( const RigNonUniformRefinement& refinement )
{
    m_nonUniformRefinement = refinement;
    m_refinement           = cvf::Vec3st( 1, 1, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimulationInputSettings::hasRefinement() const
{
    return ( m_refinement.x() > 1 || m_refinement.y() > 1 || m_refinement.z() > 1 ) || hasNonUniformRefinement();
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
const RigNonUniformRefinement& RigSimulationInputSettings::nonUniformRefinement() const
{
    return m_nonUniformRefinement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setNonUniformRefinement( const RigNonUniformRefinement& refinement )
{
    m_nonUniformRefinement = refinement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimulationInputSettings::hasNonUniformRefinement() const
{
    return m_nonUniformRefinement.hasNonUniformRefinement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setKeywordsToRemove( const std::vector<std::string>& keywords )
{
    m_keywordsToRemove = keywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::string>& RigSimulationInputSettings::keywordsToRemove() const
{
    return m_keywordsToRemove;
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
RiaModelExportDefines::BoundaryCondition RigSimulationInputSettings::boundaryCondition() const
{
    return m_boundaryCondition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setBoundaryCondition( RiaModelExportDefines::BoundaryCondition value )
{
    m_boundaryCondition = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigSimulationInputSettings::porvMultiplier() const
{
    return m_porvMultiplier;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setPorvMultiplier( double value )
{
    m_porvMultiplier = value;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigModelPaddingSettings& RigSimulationInputSettings::paddingSettings() const
{
    return m_paddingSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimulationInputSettings::setPaddingSettings( const RigModelPaddingSettings& settings )
{
    m_paddingSettings = settings;
}

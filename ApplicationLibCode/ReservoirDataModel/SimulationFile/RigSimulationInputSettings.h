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

#pragma once

#include "RiaModelExportDefines.h"

#include "cafAppEnum.h"
#include "cafVecIjk.h"
#include "cvfVector3.h"

#include <QString>
#include <vector>

namespace Opm
{
class DeckRecord;
}

//==================================================================================================
///
/// Settings for exporting simulation input files (sector models)
///
//==================================================================================================
class RigSimulationInputSettings
{
public:
    using BoundaryConditionEnum = caf::AppEnum<RiaModelExportDefines::BoundaryCondition>;

    RigSimulationInputSettings();

    // Sector bounds (0-based grid coordinates)
    caf::VecIjk0 min() const;
    caf::VecIjk0 max() const;
    void         setMin( const caf::VecIjk0& min );
    void         setMax( const caf::VecIjk0& max );

    // Grid refinement
    cvf::Vec3st refinement() const;
    void        setRefinement( const cvf::Vec3st& refinement );

    // Boundary conditions
    std::vector<Opm::DeckRecord> bcpropKeywords() const;
    void                         setBcpropKeywords( const std::vector<Opm::DeckRecord>& keywords );

    RiaModelExportDefines::BoundaryCondition boundaryCondition() const;
    void                                     setBoundaryCondition( RiaModelExportDefines::BoundaryCondition value );

    double porvMultiplier() const;
    void   setPorvMultiplier( double value );

    // File paths
    QString inputDeckFileName() const;
    void    setInputDeckFileName( const QString& fileName );

    QString outputDeckFileName() const;
    void    setOutputDeckFileName( const QString& fileName );

private:
    caf::VecIjk0                 m_min;
    caf::VecIjk0                 m_max;
    cvf::Vec3st                  m_refinement;
    std::vector<Opm::DeckRecord> m_bcpropKeywords;
    BoundaryConditionEnum        m_boundaryCondition;
    double                       m_porvMultiplier;
    QString                      m_inputDeckFileName;
    QString                      m_outputDeckFileName;
};

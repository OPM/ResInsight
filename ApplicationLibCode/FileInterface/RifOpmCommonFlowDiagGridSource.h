/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "opm/utility/ECLGraphGridSource.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Opm::EclIO
{
class EGrid;
}

//==================================================================================================
// Grid topology for flow diagnostics read directly from EGRID using opm-common. Reads only grid
// headers, ACTNUM, LGR host cells and NNC arrays, never cell geometry, so it works the same for
// full and active-cells-only grid import.
//==================================================================================================
class RifOpmCommonFlowDiagGridSource : public Opm::ECLGraphGridSource
{
public:
    explicit RifOpmCommonFlowDiagGridSource( const std::string& gridFileName );
    ~RifOpmCommonFlowDiagGridSource() override;

    int                           numGrids() const override;
    std::string                   gridName( int gridID ) const override;
    std::array<int, 3>            dimensions( int gridID ) const override;
    std::vector<int>              activeToGlobal( int gridID ) const override;
    std::vector<bool>             activeIsSubdivided( int gridID ) const override;
    std::vector<Opm::ECLGraphNNC> nncs() const override;

private:
    int parentGridId( int lgrIndex ) const;

private:
    // Main grid followed by LGRs, in EGRID order
    std::vector<std::unique_ptr<Opm::EclIO::EGrid>> m_grids;
    std::vector<std::string>                        m_lgrNames;
    std::vector<std::string>                        m_lgrParents;
};

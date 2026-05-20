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

#include "cvfArray.h"
#include "cvfObject.h"

#include <cstddef>
#include <map>

class RimCellFilter;
class RimEclipseCase;

//==================================================================================================
///
/// Adapter that evaluates a single RimCellFilter against global Eclipse cell indices on behalf of
/// the COMPDAT and MSW perforation exporters. Per-grid visibility masks are built lazily and
/// cached, so repeated lookups inside a per-cell loop are a single byte test.
///
//==================================================================================================
class RicPerforationCellFilterEvaluator
{
public:
    RicPerforationCellFilterEvaluator( RimCellFilter* filter, const RimEclipseCase* eclipseCase );

    // True if a non-null, active and enabled filter is attached. When false, includesGlobalCell()
    // always returns true and callers can skip the per-cell test entirely.
    bool isEnabled() const;

    bool includesGlobalCell( size_t globalCellIndex ) const;

    // Number of cells rejected by includesGlobalCell() since construction. Used by callers to
    // report when a filter fully suppresses a perforation interval.
    size_t rejectedCellCount() const;

private:
    const cvf::UByteArray* maskForGrid( int gridIndex ) const;

    RimCellFilter*        m_filter;
    const RimEclipseCase* m_eclipseCase;
    bool                  m_enabled;

    mutable std::map<int, cvf::ref<cvf::UByteArray>> m_visibilityByGridIndex;
    mutable size_t                                   m_rejectedCellCount;
};

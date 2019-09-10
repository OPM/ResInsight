/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotCellFilter.h"

#include "cafPdmChildArrayField.h"

class RimCase;

//==================================================================================================
///
//==================================================================================================
class RimPlotCellFilterCollection : public RimPlotCellFilter
{
    CAF_PDM_HEADER_INIT;

public:
    RimPlotCellFilterCollection();

    void   addCellFilter( RimPlotCellFilter* cellFilter );
    size_t cellFilterCount() const;

    void computeCellVisibilityFromFilter( size_t timeStepIndex, cvf::UByteArray* cellVisibility );

    void setCase( RimCase* gridCase );

protected:
    void updateCellVisibilityFromFilter( size_t timeStepIndex, cvf::UByteArray* cellVisibility ) override;

private:
    caf::PdmChildArrayField<RimPlotCellFilter*> m_cellFilters;
};

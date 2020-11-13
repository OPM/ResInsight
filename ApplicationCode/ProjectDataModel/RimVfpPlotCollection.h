/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimVfpPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimVfpPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpPlotCollection();
    ~RimVfpPlotCollection() override;

    void                     addPlot( RimVfpPlot* newPlot );
    std::vector<RimVfpPlot*> plots();
    void                     deleteAllChildObjects();

private:
    caf::PdmChildArrayField<RimVfpPlot*> m_vfpPlots;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimAbstractPlotCollection.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimEnsembleFractureStatisticsPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimEnsembleFractureStatisticsPlotCollection : public caf::PdmObject, public RimPlotCollection

{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleFractureStatisticsPlotCollection();

    void addEnsembleFractureStatisticsPlot( RimEnsembleFractureStatisticsPlot* newPlot );

    std::vector<RimEnsembleFractureStatisticsPlot*> ensembleFractureStatisticsPlots() const;

    size_t plotCount() const override;
    void   loadDataAndUpdateAllPlots() override;

    void deleteAllPlots() override;

private:
    caf::PdmChildArrayField<RimEnsembleFractureStatisticsPlot*> m_ensembleFractureStatisticsPlots;
};

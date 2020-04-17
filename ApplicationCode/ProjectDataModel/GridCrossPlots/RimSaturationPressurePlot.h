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

#include "RiaPorosityModel.h"

#include "RimGridCrossPlot.h"

class RimEclipseResultCase;
class RimPlotCellPropertyFilter;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimSaturationPressurePlot : public RimGridCrossPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimSaturationPressurePlot();

    void assignCaseAndEquilibriumRegion( RiaDefines::PorosityModelType porosityModel,
                                         RimEclipseResultCase*         eclipseResultCase,
                                         int                           zeroBasedEquilRegionIndex,
                                         int                           timeStep );

    static void fixPointersAfterCopy( RimSaturationPressurePlot* source, RimSaturationPressurePlot* copy );

protected:
    void    initAfterRead() override;
    QString xAxisParameterString() const override;

private:
    RimPlotCellPropertyFilter* createEquilibriumRegionPropertyFilter( RimEclipseResultCase* eclipseResultCase,
                                                                      int                   zeroBasedEquilRegionIndex );

    RimPlotCellPropertyFilter*
        createDepthPropertyFilter( RimEclipseResultCase* eclipseResultCase, double minDepth, double maxDepth );
};

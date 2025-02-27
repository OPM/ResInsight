/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuPlotUpdater.h"

class RiuPvtPlotPanel;
class RimEclipseResultDefinition;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuPvtPlotUpdater : public RiuPlotUpdater
{
public:
    RiuPvtPlotUpdater( RiuPvtPlotPanel* targetPlotPanel );

protected:
    void     clearPlot() override;
    QWidget* plotPanel() override;

    bool queryDataAndUpdatePlot( const RimEclipseResultDefinition* eclipseResultDef,
                                 size_t                            timeStepIndex,
                                 size_t                            gridIndex,
                                 size_t                            gridLocalCellIndex ) override;

private:
    QPointer<RiuPvtPlotPanel> m_targetPlotPanel;
};

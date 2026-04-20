/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimTabbedTextProvider.h"

#include "cafPdmPointer.h"

class RimGridCrossPlot;

//==================================================================================================
/// Tabbed text provider for a RimGridCrossPlot. Each tab corresponds to a data set in the plot.
//==================================================================================================
class RimGridCrossPlotTextProvider : public RimTabbedTextProvider
{
public:
    explicit RimGridCrossPlotTextProvider( RimGridCrossPlot* crossPlot );

    bool    isValid() const override;
    QString description() const override;
    QString tabTitle( int tabIndex ) const override;
    QString tabText( int tabIndex ) const override;
    int     tabCount() const override;

private:
    caf::PdmPointer<RimGridCrossPlot> m_crossPlot;
};

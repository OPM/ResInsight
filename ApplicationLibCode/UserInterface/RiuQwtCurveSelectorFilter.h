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

#include <QObject>

#include <functional>
#include <utility>
#include <vector>

class QPoint;
class QwtPlot;

namespace caf
{
class PdmUiItem;
}

//==================================================================================================
/// Event filter that selects a realization in the plot main window tree when the user left-clicks
/// near a curve on a Qwt plot canvas.
///
/// The caller supplies a finder callback: given a canvas pixel position, return the PdmUiItem
/// (typically a RimSummaryCase*) closest to that position, or nullptr if nothing is close enough.
/// When the callback returns a non-null item it is selected via RiuDockWidgetTools.
//==================================================================================================
class RiuQwtCurveSelectorFilter : public QObject
{
public:
    using ItemFinder = std::function<const caf::PdmUiItem*( const QPoint& )>;

    RiuQwtCurveSelectorFilter( QwtPlot* plot, ItemFinder finder );

    // Returns the index of the point in `points` closest to `canvasPos` using axis-normalised
    // squared distance, or -1 if none falls within the 3% threshold.
    static int closestPointIndex( QwtPlot* plot, const QPoint& canvasPos, const std::vector<std::pair<double, double>>& points );

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override;

private:
    ItemFinder m_finder;
};

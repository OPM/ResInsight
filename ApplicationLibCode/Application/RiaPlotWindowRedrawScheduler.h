/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaDefines.h"

#include "cafScheduler.h"

#include <QPointer>

#include <map>
#include <set>

class RiuMultiPlotPage;
class RiuMultiPlotBook;
class RiuPlotWidget;

class RiaPlotWindowRedrawScheduler : public caf::Scheduler
{
    Q_OBJECT

public:
    static RiaPlotWindowRedrawScheduler* instance();

    void scheduleMultiPlotBookUpdate( RiuMultiPlotBook*                   plotWindow,
                                      RiaDefines::MultiPlotPageUpdateType updateType = RiaDefines::MultiPlotPageUpdateType::ALL );
    void scheduleMultiPlotPageUpdate( RiuMultiPlotPage*                   plotWindow,
                                      RiaDefines::MultiPlotPageUpdateType updateType = RiaDefines::MultiPlotPageUpdateType::ALL );
    void schedulePlotWidgetReplot( RiuPlotWidget* plotWidget );
    void clearAllScheduledUpdates();

    void performScheduledUpdates() override;

private:
    std::map<QPointer<RiuMultiPlotPage>, RiaDefines::MultiPlotPageUpdateType> m_plotPagesToUpdate;
    std::map<QPointer<RiuMultiPlotBook>, RiaDefines::MultiPlotPageUpdateType> m_plotBooksToUpdate;

    std::set<QPointer<RiuPlotWidget>> m_plotWidgetsToReplot;
};

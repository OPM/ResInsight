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

#include "cafPdmPointer.h"

#include "RiaDefines.h"

#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QTimer>

#include <map>
#include <vector>

class RiuMultiPlotPage;
class RiuMultiPlotBook;
class RiuPlotWidget;

class RiaPlotWindowRedrawScheduler : public QObject
{
    Q_OBJECT

public:
    static RiaPlotWindowRedrawScheduler* instance();

    void scheduleMultiPlotWindowUpdate( RiuMultiPlotBook* plotWindow );
    void scheduleMultiPlotPageUpdate(
        RiuMultiPlotPage*                   plotWindow,
        RiaDefines::MultiPlotPageUpdateType updateType = RiaDefines::MultiPlotPageUpdateType::ALL );
    void schedulePlotWidgetReplot( RiuPlotWidget* plotWidget );
    void clearAllScheduledUpdates();
    void performScheduledUpdatesAndReplots();

private slots:
    void slotUpdateAndReplotScheduledItemsWhenReady();

private:
    RiaPlotWindowRedrawScheduler()           = default;
    ~RiaPlotWindowRedrawScheduler() override = default;

    void startTimer( int msecs );

private:
    std::map<QPointer<RiuMultiPlotPage>, RiaDefines::MultiPlotPageUpdateType> m_plotPagesToUpdate;

    std::vector<QPointer<RiuPlotWidget>>    m_plotWidgetsToReplot;
    std::vector<QPointer<RiuMultiPlotBook>> m_plotWindowsToUpdate;

    QScopedPointer<QTimer> m_plotWindowUpdateTimer;
};

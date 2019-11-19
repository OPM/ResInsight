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

#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QTimer>

#include <vector>

class RiuMultiPlotWindow;
class RiuQwtPlotWidget;

class RiaPlotWindowRedrawScheduler : public QObject
{
    Q_OBJECT

public:
    static RiaPlotWindowRedrawScheduler* instance();
    void                                 schedulePlotWindowUpdate( RiuMultiPlotWindow* plotWindow );
    void                                 schedulePlotWidgetReplot( RiuQwtPlotWidget* plotWidget );
    void                                 clearAllScheduledUpdates();
    void                                 performScheduledUpdatesAndReplots();

private slots:
    void slotUpdateAndReplotScheduledItemsWhenReady();

private:
    void startTimer( int msecs );

private:
    std::vector<QPointer<RiuQwtPlotWidget>>   m_plotWidgetsToReplot;
    std::vector<QPointer<RiuMultiPlotWindow>> m_plotWindowsToUpdate;
    QScopedPointer<QTimer>                    m_plotWindowUpdateTimer;
};

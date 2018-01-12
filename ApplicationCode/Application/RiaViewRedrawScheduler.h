/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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
#include <vector>
#include "cafPdmPointer.h"

class QTimer;
class Rim3dView;


class RiaViewRedrawScheduler : public QObject
{
    Q_OBJECT;
public:
    static RiaViewRedrawScheduler* instance();
    void                scheduleDisplayModelUpdateAndRedraw(Rim3dView* resViewToUpdate);
    void                clearViewsScheduledForUpdate();

public slots:
    void                slotUpdateScheduledDisplayModels();

private:
    RiaViewRedrawScheduler() : m_resViewUpdateTimer(nullptr)  {}
    ~RiaViewRedrawScheduler();

    RiaViewRedrawScheduler(const RiaViewRedrawScheduler& o) = delete;
    void operator=(const RiaViewRedrawScheduler& o) = delete;

    std::vector<caf::PdmPointer<Rim3dView> > m_resViewsToUpdate;
    QTimer*                                  m_resViewUpdateTimer;
};



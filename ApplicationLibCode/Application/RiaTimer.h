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

#include <memory>

#include "cafSignal.h"

class QTimerEvent;
class QBasicTimer;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaTimer : public QObject, public caf::SignalEmitter
{
    Q_OBJECT

public:
    RiaTimer();
    ~RiaTimer();

    void start( int intervalMs = 1000 );
    void stop();

    caf::Signal<> tick;

protected:
    void timerEvent( QTimerEvent* event ) override;

private:
    std::unique_ptr<QBasicTimer> m_timer;
};

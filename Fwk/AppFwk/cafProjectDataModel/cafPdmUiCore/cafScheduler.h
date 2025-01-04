/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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
#include <QScopedPointer>
#include <QTimer>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// This class is used to block the scheduled task when a blocking operation is in ongoing. Currently this is used when
/// a progress dialog is visible. See ProgressInfoStatic::start()
//--------------------------------------------------------------------------------------------------
class SchedulerCallable
{
public:
    void registerCallable( const std::function<bool()>& func ) { m_callable = func; }

    static SchedulerCallable* instance()
    {
        static SchedulerCallable instance;
        return &instance;
    }

    bool isScheduledTaskBlocked()
    {
        if ( m_callable )
        {
            return m_callable();
        }
        else
        {
            return false;
        }
    }

private:
    std::function<bool()> m_callable;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class Scheduler : public QObject
{
    Q_OBJECT

public:
    Scheduler();
    ~Scheduler() override;

    virtual void performScheduledUpdates() = 0;

    void blockUpdate( bool blockUpdate );

protected:
    void startTimer( int msecs );

private slots:
    void slotUpdateScheduledItemsWhenReady();

private:
    QScopedPointer<QTimer> m_updateTimer;
    bool                   m_blockUpdate;
};
} // namespace caf

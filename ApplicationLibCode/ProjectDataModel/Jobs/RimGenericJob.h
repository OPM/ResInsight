/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <map>

class RimProcess;

//==================================================================================================
///
///
//==================================================================================================
class RimGenericJob : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum JobState
    {
        Idle, // default, nothing is happening
        Queued, // waiting for available resources to run
        Running, // job is running
        Completed, // job completed without errors
        Failed // job completed with errors
    };

    RimGenericJob();
    ~RimGenericJob() override;

    virtual bool              execute()                                                         = 0;
    virtual bool              stop()                                                            = 0;
    virtual double            percentageDone() const                                            = 0;
    virtual const QStringList jobLog() const                                                    = 0;
    virtual bool              matchesKeyValue( const QString& key, const QString& value ) const = 0;
    virtual void              processLogOutput( const QString& logLine ) {};

    virtual bool setFinished( bool runOk ) = 0;
    virtual void setStarted()              = 0;

    bool     isRunning() const;
    JobState state() const;

protected:
    JobState m_jobState;
};

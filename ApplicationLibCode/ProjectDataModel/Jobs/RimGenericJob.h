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
    RimGenericJob();
    ~RimGenericJob() override;

    bool execute();
    bool setFinished( bool runOk );

    bool isRunning() const;
    bool stop();

    double            percentageDone() const;
    const QStringList jobLog() const;

    virtual bool matchesKeyValue( const QString& key, const QString& value ) const;

    virtual void processLogOutput( const QString& logLine ) = 0;

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    virtual QStringList                command()     = 0;
    virtual std::map<QString, QString> environment() = 0;
    virtual QString                    workingDirectory() const;
    virtual bool                       onPrepare()                         = 0;
    virtual bool                       onRun()                             = 0;
    virtual void                       onCompleted( bool success )         = 0;
    virtual void                       onProgress( double percentageDone ) = 0;

protected:
    double m_percentageDone;
    int    m_warningsDetected;
    int    m_errorsDetected;

private:
    bool                        m_lastRunFailed;
    bool                        m_isRunning;
    caf::PdmPointer<RimProcess> m_process;
};

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

#include "RimGenericJob.h"

#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>

#include <map>

class RimProcess;

//==================================================================================================
///
///
//==================================================================================================
class RimSingleJob : public RimGenericJob
{
    CAF_PDM_HEADER_INIT;

public:
    RimSingleJob();
    ~RimSingleJob() override;

    bool              execute() override;
    bool              stop() override;
    double            percentageDone() const override;
    const QStringList jobLog() const override;
    bool              matchesKeyValue( const QString& key, const QString& value ) const override;

    bool setFinished( bool runOk ) override;
    void setStarted() override;

protected:
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
    caf::PdmPointer<RimProcess> m_process;
};

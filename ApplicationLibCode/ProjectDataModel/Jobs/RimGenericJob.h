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

#include <QString>
#include <QStringList>

#include <map>

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

    double percentageDone() const;

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    virtual QString                    title()       = 0;
    virtual QStringList                command()     = 0;
    virtual std::map<QString, QString> environment() = 0;
    virtual QString                    workingDirectory() const;
    virtual bool                       onPrepare()                         = 0;
    virtual bool                       onRun()                             = 0;
    virtual void                       onCompleted( bool success )         = 0;
    virtual void                       onProgress( double percentageDone ) = 0;

private:
    double m_percentageDone;
    bool   m_lastRunFailed;
    bool   m_isRunning;
};

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

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    virtual QString     title()   = 0;
    virtual QStringList command() = 0;
    virtual QString     workingDirectory() const;
    virtual bool        onPrepare()                 = 0;
    virtual void        onCompleted( bool success ) = 0;
};

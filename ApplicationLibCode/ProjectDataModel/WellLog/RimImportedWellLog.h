/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimWellLog.h"

#include "cafPdmField.h"
#include "cvfObject.h"

#include <QString>

class RigImportedWellLogData;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RimImportedWellLog : public RimWellLog
{
    CAF_PDM_HEADER_INIT;

public:
    RimImportedWellLog();

    void    setName( const QString& name );
    QString name() const override;

    QString wellName() const override;

    RigWellLogData* wellLogData() override;
    void            setWellLogData( RigImportedWellLogData* wellLogData );

    std::vector<std::pair<double, double>>
        findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString = nullptr ) override;

private:
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    cvf::ref<RigImportedWellLogData> m_wellLogData;
    caf::PdmField<QString>           m_name;
};
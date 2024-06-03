/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigOsduWellLogData.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <QDateTime>

class RimWellLogChannel;
class RimWellPath;

class QString;

//==================================================================================================
///
///
//==================================================================================================
class RimOsduWellLog : public RimWellLog
{
    CAF_PDM_HEADER_INIT;

public:
    RimOsduWellLog();
    ~RimOsduWellLog() override;

    void    setName( const QString& name );
    QString name() const override;

    QString wellName() const override;

    RigOsduWellLogData* wellLogData() override;
    void                setWellLogData( RigOsduWellLogData* wellLogData );

    void    setWellLogId( const QString& wellLogId );
    QString wellLogId() const;

    bool hasFlowData() const;

    std::vector<std::pair<double, double>>
        findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString = nullptr ) override;

private:
    caf::PdmFieldHandle* userDescriptionField() override;

    static bool isDateValid( const QDateTime dateTime );

private:
    cvf::ref<RigOsduWellLogData> m_wellLogData;
    caf::PdmField<QString>       m_wellName;
    caf::PdmField<QString>       m_name;
    caf::PdmField<QString>       m_wellLogId;
};

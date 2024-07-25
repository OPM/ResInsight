/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogFile.h"

#include "RigWellLogLasFile.h"
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
class RimWellLogLasFile : public RimWellLogFile
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogLasFile();

    static RimWellLogLasFile* readWellLogFile( const QString& logFilePath, QString* errorMessage );

    QString name() const override;

    bool readFile( QString* errorMessage ) override;

    QString wellName() const override;

    RigWellLogLasFile* wellLogData() override;

    bool hasFlowData() const;

    enum WellFlowCondition
    {
        WELL_FLOW_COND_RESERVOIR,
        WELL_FLOW_COND_STANDARD
    };

    RimWellLogLasFile::WellFlowCondition wellFlowRateCondition() const;

    std::vector<std::pair<double, double>>
        findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString = nullptr ) override;

private:
    void setupBeforeSave() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

    static bool isDateValid( const QDateTime dateTime );

private:
    cvf::ref<RigWellLogLasFile>                    m_wellLogDataFile;
    caf::PdmField<QString>                         m_wellName;
    caf::PdmField<QString>                         m_name;
    bool                                           m_lasFileHasValidDate;
    caf::PdmField<caf::AppEnum<WellFlowCondition>> m_wellFlowCondition;

    caf::PdmField<QString> m_invalidDateMessage;
};

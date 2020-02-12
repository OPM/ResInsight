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

#include "RigWellLogFile.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <QDateTime>

class RimWellLogFileChannel;
class RimWellPath;

class QString;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogFile : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    const static QDateTime DEFAULT_DATE_TIME;

public:
    RimWellLogFile();
    ~RimWellLogFile() override;

    static RimWellLogFile* readWellLogFile( const QString& logFilePath, QString* errorMessage );

    void    setFileName( const QString& fileName );
    QString fileName() const { return m_fileName().path(); }
    QString name() const { return m_name; }

    bool readFile( QString* errorMessage );

    QString   wellName() const;
    QDateTime date() const;

    RigWellLogFile*                     wellLogFileData() { return m_wellLogDataFile.p(); }
    std::vector<RimWellLogFileChannel*> wellLogChannels() const;

    bool hasFlowData() const;

    enum WellFlowCondition
    {
        WELL_FLOW_COND_RESERVOIR,
        WELL_FLOW_COND_STANDARD
    };

    RimWellLogFile::WellFlowCondition wellFlowRateCondition() const { return m_wellFlowCondition(); }

    void updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath );

    static std::vector<std::pair<double, double>> findMdAndChannelValuesForWellPath( const RimWellPath* wellPath,
                                                                                     const QString&     channelName,
                                                                                     QString* unitString = nullptr );

private:
    void setupBeforeSave() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* userDescriptionField() override { return &m_name; }

    static bool isDateValid( const QDateTime dateTime );

    caf::PdmChildArrayField<RimWellLogFileChannel*> m_wellLogChannelNames;

private:
    cvf::ref<RigWellLogFile>                       m_wellLogDataFile;
    caf::PdmField<QString>                         m_wellName;
    caf::PdmField<caf::FilePath>                   m_fileName;
    caf::PdmField<QString>                         m_name;
    caf::PdmField<QDateTime>                       m_date;
    bool                                           m_lasFileHasValidDate;
    caf::PdmField<caf::AppEnum<WellFlowCondition>> m_wellFlowCondition;

    caf::PdmField<QString> m_invalidDateMessage;
};

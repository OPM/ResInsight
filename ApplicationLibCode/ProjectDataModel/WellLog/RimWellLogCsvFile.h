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

#include "RigWellLogCsvFile.h"
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
class RimWellLogCsvFile : public RimWellLogFile
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogCsvFile();
    ~RimWellLogCsvFile() override;

    // static RimWellLogCsvFile* readWellLogFile( const QString& logFilePath, QString* errorMessage );

    QString name() const { return m_name; }

    bool readFile( QString* errorMessage );

    QString wellName() const;

    RigWellLogCsvFile* wellLogFileData();

    std::vector<std::pair<double, double>>
        findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString = nullptr ) override;

private:
    // void setupBeforeSave() override;
    // void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    // void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    // void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* userDescriptionField() override { return &m_name; }

private:
    cvf::ref<RigWellLogCsvFile> m_wellLogDataFile;
    caf::PdmField<QString>      m_wellName;
    caf::PdmField<QString>      m_name;
};

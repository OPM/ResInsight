/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QDateTime>
#include <QString>

class RimWellLogFileChannel;
class RimWellPath;

class RigWellLogFile;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogFile : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogFile();
    ~RimWellLogFile() override;

    virtual void                                setFileName( const QString& fileName );
    virtual QString                             fileName() const;
    virtual std::vector<RimWellLogFileChannel*> wellLogChannels() const;

    virtual QString         wellName() const                  = 0;
    virtual QString         name() const                      = 0;
    virtual bool            readFile( QString* errorMessage ) = 0;
    virtual RigWellLogFile* wellLogFileData()                 = 0;

    virtual QDateTime date() const;

    virtual std::vector<std::pair<double, double>>
        findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString = nullptr ) = 0;

    const static QDateTime DEFAULT_DATE_TIME;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmChildArrayField<RimWellLogFileChannel*> m_wellLogChannelNames;
    caf::PdmField<caf::FilePath>                    m_fileName;
    caf::PdmField<QDateTime>                        m_date;
};

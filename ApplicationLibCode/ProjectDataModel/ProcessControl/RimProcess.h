/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QProcess>
#include <QString>
#include <QStringList>

class RimProcess : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimProcess();
    ~RimProcess() override;

    void setDescription( QString desc );
    void setCommand( QString cmdStr );
    void addParameter( QString paramStr );
    void setParameters( QStringList parameterList );
    void setID( int id );

    QString commandLine() const;

    QString     command() const;
    QStringList parameters() const;
    int         ID() const;

    QString execute();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    bool    needsCommandInterpreter() const;
    QString handleSpaces( QString argument ) const;

    caf::PdmField<QString> m_command;
    QStringList            m_arguments;
    caf::PdmField<QString> m_description;
    caf::PdmField<int>     m_id;
};

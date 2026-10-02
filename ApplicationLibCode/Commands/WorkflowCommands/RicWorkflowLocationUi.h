/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <optional>

//==================================================================================================
/// Name and folder of a workflow, asked for by New, Duplicate as Editable and Save As.
/// The folder follows the name until the user edits the folder.
//==================================================================================================
class RicWorkflowLocationUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    struct Location
    {
        QString name;
        QString folder;
    };

    RicWorkflowLocationUi();

    void    setName( const QString& name );
    QString name() const;
    QString folder() const;

    static std::optional<Location> askForLocation( const QString& title, const QString& defaultName );
    static QString                 defaultFolder( const QString& name );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<QString>       m_name;
    caf::PdmField<caf::FilePath> m_folder;
    bool                         m_folderEditedByUser = false;
};

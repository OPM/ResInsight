/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimWorkflowTaskInput.h"

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimWorkflow : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflow();

    void    setWorkflowDirectory( const QString& directory );
    QString workflowDirectory() const;

    bool    loadFromDirectory( QString* errorMessage = nullptr );
    QString writeInputYaml( const QString& path ) const;

    std::vector<RimWorkflowTaskInput*> taskInputs() const;

    void runWorkflow();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<QString>                          m_name;
    caf::PdmField<QString>                          m_description;
    caf::PdmField<caf::FilePath>                    m_workflowDirectory;
    caf::PdmField<QString>                          m_loadError;
    caf::PdmField<bool>                             m_runButton;
    caf::PdmChildArrayField<RimWorkflowTaskInput*>  m_taskInputs;
};

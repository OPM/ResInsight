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

#include "RimWorkflow.h"

#include "cafPdmChildField.h"
#include "cafPdmObjectCollection.h"

#include <QString>

class RimWorkflowInstalledCollection;

//==================================================================================================
/// Workflow folders in the discovery folder, new unsaved workflows, and the workflows registered
/// by installed packages (in the "Installed" sub folder)
//==================================================================================================
class RimWorkflowCollection : public caf::PdmObjectCollection<RimWorkflow>
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflowCollection();
    ~RimWorkflowCollection() override;

    void rescanWorkflows();
    bool hasUnsavedChanges() const;

    std::vector<RimWorkflow*> allWorkflows() const;
    RimWorkflow*              findWorkflowByDirectory( const QString& directory ) const;

    static QString discoveryDirectory();

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmChildField<RimWorkflowInstalledCollection*> m_installed;
};

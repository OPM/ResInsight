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

#include <QString>
#include <QVariantMap>

class RimWorkflow;
class RiuWorkflowEditorWidget;

//==================================================================================================
/// Shared code of the workflow commands and the workflow editor toolbar
//==================================================================================================
namespace RicWorkflowFeatureTools
{
// The workflow of the selected tree item, else the workflow shown in the editor
RimWorkflow*             selectedWorkflow();
RimWorkflow*             displayedWorkflow();
RiuWorkflowEditorWidget* workflowEditor();

// The editor when it shows an editable, unlocked workflow; graph commands act on it
RiuWorkflowEditorWidget* editableWorkflowEditor();

RimWorkflow* newWorkflow();
RimWorkflow* duplicateAsEditable( RimWorkflow* workflow );
bool         saveWorkflow( RimWorkflow* workflow );
bool         saveWorkflowAs( RimWorkflow* workflow );
bool         rescanWorkflows();

void showError( const QString& title, const QString& message );
} // namespace RicWorkflowFeatureTools

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

#include "RicWorkflowFeatureTools.h"

#include "RicWorkflowLocationUi.h"

#include "RimProject.h"
#include "Workflow/RimWorkflow.h"
#include "Workflow/RimWorkflowCollection.h"
#include "Workflow/RimWorkflowDefinitionTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"
#include "RiuWorkflowEditorWidget.h"
#include "RiuWorkflowTaskPalette.h"

#include "cafSelectionManagerTools.h"

#include <QMessageBox>

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowCollection* workflowCollection()
{
    RimProject* project = RimProject::current();
    return project ? project->workflowCollection() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void addAndSelect( RimWorkflow* workflow )
{
    RimWorkflowCollection* collection = workflowCollection();
    collection->addItem( workflow );
    collection->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( workflow );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow* RicWorkflowFeatureTools::selectedWorkflow()
{
    if ( auto* workflow = caf::firstAncestorOfTypeFromSelectedObject<RimWorkflow>() ) return workflow;
    return displayedWorkflow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow* RicWorkflowFeatureTools::displayedWorkflow()
{
    auto* window = RiuMainWindow::instance();
    return window ? window->displayedWorkflow() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowEditorWidget* RicWorkflowFeatureTools::workflowEditor()
{
    auto* window = RiuMainWindow::instance();
    return window ? window->workflowEditor() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowEditorWidget* RicWorkflowFeatureTools::editableWorkflowEditor()
{
    RiuWorkflowEditorWidget* editor = workflowEditor();
    if ( !editor || !editor->workflow() ) return nullptr;
    if ( !editor->workflow()->isEditable() || editor->workflow()->isLocked() ) return nullptr;
    return editor;
}

//--------------------------------------------------------------------------------------------------
/// An empty workflow, saved to its folder on the first Save
//--------------------------------------------------------------------------------------------------
RimWorkflow* RicWorkflowFeatureTools::newWorkflow()
{
    if ( !workflowCollection() ) return nullptr;

    const auto location = RicWorkflowLocationUi::askForLocation( "New Workflow", "new_workflow" );
    if ( !location ) return nullptr;

    auto* workflow = new RimWorkflow;
    workflow->setUnsavedDefinition( RimWorkflowDefinitionTools::createEmpty( location->name ), {}, location->folder );
    addAndSelect( workflow );
    return workflow;
}

//--------------------------------------------------------------------------------------------------
/// An editable copy with the values of the selected job. Python modules next to the source YAML
/// are copied on the first Save.
//--------------------------------------------------------------------------------------------------
RimWorkflow* RicWorkflowFeatureTools::duplicateAsEditable( RimWorkflow* workflow )
{
    if ( !workflow || !workflowCollection() ) return nullptr;
    if ( !workflow->loadError().isEmpty() )
    {
        showError( "Duplicate Workflow", workflow->loadError() );
        return nullptr;
    }
    if ( !workflow->definition().editable )
    {
        showError( "Duplicate Workflow",
                   QString( "An editable copy is not possible:\n%1" ).arg( workflow->definition().readOnlyReasons.join( "\n" ) ) );
        return nullptr;
    }

    const auto location = RicWorkflowLocationUi::askForLocation( "Duplicate as Editable", workflow->name() + "_copy" );
    if ( !location ) return nullptr;

    RimWorkflowDefinition copy   = RimWorkflowDefinitionTools::editableCopy( workflow->definition(), location->name );
    const QJsonObject     values = workflow->inputValuesForSave();
    for ( auto it = values.begin(); it != values.end(); ++it )
        copy.inputs[it.key()] = it.value();

    auto* duplicate = new RimWorkflow;
    duplicate->setUnsavedDefinition( copy, workflow->sourceDirectory(), location->folder );
    addAndSelect( duplicate );
    if ( !duplicate->isEditable() )
        showError( "Duplicate Workflow", QString( "The copy cannot be edited: %1" ).arg( duplicate->readOnlyReason() ) );
    return duplicate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWorkflowFeatureTools::saveWorkflow( RimWorkflow* workflow )
{
    if ( !workflow || !workflow->isEditable() ) return false;
    if ( workflow->source() == RimWorkflow::Source::Unsaved && workflow->workflowDirectory().isEmpty() ) return saveWorkflowAs( workflow );

    const QStringList warnings = workflow->saveInPlaceWarnings();
    if ( !warnings.isEmpty() )
    {
        const auto answer = QMessageBox::question( Riu3DMainWindowTools::mainWindowWidget(),
                                                   "Save Workflow",
                                                   warnings.join( "\n\n" ) + "\n\nSave anyway?",
                                                   QMessageBox::Save | QMessageBox::Cancel,
                                                   QMessageBox::Cancel );
        if ( answer != QMessageBox::Save ) return false;
    }

    auto result = workflow->save();
    if ( !result )
    {
        showError( "Save Workflow", result.error() );
        return false;
    }
    workflow->updateConnectedEditors();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWorkflowFeatureTools::saveWorkflowAs( RimWorkflow* workflow )
{
    if ( !workflow || !workflow->isEditable() ) return false;

    const auto location = RicWorkflowLocationUi::askForLocation( "Save Workflow As", workflow->name() );
    if ( !location ) return false;

    if ( location->name != workflow->name() )
    {
        auto renamed = workflow->applyEdit( RimWorkflowDefinitionTools::renameWorkflow( workflow->definition(), location->name ) );
        if ( !renamed )
        {
            showError( "Save Workflow As", renamed.error() );
            return false;
        }
    }

    auto result = workflow->saveAs( location->folder );
    if ( !result )
    {
        showError( "Save Workflow As", result.error() );
        return false;
    }
    workflow->updateConnectedEditors();
    return true;
}

//--------------------------------------------------------------------------------------------------
/// Reload the task catalog and the workflow folders. Unsaved edits of saved workflows are lost.
//--------------------------------------------------------------------------------------------------
bool RicWorkflowFeatureTools::rescanWorkflows()
{
    RimWorkflowCollection* collection = workflowCollection();
    if ( !collection ) return false;

    QStringList dirtyNames;
    for ( RimWorkflow* workflow : collection->items() )
    {
        if ( workflow->source() != RimWorkflow::Source::Unsaved && workflow->isEditable() && workflow->isDirty() )
            dirtyNames << workflow->name();
    }
    if ( !dirtyNames.isEmpty() )
    {
        const auto answer =
            QMessageBox::question( Riu3DMainWindowTools::mainWindowWidget(),
                                   "Rescan Workflows",
                                   QString( "Rescanning discards the unsaved changes of: %1\n\nContinue?" ).arg( dirtyNames.join( ", " ) ),
                                   QMessageBox::Discard | QMessageBox::Cancel,
                                   QMessageBox::Cancel );
        if ( answer != QMessageBox::Discard ) return false;
    }

    collection->rescanWorkflows();

    if ( auto* window = RiuMainWindow::instance() ) window->workflowDefinitionChanged( nullptr );
    if ( auto* editor = workflowEditor() ) editor->palette()->reload();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWorkflowFeatureTools::showError( const QString& title, const QString& message )
{
    QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(), title, message );
}

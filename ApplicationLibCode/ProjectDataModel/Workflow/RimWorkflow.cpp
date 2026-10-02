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
#include "RimWorkflow.h"

#include "RimWorkflowDefinitionTools.h"
#include "RimWorkflowDescribeTools.h"
#include "RimWorkflowHelperProcess.h"
#include "RimWorkflowJob.h"
#include "RimWorkflowValidationTools.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RiuMainWindow.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiOrdering.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTemporaryDir>

CAF_PDM_SOURCE_INIT( RimWorkflow, "Workflow" );

namespace
{
constexpr size_t maxUndoSteps = 100;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow::RimWorkflow()
{
    CAF_PDM_InitObject( "Workflow", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    m_name.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_workflowDirectory, "WorkflowDirectory", "Directory" );
    m_workflowDirectory.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_registeredId, "RegisteredId", "Registered Id" );
    m_registeredId.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_editable, "Editable", false, "Editable" );
    m_editable.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_readOnlyReason, "ReadOnlyReason", "Read-only Because" );
    m_readOnlyReason.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_loadError, "LoadError", "Load Error" );
    m_loadError.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_jobs, "Jobs", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow::~RimWorkflow() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::setWorkflowDirectory( const QString& directory )
{
    m_workflowDirectory = directory;
    m_sourceDirectory   = directory;
    m_name              = QFileInfo( QDir( directory ).absolutePath() ).fileName();
    updateUiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::workflowDirectory() const
{
    return m_workflowDirectory().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflow::graph() const
{
    return m_graph;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::loadError() const
{
    return m_loadError();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowJob*> RimWorkflow::jobs() const
{
    std::vector<RimWorkflowJob*> result;
    result.reserve( m_jobs.size() );
    for ( RimWorkflowJob* job : m_jobs.childrenByType() )
    {
        if ( job ) result.push_back( job );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::addJob( RimWorkflowJob* job )
{
    if ( !job ) return;
    m_jobs.push_back( job );
    job->syncTaskInputs( m_graph.value( "tasks" ).toArray() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::syncJobs( const QMap<QString, QString>& renames )
{
    const QJsonArray tasks = m_graph.value( "tasks" ).toArray();
    for ( RimWorkflowJob* job : jobs() )
        job->syncTaskInputs( tasks, renames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewWorkflowJobFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicSaveWorkflowFeature";
    menuBuilder << "RicSaveWorkflowAsFeature";
    menuBuilder << "RicDuplicateWorkflowAsEditableFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_name.uiCapability()->setUiReadOnly( !isEditable() || isLocked() );

    uiOrdering.add( &m_name );
    uiOrdering.add( &m_description );
    if ( m_source == Source::Registered )
        uiOrdering.add( &m_registeredId );
    else if ( !workflowDirectory().isEmpty() )
        uiOrdering.add( &m_workflowDirectory );
    uiOrdering.add( &m_editable );
    if ( !m_readOnlyReason().isEmpty() ) uiOrdering.add( &m_readOnlyReason );
    if ( !m_loadError().isEmpty() ) uiOrdering.add( &m_loadError );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField != &m_name ) return;

    auto result = applyEdit( RimWorkflowDefinitionTools::renameWorkflow( m_definition, m_name() ) );
    if ( !result )
    {
        RiaLogging::warning( result.error().toStdString() );
        m_name = m_definition.name;
        updateUiName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::setLoadError( const QString& message, QString* errorMessage )
{
    m_loadError = message;
    RiaLogging::warning( QString( "Workflow '%1': %2" ).arg( m_name(), message ).toStdString() );
    if ( errorMessage ) *errorMessage = message;
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Load a YAML folder through `taskmaestro_helper load --describe`
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::loadFromDirectory( QString* errorMessage )
{
    m_graph     = {};
    m_loadError = "";

    const QString directory = workflowDirectory();
    if ( directory.isEmpty() ) return false;
    if ( !QDir( directory ).exists( "workflow.yaml" ) ) return setLoadError( "Missing workflow.yaml", errorMessage );

    auto output = RimWorkflowHelperProcess::runSync( { "load", QDir( directory ).absolutePath(), "--describe" } );
    if ( !output ) return setLoadError( output.error(), errorMessage );

    auto definition = RimWorkflowDefinitionTools::fromJson( output.value() );
    if ( !definition ) return setLoadError( definition.error(), errorMessage );

    m_registeredId = "";
    setDefinition( definition.value(), Source::YamlDirectory );
    recordModificationTime();

    const auto validation =
        RimWorkflowValidationTools::issuesFromDescribe( m_definition.describe, m_definition.describeError, m_definition.nodeNames() );
    setValidationResult( m_revision, validation.issues, validation.describeSucceeded );
    if ( !validation.describeSucceeded )
    {
        m_loadError = RimWorkflowHelperProcess::errorMessage( m_definition.describeError );
        RiaLogging::warning( QString( "Workflow '%1' is not valid: %2" ).arg( m_name(), m_loadError() ).toStdString() );
    }

    RiaLogging::info(
        QString( "Loaded workflow '%1' (%2 tasks) from %3" ).arg( m_name() ).arg( m_definition.nodes.size() ).arg( directory ).toStdString() );
    return true;
}

//--------------------------------------------------------------------------------------------------
/// A workflow registered by an installed package, as listed by `taskmaestro_helper catalog`.
/// Registered workflows are read-only; Duplicate as Editable makes an editable copy.
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::loadFromRegistered( const QJsonObject& catalogEntry, QString* errorMessage )
{
    m_graph        = {};
    m_loadError    = "";
    m_registeredId = catalogEntry.value( "id" ).toString();
    m_name         = catalogEntry.value( "name" ).toString( m_registeredId() );
    m_sourceDirectory.clear();

    RimWorkflowDefinition definition;
    if ( catalogEntry.value( "definition" ).isObject() )
    {
        auto parsed = RimWorkflowDefinitionTools::fromJson( catalogEntry.value( "definition" ).toObject() );
        if ( !parsed ) return setLoadError( parsed.error(), errorMessage );
        definition = parsed.value();
    }
    else
    {
        definition.name     = m_name();
        definition.editable = false;
    }

    // `definition.editable` tells whether an editable copy is possible; the installed workflow
    // itself is always read-only (see isEditable())
    for ( const QJsonValue& reason : catalogEntry.value( "readonly_reasons" ).toArray() )
    {
        if ( !definition.readOnlyReasons.contains( reason.toString() ) ) definition.readOnlyReasons.append( reason.toString() );
    }
    if ( !definition.readOnlyReasons.isEmpty() ) definition.editable = false;
    if ( definition.editable ) definition.readOnlyReasons.append( "Installed workflows are read-only; duplicate to edit" );
    if ( definition.describe.isEmpty() ) definition.describe = catalogEntry.value( "describe" ).toObject();

    setDefinition( definition, Source::Registered );
    if ( m_graph.value( "tasks" ).toArray().isEmpty() ) return setLoadError( "The workflow could not be described", errorMessage );
    return true;
}

//--------------------------------------------------------------------------------------------------
/// A new workflow, or an editable copy. `sourceDirectory` holds Python modules used by the tasks,
/// and `targetDirectory` is the folder the first Save writes to.
//--------------------------------------------------------------------------------------------------
void RimWorkflow::setUnsavedDefinition( const RimWorkflowDefinition& definition, const QString& sourceDirectory, const QString& targetDirectory )
{
    m_workflowDirectory = targetDirectory;
    m_registeredId      = "";
    m_loadError         = "";
    m_sourceDirectory   = sourceDirectory;

    RimWorkflowDefinition editable = definition;
    if ( definition.editable ) editable.readOnlyReasons.clear();
    editable.readOnlyReasons.append( RimWorkflowDefinitionTools::structuralReadOnlyReasons( definition ) );
    editable.readOnlyReasons.removeDuplicates();
    editable.editable = editable.readOnlyReasons.isEmpty();
    setDefinition( editable, Source::Unsaved );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::setDefinition( const RimWorkflowDefinition& definition, Source source )
{
    m_source          = source;
    m_definition      = definition;
    m_savedDefinition = definition;
    m_undoStack.clear();
    m_redoStack.clear();
    m_validationIssues.clear();
    m_validationRevision = -1;
    ++m_revision;

    if ( !definition.name.isEmpty() ) m_name = definition.name;
    m_description    = definition.headerComment;
    m_editable       = isEditable();
    m_readOnlyReason = definition.readOnlyReasons.join( "\n" );

    rebuildGraph();
    ensureDefaultJob();
    syncJobs();
    updateUiName();
    notifyDefinitionChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow::Source RimWorkflow::source() const
{
    return m_source;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::registeredId() const
{
    return m_registeredId();
}

//--------------------------------------------------------------------------------------------------
/// The folder with the Python modules of the workflow (the YAML folder or the folder it was copied from)
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::sourceDirectory() const
{
    return m_sourceDirectory;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWorkflowDefinition& RimWorkflow::definition() const
{
    return m_definition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::isEditable() const
{
    return m_definition.editable && m_source != Source::Registered;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::readOnlyReason() const
{
    return m_readOnlyReason();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::isDirty() const
{
    return m_source == Source::Unsaved || m_definition != m_savedDefinition;
}

//--------------------------------------------------------------------------------------------------
/// The workflow cannot be edited while one of its jobs is running
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::isLocked() const
{
    for ( RimWorkflowJob* job : jobs() )
    {
        if ( job->isRunning() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWorkflow::revision() const
{
    return m_revision;
}

//--------------------------------------------------------------------------------------------------
/// Apply the result of an edit operation from RimWorkflowDefinitionTools. `renames` maps old task
/// names to new names, so that input values follow renamed tasks.
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWorkflow::applyEdit( const std::expected<RimWorkflowDefinition, QString>& result,
                                                     const QMap<QString, QString>&                        renames )
{
    if ( !result ) return std::unexpected( result.error() );
    if ( !isEditable() )
        return std::unexpected( m_readOnlyReason().isEmpty() ? QString( "The workflow is read-only" ) : m_readOnlyReason() );
    if ( isLocked() ) return std::unexpected( "The workflow cannot be edited while a job is running" );
    if ( result.value() == m_definition ) return {};

    m_undoStack.push_back( { m_definition, renames } );
    if ( m_undoStack.size() > maxUndoSteps ) m_undoStack.erase( m_undoStack.begin() );
    m_redoStack.clear();

    m_definition = result.value();
    definitionChanged( renames );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::canUndo() const
{
    return isEditable() && !isLocked() && !m_undoStack.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::canRedo() const
{
    return isEditable() && !isLocked() && !m_redoStack.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::undo()
{
    if ( !canUndo() ) return;

    UndoEntry entry = m_undoStack.back();
    m_undoStack.pop_back();
    m_redoStack.push_back( { m_definition, entry.renames } );
    m_definition = entry.definition;
    definitionChanged( invertedRenames( entry.renames ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::redo()
{
    if ( !canRedo() ) return;

    UndoEntry entry = m_redoStack.back();
    m_redoStack.pop_back();
    m_undoStack.push_back( { m_definition, entry.renames } );
    m_definition = entry.definition;
    definitionChanged( entry.renames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::definitionChanged( const QMap<QString, QString>& renames )
{
    ++m_revision;
    m_name        = m_definition.name;
    m_description = m_definition.headerComment;

    // Keep the issues that have not been checked again until the next validation
    rebuildGraph();
    syncJobs( renames );
    updateUiName();
    notifyDefinitionChanged();
}

//--------------------------------------------------------------------------------------------------
/// Let the workflow editor show the new state
//--------------------------------------------------------------------------------------------------
void RimWorkflow::notifyDefinitionChanged()
{
    if ( auto* window = RiuMainWindow::instance() ) window->workflowDefinitionChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMap<QString, QString> RimWorkflow::invertedRenames( const QMap<QString, QString>& renames )
{
    QMap<QString, QString> inverted;
    for ( auto it = renames.begin(); it != renames.end(); ++it )
        inverted.insert( it.value(), it.key() );
    return inverted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::rebuildGraph()
{
    if ( isEditable() )
    {
        std::vector<RimWorkflowIssue> describeIssues;
        if ( isValidationCurrent() ) describeIssues = m_validationIssues;
        m_graph = RimWorkflowDefinitionTools::graphFromDefinition( m_definition, true, describeIssues );
    }
    else if ( !m_definition.describe.isEmpty() )
    {
        m_graph             = RimWorkflowDescribeTools::graphFromDescribe( m_definition.describe );
        m_graph["editable"] = false;
    }
    else
    {
        m_graph = RimWorkflowDefinitionTools::graphFromDefinition( m_definition, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::updateUiName()
{
    setUiName( isDirty() && isEditable() ? m_name() + " *" : m_name() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::ensureDefaultJob()
{
    if ( !m_jobs.empty() ) return;

    auto* job = new RimWorkflowJob;
    job->setJobName( "Default" );
    m_jobs.push_back( job );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::setValidationResult( int revision, const std::vector<RimWorkflowIssue>& issues, bool describeSucceeded )
{
    if ( revision != m_revision ) return;

    m_validationIssues   = issues;
    m_validationRevision = revision;
    m_describeSucceeded  = describeSucceeded;
    rebuildGraph();
    notifyDefinitionChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWorkflowIssue> RimWorkflow::validationIssues() const
{
    return m_validationIssues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::isValidationCurrent() const
{
    return m_validationRevision == m_revision;
}

//--------------------------------------------------------------------------------------------------
/// True when the current revision has errors, from the static checks or from the last describe
//--------------------------------------------------------------------------------------------------
bool RimWorkflow::lastValidationFailed() const
{
    if ( isValidationCurrent() && !m_describeSucceeded ) return true;
    if ( !isEditable() ) return false;
    for ( const auto& issue : RimWorkflowDefinitionTools::validate( m_definition ) )
    {
        if ( issue.severity == RimWorkflowIssue::Severity::Error ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// input.yaml content written on save: the literal values of the first job, falling back to the
/// values loaded from the file for tasks that have no inputs in the job
//--------------------------------------------------------------------------------------------------
QJsonObject RimWorkflow::inputValuesForSave() const
{
    QJsonObject inputs  = m_definition.inputs;
    const auto  allJobs = jobs();
    if ( allJobs.empty() ) return inputs;

    const QJsonObject literal = allJobs.front()->literalInputValues();
    for ( const auto& node : m_definition.nodes )
    {
        if ( literal.contains( node.name ) ) inputs[node.name] = literal.value( node.name );
    }
    return inputs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<QJsonObject, QString> RimWorkflow::saveToDirectory( const QString& directory, bool backup )
{
    QStringList args{ "save", "--out-dir", directory, "--describe" };
    if ( backup ) args << "--backup";

    auto output =
        RimWorkflowHelperProcess::runSync( args, definitionJsonForSave(), RimWorkflowHelperProcess::defaultTimeoutMs, m_sourceDirectory );
    if ( !output ) return output;

    m_definition.inputs        = inputValuesForSave();
    m_definition.describe      = output->value( "describe" ).toObject();
    m_definition.describeError = output->value( "describe_error" ).toObject();
    m_definition.source        = QJsonObject{ { "workflow_yaml", output->value( "workflow_yaml" ) },
                                              { "input_yaml", output->value( "input_yaml" ) },
                                              { "registered_id", QJsonValue::Null } };
    m_savedDefinition          = m_definition;
    m_source                   = Source::YamlDirectory;
    m_loadedModificationTime   = QFileInfo( QDir( directory ).absoluteFilePath( "workflow.yaml" ) ).lastModified();

    const auto validation = RimWorkflowValidationTools::issuesFromHelperResult( output.value(), m_definition.nodeNames() );
    setValidationResult( m_revision, validation.issues, validation.describeSucceeded );
    updateUiName();
    return output;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflow::recordModificationTime()
{
    m_loadedModificationTime = QFileInfo( QDir( workflowDirectory() ).absoluteFilePath( "workflow.yaml" ) ).lastModified();
}

//--------------------------------------------------------------------------------------------------
/// Reasons to think twice before overwriting the workflow folder: workflow.yaml was changed by
/// someone else since it was loaded, or the folder is a link into a git checkout
//--------------------------------------------------------------------------------------------------
QStringList RimWorkflow::saveInPlaceWarnings() const
{
    QStringList warnings;
    if ( m_source != Source::YamlDirectory || workflowDirectory().isEmpty() ) return warnings;

    const QFileInfo yamlFile( QDir( workflowDirectory() ).absoluteFilePath( "workflow.yaml" ) );
    if ( m_loadedModificationTime.isValid() && yamlFile.exists() && yamlFile.lastModified() != m_loadedModificationTime )
    {
        warnings << "workflow.yaml has been changed on disk since it was loaded. Saving overwrites those changes.";
    }

    const QFileInfo directoryInfo( workflowDirectory() );
    if ( directoryInfo.isSymLink() || yamlFile.isSymLink() )
    {
        QDir dir( yamlFile.canonicalPath() );
        do
        {
            if ( dir.exists( ".git" ) )
            {
                warnings
                    << QString( "The workflow folder links into the git checkout '%1'. Saving changes files there." ).arg( dir.absolutePath() );
                break;
            }
        } while ( dir.cdUp() );
    }
    return warnings;
}

//--------------------------------------------------------------------------------------------------
/// Write workflow.yaml and input.yaml to the workflow folder. A backup is made the first time.
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWorkflow::save()
{
    if ( !isEditable() ) return std::unexpected( "The workflow is read-only" );
    if ( workflowDirectory().isEmpty() ) return std::unexpected( "The workflow has no folder; use Save As" );
    if ( m_source == Source::Unsaved ) return saveAs( workflowDirectory() );

    auto output = saveToDirectory( workflowDirectory(), true );
    if ( !output ) return std::unexpected( output.error() );

    RiaLogging::info( QString( "Saved workflow '%1' to %2" ).arg( m_name(), workflowDirectory() ).toStdString() );
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Save to a new folder. Python modules next to the original workflow.yaml are copied along.
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWorkflow::saveAs( const QString& directory )
{
    if ( !isEditable() ) return std::unexpected( "The workflow is read-only" );
    if ( directory.isEmpty() ) return std::unexpected( "No folder given" );

    QDir target( directory );
    if ( !target.exists() && !QDir().mkpath( directory ) )
        return std::unexpected( QString( "Could not create the folder '%1'" ).arg( directory ) );

    if ( !m_sourceDirectory.isEmpty() && QDir( m_sourceDirectory ).absolutePath() != target.absolutePath() )
    {
        const QDir source( m_sourceDirectory );
        for ( const QString& fileName : source.entryList( { "*.py" }, QDir::Files ) )
        {
            if ( !target.exists( fileName ) ) QFile::copy( source.absoluteFilePath( fileName ), target.absoluteFilePath( fileName ) );
        }
    }

    auto output = saveToDirectory( target.absolutePath(), false );
    if ( !output ) return std::unexpected( output.error() );

    m_workflowDirectory = target.absolutePath();
    m_sourceDirectory   = target.absolutePath();
    m_registeredId      = "";
    updateUiName();
    notifyDefinitionChanged();

    RiaLogging::info( QString( "Saved workflow '%1' to %2" ).arg( m_name(), target.absolutePath() ).toStdString() );
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Write the current revision as workflow.yaml to a private temporary folder, used to run and
/// validate unsaved edits
//--------------------------------------------------------------------------------------------------
std::expected<QString, QString> RimWorkflow::writeEditDirectorySync()
{
    auto directory = editDirectory();
    if ( !directory ) return directory;
    if ( m_editDirectoryRevision == m_revision ) return directory;

    auto output = RimWorkflowHelperProcess::runSync( { "save", "--out-dir", directory.value(), "--workflow-only" },
                                                     definitionJsonForSave(),
                                                     RimWorkflowHelperProcess::defaultTimeoutMs,
                                                     m_sourceDirectory );
    if ( !output ) return std::unexpected( output.error() );

    m_editDirectoryRevision = m_revision;
    return directory;
}

//--------------------------------------------------------------------------------------------------
/// The temporary folder that holds the YAML of unsaved edits, for validation and for running
//--------------------------------------------------------------------------------------------------
std::expected<QString, QString> RimWorkflow::editDirectory()
{
    if ( !m_editDirectory )
    {
        m_editDirectory = std::make_unique<QTemporaryDir>( QDir::tempPath() + "/resinsight_workflow_edit_XXXXXX" );
        if ( !m_editDirectory->isValid() )
        {
            m_editDirectory.reset();
            return std::unexpected( "Could not create a temporary folder for the workflow" );
        }
    }
    return m_editDirectory->path();
}

//--------------------------------------------------------------------------------------------------
/// Called when the edit folder has been written for `revision` by someone else (live validation)
//--------------------------------------------------------------------------------------------------
void RimWorkflow::setEditDirectoryRevision( int revision )
{
    if ( revision == m_revision ) m_editDirectoryRevision = revision;
}

//--------------------------------------------------------------------------------------------------
/// The definition as sent to `taskmaestro_helper save`
//--------------------------------------------------------------------------------------------------
QByteArray RimWorkflow::definitionJsonForSave() const
{
    RimWorkflowDefinition toSave = m_definition;
    toSave.inputs                = inputValuesForSave();
    return QJsonDocument( RimWorkflowDefinitionTools::toJson( toSave ) ).toJson( QJsonDocument::Compact );
}

//--------------------------------------------------------------------------------------------------
/// The arguments of `taskmaestro_helper run` for the current state of the workflow
//--------------------------------------------------------------------------------------------------
std::expected<RimWorkflow::RunSource, QString> RimWorkflow::prepareRunSource()
{
    if ( m_source == Source::Registered )
    {
        if ( m_registeredId().isEmpty() ) return std::unexpected( "The workflow has no registered id" );
        return RunSource{ .baseArgs = { "run", "--registered", m_registeredId() } };
    }

    if ( m_source == Source::YamlDirectory && !isDirty() )
    {
        if ( workflowDirectory().isEmpty() ) return std::unexpected( "The workflow has no folder" );
        return RunSource{ .baseArgs = { "run", workflowDirectory() }, .extraPythonPath = workflowDirectory() };
    }

    auto editDirectory = writeEditDirectorySync();
    if ( !editDirectory ) return std::unexpected( editDirectory.error() );
    return RunSource{ .baseArgs = { "run", editDirectory.value() }, .extraPythonPath = m_sourceDirectory };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflow::findPythonExecutable()
{
    QStringList candidates;
    if ( auto* prefs = RiaPreferences::current() )
    {
        QString configured = prefs->pythonExecutable();
        if ( !configured.isEmpty() && configured != "python" ) candidates << configured;
    }
    candidates << "python3" << "python";

    for ( const QString& candidate : candidates )
    {
        if ( candidate.contains( '/' ) || candidate.contains( '\\' ) )
        {
            if ( QFileInfo( candidate ).isExecutable() ) return candidate;
        }
        else if ( !QStandardPaths::findExecutable( candidate ).isEmpty() )
        {
            return candidate;
        }
    }
    return {};
}

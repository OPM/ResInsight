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

#include "RiuWorkflowEditorWidget.h"

#include "RiuWorkflowTaskPalette.h"
#include "RiuWorkflowValidationRunner.h"

#include "WorkflowCommands/RicWorkflowFeatureTools.h"

#include "Workflow/RimWorkflow.h"
#include "Workflow/RimWorkflowDefinition.h"
#include "Workflow/RimWorkflowDefinitionTools.h"
#include "Workflow/RimWorkflowFieldBinding.h"
#include "Workflow/RimWorkflowJob.h"
#include "Workflow/RimWorkflowPortCompatibility.h"
#include "Workflow/RimWorkflowTaskCatalog.h"
#include "Workflow/RimWorkflowTaskInput.h"
#include "Workflow/RimWorkflowValidationTools.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <map>

namespace
{
//--------------------------------------------------------------------------------------------------
/// The action of a command feature carrying user data for the current menu
//--------------------------------------------------------------------------------------------------
QAction* featureAction( const QString& commandId, const QString& text, const QVariantMap& userData )
{
    auto* feature = caf::CmdFeatureManager::instance()->getCommandFeature( commandId.toStdString() );
    if ( !feature ) return nullptr;

    QAction* action = feature->actionWithUserData( text, userData );
    action->setCheckable( false );
    action->setEnabled( true );
    return action;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowEditorWidget::RiuWorkflowEditorWidget( QWidget* parent )
    : QWidget( parent )
{
    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 2 );

    auto addAction = [this]( const QString& text, const QString& icon, const QKeySequence& shortcut, auto slot )
    {
        auto* action = new QAction( icon.isEmpty() ? QIcon() : QIcon( icon ), text, this );
        if ( !shortcut.isEmpty() )
        {
            action->setShortcut( shortcut );
            action->setShortcutContext( Qt::WidgetWithChildrenShortcut );
            this->addAction( action );
        }
        connect( action, &QAction::triggered, this, slot );
        return action;
    };

    m_saveAction      = addAction( "Save",
                              ":/Save.svg",
                              QKeySequence::Save,
                              [this]()
                              {
                                  if ( m_workflow ) RicWorkflowFeatureTools::saveWorkflow( m_workflow );
                              } );
    m_saveAsAction    = addAction( "Save As...",
                                ":/SaveAs.svg",
                                QKeySequence(),
                                [this]()
                                {
                                    if ( m_workflow ) RicWorkflowFeatureTools::saveWorkflowAs( m_workflow );
                                } );
    m_undoAction      = addAction( "Undo",
                              ":/undo.png",
                              QKeySequence::Undo,
                              [this]()
                              {
                                  if ( m_workflow && m_workflow->canUndo() && !m_workflow->isLocked() ) m_workflow->undo();
                              } );
    m_redoAction      = addAction( "Redo",
                              ":/redo.png",
                              QKeySequence( Qt::CTRL | Qt::Key_Y ),
                              [this]()
                              {
                                  if ( m_workflow && m_workflow->canRedo() && !m_workflow->isLocked() ) m_workflow->redo();
                              } );
    m_duplicateAction = addAction( "Duplicate as Editable",
                                   ":/Copy.svg",
                                   QKeySequence(),
                                   [this]()
                                   {
                                       if ( m_workflow ) RicWorkflowFeatureTools::duplicateAsEditable( m_workflow );
                                   } );
    m_fitAction       = addAction( "Fit", "", QKeySequence(), [this]() { m_graphView->fitGraph(); } );

    m_splitter  = new QSplitter( Qt::Horizontal, this );
    m_palette   = new RiuWorkflowTaskPalette( m_splitter );
    m_graphView = new RiuWorkflowGraphView( m_splitter );
    m_splitter->addWidget( m_graphView );
    m_splitter->addWidget( m_palette );
    m_splitter->setStretchFactor( 0, 1 );
    m_splitter->setStretchFactor( 1, 0 );
    m_splitter->setSizes( { 800, 220 } );
    layout->addWidget( m_splitter, 1 );

    m_statusLabel = new QLabel( this );
    m_statusLabel->setWordWrap( true );
    m_statusLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );
    m_statusLabel->setContentsMargins( 4, 2, 4, 2 );
    layout->addWidget( m_statusLabel );

    m_validationRunner = new RiuWorkflowValidationRunner( this );
    connect( m_validationRunner, &RiuWorkflowValidationRunner::validationStarted, this, &RiuWorkflowEditorWidget::updateStatus );
    connect( m_validationRunner, &RiuWorkflowValidationRunner::validationFinished, this, &RiuWorkflowEditorWidget::onValidationFinished );

    m_graphView->setConnectionValidator(
        [this]( const QString& from, const QString& output, const QString& to, const QString& input ) -> std::expected<void, QString>
        {
            if ( !m_workflow ) return std::unexpected( QString( "No workflow" ) );
            return RimWorkflowPortCompatibility::canConnect( m_workflow->definition(), from, output, to, input );
        } );

    connect( m_graphView, &RiuWorkflowGraphView::nodeSelected, this, &RiuWorkflowEditorWidget::taskSelected );
    connect( m_graphView, &RiuWorkflowGraphView::connectRequested, this, &RiuWorkflowEditorWidget::connectPorts );
    connect( m_graphView, &RiuWorkflowGraphView::deleteRequested, this, &RiuWorkflowEditorWidget::deleteItem );
    connect( m_graphView, &RiuWorkflowGraphView::renameRequested, this, &RiuWorkflowEditorWidget::renameTask );
    connect( m_graphView,
             &RiuWorkflowGraphView::addTaskRequested,
             this,
             [this]( const QString& taskId, const QPointF& scenePos ) { addTask( taskId, scenePos ); } );
    connect( m_graphView, &RiuWorkflowGraphView::contextMenuRequested, this, &RiuWorkflowEditorWidget::showContextMenu );
    connect( m_palette, &RiuWorkflowTaskPalette::taskActivated, this, [this]( const QString& taskId ) { addTask( taskId ); } );

    updateActions();
    updateStatus();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowEditorWidget::~RiuWorkflowEditorWidget()
{
    m_validationRunner->cancel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::setWorkflow( RimWorkflow* workflow, RimWorkflowJob* job )
{
    const bool workflowChanged = workflow != m_workflow.p();
    if ( workflowChanged )
    {
        m_validationRunner->cancel();
        m_message.clear();
    }

    m_workflow = workflow;
    m_job      = job;
    showWorkflow( workflowChanged ? RiuWorkflowGraphView::ViewportPolicy::Fit : RiuWorkflowGraphView::ViewportPolicy::Keep );
    scheduleValidation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflow* RiuWorkflowEditorWidget::workflow() const
{
    return m_workflow.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowJob* RiuWorkflowEditorWidget::job() const
{
    return m_job.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::clear()
{
    m_validationRunner->cancel();
    m_workflow = nullptr;
    m_job      = nullptr;
    m_message.clear();
    showWorkflow( RiuWorkflowGraphView::ViewportPolicy::Fit );
}

//--------------------------------------------------------------------------------------------------
/// Show the current revision of the workflow without moving the view
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::refresh()
{
    showWorkflow( RiuWorkflowGraphView::ViewportPolicy::Keep );
    scheduleValidation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::updateJobState()
{
    m_graphView->resetTaskStates();
    if ( !m_job ) return;

    const auto states = m_job->taskStates();
    const auto errors = m_job->taskErrors();
    for ( auto it = states.cbegin(); it != states.cend(); ++it )
        m_graphView->setTaskState( it.key(), it.value(), errors.value( it.key() ) );

    const auto itemStates = m_job->itemStates();
    const auto itemErrors = m_job->itemErrors();
    for ( auto task = itemStates.cbegin(); task != itemStates.cend(); ++task )
    {
        for ( auto item = task.value().cbegin(); item != task.value().cend(); ++item )
            m_graphView->setTaskItemState( task.key(), item.key(), item.value(), itemErrors.value( task.key() ).value( item.key() ) );
    }
    m_graphView->setRunStatus( m_job->runStatus() );

    // A running job locks the workflow
    m_graphView->setEditable( m_workflow && m_workflow->isEditable() && !m_workflow->isLocked() );
    updateActions();
    updateStatus();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::updateTaskInputValues()
{
    if ( !m_job ) return;
    for ( RimWorkflowTaskInput* task : m_job->taskInputs() )
    {
        for ( RimWorkflowFieldBinding* binding : task->items() )
        {
            if ( binding ) m_graphView->setTaskInputValue( task->taskName(), binding->fieldName(), binding->displayValue() );
        }
    }
    updateTaskIssues();
}

//--------------------------------------------------------------------------------------------------
/// Show the issues of the workflow, with missing-value warnings taken from the displayed job
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::updateTaskIssues()
{
    if ( !m_workflow || !m_job ) return;

    for ( const QJsonValue& value : m_workflow->graph().value( "tasks" ).toArray() )
    {
        const QJsonObject     graphTask = value.toObject();
        const QString         taskName  = graphTask.value( "name" ).toString();
        RimWorkflowTaskInput* taskInput = m_job->taskInput( taskName );

        std::map<QString, bool> fieldHasValue;
        for ( const QJsonValue& field : graphTask.value( "config_fields" ).toArray() )
        {
            const QJsonObject schema    = field.toObject();
            const QString     fieldName = schema.value( "name" ).toString();
            bool              hasValue  = false;
            if ( taskInput )
            {
                for ( const RimWorkflowFieldBinding* binding : taskInput->items() )
                {
                    if ( binding && binding->fieldName() == fieldName ) hasValue = !binding->toJsonValue().isNull();
                }
            }
            fieldHasValue[fieldName] = hasValue || !schema.value( "required" ).toBool();
        }

        m_graphView->setTaskIssues( taskName,
                                    RimWorkflowValidationTools::issuesWithJobValues( taskName,
                                                                                     graphTask.value( "issues" ).toArray(),
                                                                                     fieldHasValue ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView* RiuWorkflowEditorWidget::graphView() const
{
    return m_graphView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowTaskPalette* RiuWorkflowEditorWidget::palette() const
{
    return m_palette;
}

//--------------------------------------------------------------------------------------------------
/// Apply an edit to the displayed workflow. The workflow notifies the main window, which refreshes
/// this widget.
//--------------------------------------------------------------------------------------------------
bool RiuWorkflowEditorWidget::applyEdit( const std::expected<RimWorkflowDefinition, QString>& result, const QMap<QString, QString>& renames )
{
    if ( !m_workflow ) return false;

    auto applied = m_workflow->applyEdit( result, renames );
    if ( !applied )
    {
        showMessage( applied.error() );
        return false;
    }

    m_message.clear();
    updateActions();
    updateStatus();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::addTask( const QString& taskId, const std::optional<QPointF>& scenePos )
{
    if ( !m_workflow ) return;

    QString addedName;
    const auto result = RimWorkflowDefinitionTools::addTask( m_workflow->definition(), RimWorkflowTaskCatalog::instance(), taskId, &addedName );
    if ( result && scenePos ) m_graphView->setPendingNodePosition( addedName, *scenePos );
    if ( applyEdit( result ) ) m_graphView->selectTask( addedName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::deleteItem( const RiuWorkflowGraphView::ItemRef& item )
{
    if ( !m_workflow ) return;

    if ( item.kind == RiuWorkflowGraphView::ItemRef::Kind::Task )
    {
        applyEdit( RimWorkflowDefinitionTools::removeTask( m_workflow->definition(), item.task ) );
    }
    else if ( item.kind == RiuWorkflowGraphView::ItemRef::Kind::Edge )
    {
        const RimWorkflowDefinitionEdge edge{ .from = item.from, .output = item.output, .to = item.to, .input = item.input };
        applyEdit( RimWorkflowDefinitionTools::disconnect( m_workflow->definition(), edge ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::renameTask( const QString& taskName )
{
    if ( !m_workflow || !m_workflow->definition().findNode( taskName ) ) return;
    if ( !m_workflow->isEditable() || m_workflow->isLocked() )
    {
        showMessage( m_workflow->isLocked() ? QString( "The workflow is locked while a job is running" ) : m_workflow->readOnlyReason() );
        return;
    }

    bool          ok = false;
    const QString newName =
        QInputDialog::getText( this, "Rename Task", "Task name (letters, digits and underscores):", QLineEdit::Normal, taskName, &ok ).trimmed();
    if ( !ok || newName == taskName ) return;

    const auto result = RimWorkflowDefinitionTools::renameTask( m_workflow->definition(), taskName, newName );
    if ( result ) m_graphView->renameNodePosition( taskName, newName );
    if ( applyEdit( result, { { taskName, newName } } ) )
    {
        m_graphView->selectTask( newName );
    }
    else if ( result )
    {
        m_graphView->renameNodePosition( newName, taskName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::setResultTask( const QString& taskName )
{
    if ( !m_workflow ) return;
    applyEdit( RimWorkflowDefinitionTools::setResultTask( m_workflow->definition(), taskName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::setOptionalInputConfigured( const QString& taskName, const QString& field, bool configured )
{
    if ( !m_workflow ) return;
    applyEdit( RimWorkflowDefinitionTools::setOptionalInputConfigured( m_workflow->definition(), taskName, field, configured ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::connectPorts( const QString& from, const QString& output, const QString& to, const QString& input )
{
    if ( !m_workflow ) return;
    const RimWorkflowDefinitionEdge edge{ .from = from, .output = output, .to = to, .input = input };
    applyEdit( RimWorkflowDefinitionTools::connect( m_workflow->definition(), edge ) );
}

//--------------------------------------------------------------------------------------------------
/// Shows why the workflow cannot be edited
//--------------------------------------------------------------------------------------------------
bool RiuWorkflowEditorWidget::canEdit()
{
    if ( !m_workflow ) return false;
    if ( m_workflow->isEditable() && !m_workflow->isLocked() ) return true;

    showMessage( m_workflow->isLocked() ? QString( "The workflow is locked while a job is running" ) : m_workflow->readOnlyReason() );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWorkflowDefinitionEdge* RiuWorkflowEditorWidget::findEdge( const RiuWorkflowGraphView::ItemRef& item ) const
{
    if ( !m_workflow || item.kind != RiuWorkflowGraphView::ItemRef::Kind::Edge ) return nullptr;

    const RimWorkflowDefinitionEdge wanted{ .from = item.from, .output = item.output, .to = item.to, .input = item.input };
    for ( const auto& edge : m_workflow->definition().edges )
    {
        if ( edge.sameConnection( wanted ) ) return &edge;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Run the task once per item of a mapping. The dialog edits the current map, if any.
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::editTaskMap( const QString& taskName )
{
    if ( !canEdit() ) return;
    const RimWorkflowDefinition&     definition = m_workflow->definition();
    const RimWorkflowDefinitionNode* node       = definition.findNode( taskName );
    if ( !node ) return;

    QStringList fields;
    for ( const auto& port : RimWorkflowPortCompatibility::inputPorts( definition.taskType( node->taskId ) ) )
    {
        if ( !port.isWhole() ) fields.append( port.name );
    }
    if ( fields.size() < 2 )
    {
        showMessage( QString( "Task '%1' needs two inputs to receive the key and the value of each item" ).arg( taskName ) );
        return;
    }

    const RimWorkflowTaskMap current = node->map.value_or( RimWorkflowTaskMap{ .over = "items" } );

    QDialog dialog( this );
    dialog.setWindowTitle( QString( "Map Over - %1" ).arg( taskName ) );
    auto* form = new QFormLayout( &dialog );

    auto* overEdit = new QLineEdit( current.over, &dialog );
    overEdit->setToolTip( "Name of the configured mapping. Each key and value of the mapping runs the task once." );
    form->addRow( "Map over:", overEdit );

    auto addFieldCombo = [&]( const QString& label, const QString& value, const QString& fallback, const QString& toolTip )
    {
        auto* combo = new QComboBox( &dialog );
        combo->addItems( fields );
        combo->setCurrentText( fields.contains( value ) ? value : fallback );
        combo->setToolTip( toolTip );
        form->addRow( label, combo );
        return combo;
    };
    auto* keyCombo   = addFieldCombo( "Key goes to:", current.keyAs, fields.front(), "The input that receives the key of each item" );
    auto* valueCombo = addFieldCombo( "Value goes to:", current.valueAs, fields.at( 1 ), "The input that receives the value of each item" );

    auto* errorCombo = new QComboBox( &dialog );
    errorCombo->addItem( "Stop at the first failed item", "fail_fast" );
    errorCombo->addItem( "Run every item, then fail if any failed", "collect_all" );
    errorCombo->setCurrentIndex( std::max( 0, errorCombo->findData( current.errorMode ) ) );
    form->addRow( "On failure:", errorCombo );

    auto* errorLabel = new QLabel( &dialog );
    errorLabel->setStyleSheet( "color: #b00020;" );
    errorLabel->setWordWrap( true );
    form->addRow( errorLabel );

    auto* buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog );
    form->addRow( buttons );

    auto mapFromDialog = [&]()
    {
        return RimWorkflowTaskMap{ .over      = overEdit->text().trimmed(),
                                   .keyAs     = keyCombo->currentText(),
                                   .valueAs   = valueCombo->currentText(),
                                   .errorMode = errorCombo->currentData().toString() };
    };
    auto updateValidity = [&]()
    {
        const auto valid = RimWorkflowDefinitionTools::checkTaskMap( definition, taskName, mapFromDialog() );
        errorLabel->setText( valid ? QString() : valid.error() );
        errorLabel->setVisible( !valid );
        buttons->button( QDialogButtonBox::Ok )->setEnabled( valid.has_value() );
    };
    QObject::connect( overEdit, &QLineEdit::textChanged, &dialog, updateValidity );
    QObject::connect( keyCombo, &QComboBox::currentIndexChanged, &dialog, updateValidity );
    QObject::connect( valueCombo, &QComboBox::currentIndexChanged, &dialog, updateValidity );
    QObject::connect( buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept );
    QObject::connect( buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject );
    updateValidity();

    if ( dialog.exec() != QDialog::Accepted ) return;

    const RimWorkflowTaskMap map = mapFromDialog();
    if ( node->map && *node->map == map ) return;
    applyEdit( RimWorkflowDefinitionTools::setTaskMap( definition, taskName, map ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::removeTaskMap( const QString& taskName )
{
    if ( !canEdit() ) return;
    applyEdit( RimWorkflowDefinitionTools::setTaskMap( m_workflow->definition(), taskName, std::nullopt ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::renameCollectKey( const RiuWorkflowGraphView::ItemRef& item )
{
    if ( !canEdit() ) return;
    const RimWorkflowDefinitionEdge* edge = findEdge( item );
    if ( !edge || edge->collect != RimWorkflowDefinitionEdge::Collect::Dict ) return;

    bool          ok  = false;
    const QString key = QInputDialog::getText( this,
                                               "Rename Key",
                                               QString( "Key of '%1' in '%2':" ).arg( edge->from, edge->input ),
                                               QLineEdit::Normal,
                                               edge->key,
                                               &ok )
                            .trimmed();
    if ( !ok || key == edge->key ) return;

    applyEdit( RimWorkflowDefinitionTools::setCollectKey( m_workflow->definition(), *edge, key ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::moveCollectMember( const RiuWorkflowGraphView::ItemRef& item, int delta )
{
    if ( !canEdit() ) return;
    const RimWorkflowDefinitionEdge* edge = findEdge( item );
    if ( !edge ) return;

    applyEdit( RimWorkflowDefinitionTools::moveCollectMember( m_workflow->definition(), *edge, delta ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::showMessage( const QString& message )
{
    m_message = message;
    updateStatus();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::showWorkflow( RiuWorkflowGraphView::ViewportPolicy policy )
{
    if ( !m_workflow )
    {
        m_graphView->setEditable( false );
        m_graphView->showGraph( {}, {}, policy );
    }
    else
    {
        m_graphView->setEditable( m_workflow->isEditable() && !m_workflow->isLocked() );
        m_graphView->showGraph( m_workflow->graph(), m_workflow->loadError(), policy );
        updateTaskInputValues();
        if ( m_job ) updateJobState();
    }

    m_palette->setEnabled( m_workflow && m_workflow->isEditable() && !m_workflow->isLocked() );
    updateActions();
    updateStatus();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::showContextMenu( const RiuWorkflowGraphView::ItemRef& item, const QPoint& globalPos, const QPointF& scenePos )
{
    if ( !m_workflow ) return;

    const bool editable = m_workflow->isEditable() && !m_workflow->isLocked();

    QMenu menu;
    if ( item.kind == RiuWorkflowGraphView::ItemRef::Kind::Task && editable )
    {
        const QVariantMap data{ { "task", item.task } };
        if ( auto* action = featureAction( "RicRenameWorkflowTaskFeature", "Rename...", data ) ) menu.addAction( action );
        if ( auto* action = featureAction( "RicSetWorkflowResultTaskFeature", "Set as Result Task", data ) )
        {
            action->setEnabled( RimWorkflowDefinitionTools::effectiveResultTask( m_workflow->definition() ) != item.task ||
                                m_workflow->definition().resultTask.isEmpty() );
            menu.addAction( action );
        }
        appendOptionalInputsMenu( &menu, item.task );
        appendMapMenu( &menu, item.task );
        menu.addSeparator();
        if ( auto* action = featureAction( "RicDeleteWorkflowTaskFeature", "Delete Task", data ) ) menu.addAction( action );
    }
    else if ( item.kind == RiuWorkflowGraphView::ItemRef::Kind::Edge && editable )
    {
        const QVariantMap data{ { "from", item.from }, { "output", item.output }, { "to", item.to }, { "input", item.input } };
        appendCollectMenu( &menu, item );
        if ( auto* action = featureAction( "RicDeleteWorkflowConnectionFeature", "Delete Connection", data ) ) menu.addAction( action );
    }
    else
    {
        if ( editable )
        {
            appendAddTaskMenu( &menu, scenePos );
            menu.addSeparator();
            menu.addAction( m_undoAction );
            menu.addAction( m_redoAction );
            menu.addSeparator();
            menu.addAction( m_saveAction );
            menu.addAction( m_saveAsAction );
        }
        menu.addAction( m_duplicateAction );
        menu.addSeparator();
        menu.addAction( m_fitAction );
    }

    if ( !menu.isEmpty() ) menu.exec( globalPos );
}

//--------------------------------------------------------------------------------------------------
/// The installed task types, grouped by package
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::appendAddTaskMenu( QMenu* menu, const QPointF& scenePos )
{
    QMenu* addMenu = menu->addMenu( "Add Task" );

    const RimWorkflowTaskCatalog& catalog = RimWorkflowTaskCatalog::instance();
    if ( !catalog.isValid() )
    {
        addMenu->addAction( QString( "Task catalog unavailable: %1" ).arg( catalog.loadError() ) )->setEnabled( false );
        return;
    }

    std::map<QString, std::vector<QJsonObject>> groups;
    for ( const QJsonObject& task : catalog.tasks() )
    {
        const QString id = task.value( "id" ).toString();
        groups[id.contains( '.' ) ? id.section( '.', 0, -2 ) : QString( "other" )].push_back( task );
    }

    QSet<QString> usedTexts;
    for ( auto& [group, tasks] : groups )
    {
        std::sort( tasks.begin(),
                   tasks.end(),
                   []( const QJsonObject& a, const QJsonObject& b ) { return a.value( "id" ).toString() < b.value( "id" ).toString(); } );

        QMenu* groupMenu = groups.size() > 1 ? addMenu->addMenu( group ) : addMenu;
        for ( const QJsonObject& task : tasks )
        {
            const QString id   = task.value( "id" ).toString();
            QString       text = task.value( "name" ).toString( id );
            if ( text.isEmpty() || usedTexts.contains( text ) ) text = id;
            usedTexts.insert( text );

            const QVariantMap data{ { "taskId", id }, { "x", scenePos.x() }, { "y", scenePos.y() } };
            if ( auto* action = featureAction( "RicAddWorkflowTaskFeature", text, data ) )
            {
                action->setToolTip( task.value( "description" ).toString() );
                groupMenu->addAction( action );
            }
        }
    }
    addMenu->setToolTipsVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::appendMapMenu( QMenu* menu, const QString& taskName )
{
    const RimWorkflowDefinitionNode* node = m_workflow->definition().findNode( taskName );
    if ( !node ) return;

    const QVariantMap data{ { "task", taskName } };
    if ( auto* action = featureAction( "RicMapWorkflowTaskFeature", node->map ? "Edit Map..." : "Map Over...", data ) )
    {
        action->setToolTip( "Run the task once for each item of a mapping and collect the outputs by key" );
        menu->addAction( action );
    }
    if ( node->map )
    {
        if ( auto* action = featureAction( "RicRemoveWorkflowTaskMapFeature", "Remove Map", data ) ) menu->addAction( action );
    }
}

//--------------------------------------------------------------------------------------------------
/// Collected connections: dict members have a key, list members an order
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::appendCollectMenu( QMenu* menu, const RiuWorkflowGraphView::ItemRef& item )
{
    const RimWorkflowDefinitionEdge* edge = findEdge( item );
    if ( !edge || !edge->isCollected() ) return;

    const QVariantMap data{ { "from", item.from }, { "output", item.output }, { "to", item.to }, { "input", item.input } };
    if ( edge->collect == RimWorkflowDefinitionEdge::Collect::Dict )
    {
        if ( auto* action = featureAction( "RicRenameWorkflowCollectKeyFeature", "Rename Key...", data ) ) menu->addAction( action );
    }
    else
    {
        int index = 0;
        int count = 0;
        for ( const auto& other : m_workflow->definition().edges )
        {
            if ( other.to != edge->to || other.input != edge->input || !other.isCollected() ) continue;
            if ( &other == edge ) index = count;
            ++count;
        }

        for ( const auto& [text, delta] : { std::pair{ QString( "Move Earlier" ), -1 }, std::pair{ QString( "Move Later" ), 1 } } )
        {
            QVariantMap moveData = data;
            moveData["delta"]    = delta;
            if ( auto* action = featureAction( "RicMoveWorkflowCollectMemberFeature", text, moveData ) )
            {
                action->setEnabled( index + delta >= 0 && index + delta < count );
                menu->addAction( action );
            }
        }
    }
    menu->addSeparator();
}

//--------------------------------------------------------------------------------------------------
/// Optional inputs are configured by the user only when checked; connected inputs are disabled
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::appendOptionalInputsMenu( QMenu* menu, const QString& taskName )
{
    const RimWorkflowDefinition&     definition = m_workflow->definition();
    const RimWorkflowDefinitionNode* node       = definition.findNode( taskName );
    if ( !node ) return;

    std::vector<RimWorkflowPort> optionalPorts;
    for ( const auto& port : RimWorkflowPortCompatibility::inputPorts( definition.taskTypeForNode( taskName ) ) )
    {
        if ( !port.isWhole() && !port.required ) optionalPorts.push_back( port );
    }
    if ( optionalPorts.empty() ) return;

    QStringList connected;
    for ( const auto& edge : definition.incomingEdges( taskName ) )
        connected.append( edge.input );

    QMenu* optionalMenu = menu->addMenu( "Optional Inputs" );
    for ( const auto& port : optionalPorts )
    {
        const bool        configured = node->explicitConfigFields.contains( port.name );
        const QVariantMap data{ { "task", taskName }, { "field", port.name }, { "configured", !configured } };
        if ( auto* action = featureAction( "RicToggleWorkflowOptionalInputFeature", port.name, data ) )
        {
            action->setCheckable( true );
            action->setChecked( configured );
            action->setEnabled( !connected.contains( port.name ) );
            action->setToolTip( port.description );
            optionalMenu->addAction( action );
        }
    }
    optionalMenu->setToolTipsVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::updateActions()
{
    RimWorkflow* workflow = m_workflow.p();
    const bool   editable = workflow && workflow->isEditable() && !workflow->isLocked();

    m_saveAction->setEnabled( editable && ( workflow->isDirty() || workflow->source() == RimWorkflow::Source::Unsaved ) );
    m_saveAsAction->setEnabled( editable );
    m_undoAction->setEnabled( editable && workflow->canUndo() );
    m_redoAction->setEnabled( editable && workflow->canRedo() );
    m_duplicateAction->setEnabled( workflow && workflow->loadError().isEmpty() && workflow->definition().editable );
    m_fitAction->setEnabled( workflow != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::updateStatus()
{
    RimWorkflow* workflow = m_workflow.p();
    QStringList  lines;
    QString      color;

    if ( !workflow )
    {
        m_statusLabel->clear();
        return;
    }

    if ( !workflow->loadError().isEmpty() )
    {
        lines << workflow->loadError();
        color = "#c0392b";
    }
    else if ( !workflow->isEditable() )
    {
        lines << QString( "Read-only: %1. Use Duplicate as Editable to change it." ).arg( workflow->readOnlyReason() );
        color = "#7f8c8d";
    }
    else
    {
        if ( !m_message.isEmpty() )
        {
            lines << m_message;
            color = "#c0392b";
        }
        if ( workflow->isLocked() ) lines << "The workflow is locked while a job is running.";

        const QJsonArray issues = workflow->graph().value( "issues" ).toArray();
        for ( const QJsonValue& issue : issues )
            lines << issue.toObject().value( "message" ).toString();

        if ( m_validationRunner->isRunning() || !workflow->isValidationCurrent() )
        {
            lines << "Validating...";
        }
        else if ( workflow->lastValidationFailed() )
        {
            if ( issues.isEmpty() ) lines << "taskmaestro reports problems; see the marked tasks.";
            if ( color.isEmpty() ) color = "#d35400";
        }
        else if ( issues.isEmpty() && m_message.isEmpty() )
        {
            lines << "Valid";
            if ( color.isEmpty() ) color = "#27ae60";
        }
        if ( !issues.isEmpty() && color.isEmpty() ) color = "#d35400";

        if ( workflow->source() == RimWorkflow::Source::Unsaved )
            lines << "Not saved yet";
        else if ( workflow->isDirty() )
            lines << "Unsaved changes";
    }

    m_statusLabel->setStyleSheet( color.isEmpty() ? QString() : QString( "color: %1;" ).arg( color ) );
    m_statusLabel->setText( lines.join( "  |  " ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::scheduleValidation()
{
    if ( m_workflow && m_workflow->isEditable() && !m_workflow->isValidationCurrent() ) m_validationRunner->schedule( m_workflow );
    updateStatus();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowEditorWidget::onValidationFinished( RimWorkflow* workflow )
{
    if ( workflow != m_workflow.p() ) return;
    updateActions();
    updateStatus();
}

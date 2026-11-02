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

#include "RiuWorkflowGraphView.h"

#include "cafPdmPointer.h"

#include <QWidget>

#include <expected>
#include <optional>

class QAction;
class QLabel;
class QMenu;
class QSplitter;
class QTabBar;
class RimWorkflow;
class RimWorkflowJob;
struct RimWorkflowDefinition;
struct RimWorkflowDefinitionEdge;
class RiuWorkflowTaskPalette;
class RiuWorkflowValidationRunner;

//==================================================================================================
/// The workflow dock: the graph, the task palette and a status line. Edits go through
/// RimWorkflow::applyEdit(); the workflow then calls RiuMainWindow::workflowDefinitionChanged(),
/// which refreshes this widget. Nested workflows open in tabs above the graph; they are read-only
/// and have no job. workflow() and rootJob() are those of the first tab.
//==================================================================================================
class RiuWorkflowEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RiuWorkflowEditorWidget( QWidget* parent = nullptr );
    ~RiuWorkflowEditorWidget() override;

    void            setWorkflow( RimWorkflow* workflow, RimWorkflowJob* job );
    RimWorkflow*    workflow() const;
    RimWorkflowJob* job() const;
    RimWorkflowJob* rootJob() const;
    RimWorkflow*    displayedWorkflow() const;
    void            clear();
    void            refresh();
    void            updateJobState();
    void            updateTaskInputValues();
    void            updateTaskIssues();

    RiuWorkflowGraphView*   graphView() const;
    RiuWorkflowTaskPalette* palette() const;

    // Edit operations of the graph context menus. Errors are shown in the status line.
    bool applyEdit( const std::expected<RimWorkflowDefinition, QString>& result, const QMap<QString, QString>& renames = {} );
    void addTask( const QString& taskId, const std::optional<QPointF>& scenePos = std::nullopt );
    void deleteItem( const RiuWorkflowGraphView::ItemRef& item );
    void renameTask( const QString& taskName );
    void setResultTask( const QString& taskName );
    void setOptionalInputConfigured( const QString& taskName, const QString& field, bool configured );
    void editTaskMap( const QString& taskName );
    void removeTaskMap( const QString& taskName );
    void editTaskGuard( const QString& taskName );
    void removeTaskGuard( const QString& taskName );
    void renameCollectKey( const RiuWorkflowGraphView::ItemRef& edge );
    void moveCollectMember( const RiuWorkflowGraphView::ItemRef& edge, int delta );
    void connectPorts( const QString& from, const QString& output, const QString& to, const QString& input );

    void showMessage( const QString& message );
    void openNestedWorkflow( const QString& taskName );

signals:
    void taskSelected( const QString& taskName );

private:
    struct Tab
    {
        caf::PdmPointer<RimWorkflow>                   workflow;
        QStringList                                    taskPath; // Empty for the root workflow
        std::optional<RiuWorkflowGraphView::ViewState> viewState;
    };

    void activateTab( int index, bool keepViewState );
    void closeTab( int index );
    void closeNestedTabs();
    bool reopenNestedTabs();
    void updateTabBar();
    bool isNestedTabShown() const;

    void showWorkflow( RiuWorkflowGraphView::ViewportPolicy policy );
    void showContextMenu( const RiuWorkflowGraphView::ItemRef& item, const QPoint& globalPos, const QPointF& scenePos );
    void appendAddTaskMenu( QMenu* menu, const QPointF& scenePos );
    void appendMapMenu( QMenu* menu, const QString& taskName );
    void appendConditionMenu( QMenu* menu, const QString& taskName );
    void appendCollectMenu( QMenu* menu, const RiuWorkflowGraphView::ItemRef& item );
    bool canEdit();
    void renameNodePositions( const QMap<QString, QString>& renames );

    const RimWorkflowDefinitionEdge* findEdge( const RiuWorkflowGraphView::ItemRef& item ) const;

    void appendOptionalInputsMenu( QMenu* menu, const QString& taskName );
    void appendShowHiddenMenu( QMenu* menu );
    void setTaskHidden( const QString& taskName, bool hidden );
    void setTaskCollapsed( const QString& taskName, bool collapsed );
    void appendCollapseMenu( QMenu* menu );
    void appendTaskViewActions( QMenu* menu, const QString& taskName );

    QJsonArray  taskIssues( const QJsonObject& graphTask ) const;
    QStringList effectiveHiddenTasks() const;
    void        updateActions();
    void        updateStatus();
    void        scheduleValidation();
    void        onValidationFinished( RimWorkflow* workflow );

private:
    caf::PdmPointer<RimWorkflow>    m_workflow; // The workflow of the current tab
    caf::PdmPointer<RimWorkflowJob> m_job; // The root job in the first tab, else null
    caf::PdmPointer<RimWorkflowJob> m_rootJob;
    std::vector<Tab>                m_tabs;
    int                             m_currentTab = 0;
    QTabBar*                        m_tabBar;
    QAction*                        m_saveAction;
    QAction*                        m_saveAsAction;
    QAction*                        m_undoAction;
    QAction*                        m_redoAction;
    QAction*                        m_duplicateAction;
    QAction*                        m_fitAction;
    QAction*                        m_inlineConfigAction;
    QSplitter*                      m_splitter;
    RiuWorkflowTaskPalette*         m_palette;
    RiuWorkflowGraphView*           m_graphView;
    QLabel*                         m_statusLabel;
    RiuWorkflowValidationRunner*    m_validationRunner;
    QString                         m_message;
    QStringList                     m_shownHiddenTasks;
};

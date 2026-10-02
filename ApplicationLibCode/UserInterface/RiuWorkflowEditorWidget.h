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
class RimWorkflow;
class RimWorkflowJob;
struct RimWorkflowDefinition;
class RiuWorkflowTaskPalette;
class RiuWorkflowValidationRunner;

//==================================================================================================
/// The workflow dock: the graph, the task palette and a status line. Edits go through
/// RimWorkflow::applyEdit(); the workflow then calls RiuMainWindow::workflowDefinitionChanged(),
/// which refreshes this widget.
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
    void            clear();
    void            refresh();
    void            updateJobState();
    void            updateTaskInputValues();

    RiuWorkflowGraphView*   graphView() const;
    RiuWorkflowTaskPalette* palette() const;

    // Edit operations used by the graph commands. Errors are shown in the status line.
    bool applyEdit( const std::expected<RimWorkflowDefinition, QString>& result, const QMap<QString, QString>& renames = {} );
    void addTask( const QString& taskId, const std::optional<QPointF>& scenePos = std::nullopt );
    void deleteItem( const RiuWorkflowGraphView::ItemRef& item );
    void renameTask( const QString& taskName );
    void setResultTask( const QString& taskName );
    void setOptionalInputConfigured( const QString& taskName, const QString& field, bool configured );
    void connectPorts( const QString& from, const QString& output, const QString& to, const QString& input );

    void showMessage( const QString& message );

signals:
    void taskSelected( const QString& taskName );

private:
    void showWorkflow( RiuWorkflowGraphView::ViewportPolicy policy );
    void showContextMenu( const RiuWorkflowGraphView::ItemRef& item, const QPoint& globalPos, const QPointF& scenePos );
    void appendAddTaskMenu( QMenu* menu, const QPointF& scenePos );
    void appendOptionalInputsMenu( QMenu* menu, const QString& taskName );
    void updateActions();
    void updateStatus();
    void scheduleValidation();
    void onValidationFinished( RimWorkflow* workflow );

private:
    caf::PdmPointer<RimWorkflow>    m_workflow;
    caf::PdmPointer<RimWorkflowJob> m_job;
    QAction*                        m_saveAction;
    QAction*                        m_saveAsAction;
    QAction*                        m_undoAction;
    QAction*                        m_redoAction;
    QAction*                        m_duplicateAction;
    QAction*                        m_fitAction;
    QSplitter*                      m_splitter;
    RiuWorkflowTaskPalette*         m_palette;
    RiuWorkflowGraphView*           m_graphView;
    QLabel*                         m_statusLabel;
    RiuWorkflowValidationRunner*    m_validationRunner;
    QString                         m_message;
};

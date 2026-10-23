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
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  at <http://www.gnu.org/licenses/gpl.html> for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#pragma once

#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QPointF>

#include <expected>
#include <functional>

class QGraphicsScene;
class QGraphicsPathItem;
class QGraphicsTextItem;

namespace RiuWorkflowGraphViewItems
{
class GraphNode;
class PortItem;
} // namespace RiuWorkflowGraphViewItems

//==================================================================================================
/// Node graph of a workflow. Shows run states of a job and, in edit mode, lets the user connect
/// ports by dragging, select and delete tasks and connections, and drop tasks from the palette.
/// The view never changes the workflow itself; it emits requests handled by the editor widget.
//==================================================================================================
class RiuWorkflowGraphView : public QGraphicsView
{
    Q_OBJECT

public:
    enum class ViewportPolicy
    {
        Fit,
        Keep
    };

    struct ItemRef
    {
        enum class Kind
        {
            None,
            Task,
            Edge
        };

        Kind    kind = Kind::None;
        QString task;
        QString from;
        QString output;
        QString to;
        QString input;

        static ItemRef forTask( const QString& taskName );
        static ItemRef forEdge( const QString& from, const QString& output, const QString& to, const QString& input );
    };

    // Returns why a connection is not possible. Fields are empty for the whole input or output.
    using ConnectionValidator =
        std::function<std::expected<void, QString>( const QString& from, const QString& output, const QString& to, const QString& input )>;

    static constexpr const char* taskMimeType = "application/x-resinsight-workflow-task";

    explicit RiuWorkflowGraphView( QWidget* parent = nullptr );
    ~RiuWorkflowGraphView() override;

    void showGraph( const QJsonObject& graph, const QString& error, ViewportPolicy policy = ViewportPolicy::Fit );
    void fitGraph();

    void setEditable( bool editable );
    bool isEditable() const;
    void setConnectionValidator( const ConnectionValidator& validator );
    void setTaskIssues( const QString& taskName, const QJsonArray& issues );

    void    setPendingNodePosition( const QString& taskName, const QPointF& scenePos );
    void    renameNodePosition( const QString& oldName, const QString& newName );
    void    selectTask( const QString& taskName );
    ItemRef selectedItem() const;

    void setTaskInputValue( const QString& taskName, const QString& fieldName, const QString& value );
    void resetTaskStates();
    void setTaskState( const QString& taskName, const QString& state, const QString& error = {} );
    void setTaskItemState( const QString& taskName, const QString& item, const QString& state, const QString& error = {} );
    void setRunStatus( const QString& status );

signals:
    void nodeSelected( const QString& taskName );
    void connectRequested( const QString& from, const QString& outputField, const QString& to, const QString& inputField );
    void deleteRequested( const RiuWorkflowGraphView::ItemRef& item );
    void renameRequested( const QString& taskName );
    void addTaskRequested( const QString& taskId, const QPointF& scenePos );
    void contextMenuRequested( const RiuWorkflowGraphView::ItemRef& item, const QPoint& globalPos, const QPointF& scenePos );

protected:
    void resizeEvent( QResizeEvent* event ) override;
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseMoveEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;
    void contextMenuEvent( QContextMenuEvent* event ) override;
    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dragMoveEvent( QDragMoveEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

private:
    void onSelectionChanged();
    void rememberPositions();

    RiuWorkflowGraphViewItems::PortItem* portAt( const QPoint& viewPos ) const;
    ItemRef                              itemRefAt( const QPoint& viewPos ) const;
    void                                 startConnectionDrag( RiuWorkflowGraphViewItems::PortItem* port );
    void                                 updateConnectionDrag( const QPointF& scenePos );
    void                                 finishConnectionDrag( bool connect );

private:
    QGraphicsScene*     m_scene;
    QGraphicsTextItem*  m_runStatusLabel = nullptr;
    bool                m_fitOnResize    = true;
    bool                m_editable       = false;
    bool                m_rebuilding     = false;
    ConnectionValidator m_validator;
    QString             m_lastSelectedTask;

    QMap<QString, RiuWorkflowGraphViewItems::GraphNode*> m_taskNodes;
    QMap<QString, RiuWorkflowGraphViewItems::GraphNode*> m_configNodes;
    QMap<QString, QPointF>                               m_positions;
    QMap<QString, QPointF>                               m_pendingPositions;

    RiuWorkflowGraphViewItems::PortItem*                m_dragPort   = nullptr;
    RiuWorkflowGraphViewItems::PortItem*                m_dragTarget = nullptr;
    QGraphicsPathItem*                                  m_dragPath   = nullptr;
    QMap<RiuWorkflowGraphViewItems::PortItem*, QString> m_dragInvalidReasons;
};

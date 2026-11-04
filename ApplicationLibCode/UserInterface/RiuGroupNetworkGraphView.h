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

#include "RimGroupNetworkGraph.h"

#include <QGraphicsView>
#include <QMap>
#include <QStringList>

#include <vector>

class QGraphicsScene;

namespace RiuGroupNetworkGraphViewItems
{
class NetworkNode;
}

//==================================================================================================
/// Read-only graph of a production network, drawn in the style of the group tree plot of webviz: circular
/// nodes and edges as curves from a parent to its children. Edge width shows the flow from a node to its
/// parent. The view does not change the model; collapse and plot requests are emitted as signals.
//==================================================================================================
class RiuGroupNetworkGraphView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit RiuGroupNetworkGraphView( QWidget* parent = nullptr );
    ~RiuGroupNetworkGraphView() override;

    void setNodes( const std::vector<RimGroupNetworkGraph::Node>& nodes );
    void clear();
    void fitGraph();

    QString selectedNode() const;

signals:
    void nodeSelected( const QString& nodeName );
    void toggleCollapseRequested( const QString& nodeName );
    void createPlotRequested( const QString& nodeName );
    void expandAllRequested();
    void collapseAllRequested();

protected:
    void resizeEvent( QResizeEvent* event ) override;
    void mousePressEvent( QMouseEvent* event ) override;
    void mouseReleaseEvent( QMouseEvent* event ) override;
    void mouseDoubleClickEvent( QMouseEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;
    void contextMenuEvent( QContextMenuEvent* event ) override;

private:
    void onSelectionChanged();

    RiuGroupNetworkGraphViewItems::NetworkNode* nodeAt( const QPoint& viewPos ) const;

private:
    QGraphicsScene*                                            m_scene;
    QMap<QString, RiuGroupNetworkGraphViewItems::NetworkNode*> m_nodes;
    QStringList                                                m_structure;
    QString                                                    m_selectedNode;
    bool                                                       m_fitOnResize = true;
    bool                                                       m_updating    = false;
};

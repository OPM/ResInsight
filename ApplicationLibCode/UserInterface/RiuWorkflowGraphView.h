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

#include <QGraphicsView>
#include <QJsonObject>

class QGraphicsScene;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;

class RiuWorkflowGraphView : public QGraphicsView
{
public:
    explicit RiuWorkflowGraphView( QWidget* parent = nullptr );

    void showGraph( const QJsonObject& graph, const QString& error );
    void setTaskInputValue( const QString& taskName, const QString& fieldName, const QString& value );

protected:
    void resizeEvent( QResizeEvent* event ) override;
    void mousePressEvent( QMouseEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;

private:
    QGraphicsScene* m_scene;
    bool            m_fitOnResize = true;
};

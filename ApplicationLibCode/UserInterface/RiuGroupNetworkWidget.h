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

#include "cafPdmPointer.h"
#include "cafSignal.h"

#include <QWidget>

class QLabel;
class QSlider;
class RimGroupNetworkPlot;
class RiuGroupNetworkGraphView;

//==================================================================================================
/// The view widget of a production network plot: a time step slider above the graph and a status
/// line below it
//==================================================================================================
class RiuGroupNetworkWidget : public QWidget, public caf::SignalObserver
{
    Q_OBJECT

public:
    explicit RiuGroupNetworkWidget( QWidget* parent = nullptr );
    ~RiuGroupNetworkWidget() override;

    void                 setNetwork( RimGroupNetworkPlot* network );
    RimGroupNetworkPlot* network() const;
    void                 clear();
    void                 zoomAll();

signals:
    void nodeSelected( const QString& nodeName );

private:
    void refresh();
    void onNetworkChanged( const caf::SignalEmitter* emitter );
    void onTimeStepChanged( int index );
    void onToggleCollapse( const QString& nodeName );
    void onCreatePlot( const QString& nodeName );

private:
    caf::PdmPointer<RimGroupNetworkPlot> m_network;
    RiuGroupNetworkGraphView*            m_graphView;
    QSlider*                             m_timeStepSlider;
    QLabel*                              m_dateLabel;
    QLabel*                              m_statusLabel;
    bool                                 m_refreshing = false;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include <QWidget>

class RiuQwtPlotWidget;

class QPaintDevice;
class QScrollBar;

//==================================================================================================
//
// RiuMultiPlotInterface
//
//==================================================================================================
class RiuMultiPlotInterface : public QWidget
{
    Q_OBJECT

public:
    RiuMultiPlotInterface( QWidget* parent = nullptr )
        : QWidget( parent )
    {
    }
    virtual ~RiuMultiPlotInterface() {}

    virtual void addPlot( RiuQwtPlotWidget* plotWidget )                  = 0;
    virtual void insertPlot( RiuQwtPlotWidget* plotWidget, size_t index ) = 0;
    virtual void removePlot( RiuQwtPlotWidget* plotWidget )               = 0;
    virtual int  indexOfPlotWidget( RiuQwtPlotWidget* plotWidget )        = 0;

    virtual void setPlotTitle( const QString& plotTitle ) = 0;
    virtual void setTitleVisible( bool visible )          = 0;
    virtual void setFontSize( int fontSize )              = 0;
    virtual int  fontSize() const                         = 0;

    virtual bool previewModeEnabled() const                = 0;
    virtual void setPreviewModeEnabled( bool previewMode ) = 0;

    virtual void scheduleUpdate()                                                                                  = 0;
    virtual void scheduleReplotOfAllPlots()                                                                        = 0;
    virtual void updateVerticalScrollBar( double visibleMin, double visibleMax, double totalMin, double totalMax ) = 0;

    virtual void renderTo( QPaintDevice* paintDevice ) = 0;
};

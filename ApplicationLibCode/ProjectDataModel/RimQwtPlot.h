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

#include "RimPlot.h"

#include "RiuQwtPlotWidget.h"

#include "RimPlotWindow.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QObject>
#include <QPointer>

class RiuQwtPlotWidget;
class RimPlotCurve;
class QwtPlotCurve;
class QwtPlotItem;

class QPaintDevice;
class QWheelEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimQwtPlot : public RimPlot
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimQwtPlot();
    ~RimQwtPlot() override;

    QWidget* createPlotWidget( QWidget* parent = nullptr );

    // Pure virtual interface methods
    virtual void updateZoomInParentPlot()   = 0;
    virtual void updateZoomFromParentPlot() = 0;

    //    virtual caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const = 0;

    virtual RiuQwtPlotWidget* viewer() = 0;

    RiuPlotWidget* plotWidget() override;

protected:
    static void attachPlotWidgetSignals( RimQwtPlot* plot, RiuQwtPlotWidget* plotWidget );
    QWidget*    createViewWidget( QWidget* parent = nullptr ) override;

    // void updateFonts() override;

private:
    // void doRenderWindowContent( QPaintDevice* paintDevice ) override;

private slots:
    void         onPlotSelected( bool toggle );
    virtual void onAxisSelected( int axis, bool toggle ) {}
    virtual void onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex );
    void         onViewerDestroyed();
    void         onKeyPressEvent( QKeyEvent* event );
    void         onWheelEvent( QWheelEvent* event );
};

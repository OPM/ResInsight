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

#include "RiaDefines.h"

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
class RimPlot : public QObject, public RimPlotWindow
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    enum RowOrColSpan
    {
        UNLIMITED = -1,
        ONE       = 1,
        TWO       = 2,
        THREE     = 3,
        FOUR      = 4,
        FIVE      = 5,
        SIX       = 6
    };
    using RowOrColSpanEnum = caf::AppEnum<RowOrColSpan>;

public:
    RimPlot();
    virtual ~RimPlot();

    QWidget*     createPlotWidget( QWidget* parent = nullptr );
    RowOrColSpan rowSpan() const;
    RowOrColSpan colSpan() const;
    void         setRowSpan( RowOrColSpan rowSpan );
    void         setColSpan( RowOrColSpan colSpan );
    void         removeFromMdiAreaAndCollection();
    void         updateAfterInsertingIntoMultiPlot();

    // Pure virtual interface methods
    virtual RiuQwtPlotWidget* viewer() = 0;

    virtual void setAutoScaleXEnabled( bool enabled ) = 0;
    virtual void setAutoScaleYEnabled( bool enabled ) = 0;
    virtual void updateAxes()                         = 0;

    virtual void updateLegend()      = 0;
    virtual void updateZoomInQwt()   = 0;
    virtual void updateZoomFromQwt() = 0;

    virtual QString asciiDataForPlotExport() const = 0;

    virtual void            reattachAllCurves()                                          = 0;
    virtual void            detachAllCurves()                                            = 0;
    virtual caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const = 0;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    static void attachPlotWidgetSignals( RimPlot* plot, RiuQwtPlotWidget* plotWidget );
    QWidget*    createViewWidget( QWidget* parent = nullptr ) final;

    void updateFonts() override;

private:
    virtual void              doRenderWindowContent( QPaintDevice* paintDevice ) override;
    virtual void              handleKeyPressEvent( QKeyEvent* event ) {}
    virtual void              handleWheelEvent( QWheelEvent* event ) {}
    virtual RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* parent ) = 0;

private slots:
    void         onPlotSelected( bool toggle );
    virtual void onAxisSelected( int axis, bool toggle ) {}
    virtual void onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex );
    void         onViewerDestroyed();
    void         onKeyPressEvent( QKeyEvent* event );
    void         onWheelEvent( QWheelEvent* event );

protected:
    caf::PdmField<RowOrColSpanEnum> m_rowSpan;
    caf::PdmField<RowOrColSpanEnum> m_colSpan;
};

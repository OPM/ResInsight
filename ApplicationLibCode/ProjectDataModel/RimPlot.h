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
    ~RimPlot() override;

    QWidget*     createPlotWidget( QWidget* parent = nullptr );
    RowOrColSpan rowSpan() const;
    RowOrColSpan colSpan() const;
    void         setRowSpan( RowOrColSpan rowSpan );
    void         setColSpan( RowOrColSpan colSpan );
    void         removeFromMdiAreaAndCollection();
    void         updateAfterInsertingIntoMultiPlot();

    // Pure virtual interface methods
    virtual void setAutoScaleXEnabled( bool enabled ) = 0;
    virtual void setAutoScaleYEnabled( bool enabled ) = 0;
    virtual void updateAxes()                         = 0;

    virtual void updateLegend() = 0;

    virtual QString asciiDataForPlotExport() const = 0;

    virtual void reattachAllCurves() = 0;
    virtual void detachAllCurves()   = 0;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    template <typename PlotWindowType = RimPlotWindow>
    bool isSubPlot() const
    {
        PlotWindowType* parentPlotWindow = nullptr;
        firstAncestorOfType( parentPlotWindow );
        return parentPlotWindow != nullptr;
    }

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    //    virtual QWidget* createViewWidget( QWidget* parent = nullptr ) = 0;

    void updateFonts() override = 0;

    void doRenderWindowContent( QPaintDevice* paintDevice ) override = 0;

    virtual void handleKeyPressEvent( QKeyEvent* event ) {}
    virtual void handleWheelEvent( QWheelEvent* event ) {}

private slots:
    void onPlotSelected( bool toggle );
    void onViewerDestroyed();
    void onKeyPressEvent( QKeyEvent* event );
    void onWheelEvent( QWheelEvent* event );

protected:
    caf::PdmField<RowOrColSpanEnum> m_rowSpan;
    caf::PdmField<RowOrColSpanEnum> m_colSpan;
};

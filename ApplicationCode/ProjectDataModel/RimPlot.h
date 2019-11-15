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

#include <QPointer>

class RiuQwtPlotWidget;
class RimPlotCurve;
class QwtPlotCurve;

class RimPlot : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum RowOrColSpan
    {
        UNLIMITED = -1,
        ONE       = 1,
        TWO       = 2,
        THREE     = 3,
        FOUR      = 4,
        FIVE      = 5
    };
    using RowOrColSpanEnum = caf::AppEnum<RowOrColSpan>;

public:
    RimPlot();
    virtual ~RimPlot() = default;

    // Real implementations
    void         createPlotWidget();
    RowOrColSpan rowSpan() const;
    RowOrColSpan colSpan() const;
    void         setRowSpan( RowOrColSpan rowSpan );
    void         setColSpan( RowOrColSpan colSpan );

    // Pure virtual interface methods
    virtual RiuQwtPlotWidget* viewer() = 0;

    virtual void setAutoScaleXEnabled( bool enabled ) = 0;
    virtual void setAutoScaleYEnabled( bool enabled ) = 0;
    virtual void updateAxes()                         = 0;

    virtual void updateZoomInQwt()   = 0;
    virtual void updateZoomFromQwt() = 0;

    virtual QString asciiDataForPlotExport() const = 0;

    virtual void detachAllCurves() = 0;

    virtual caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const = 0;

    virtual void onAxisSelected( int axis, bool toggle ) = 0;

    // TODO: Refactor
    virtual void removeFromMdiAreaAndCollection() {}
    virtual void updateAfterInsertingIntoMultiPlot() {}

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

private:
    virtual void onRowOrColSpanChange() {}

protected:
    caf::PdmField<RowOrColSpanEnum> m_rowSpan;
    caf::PdmField<RowOrColSpanEnum> m_colSpan;
};

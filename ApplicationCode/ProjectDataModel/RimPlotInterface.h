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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QPointer>

class RiuQwtPlotWidget;
class RimPlotCurve;
class QwtPlotCurve;

class RimPlotInterface
{
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
    typedef caf::AppEnum<RowOrColSpan> RowOrColSpanEnum;

public:
    RimPlotInterface()          = default;
    virtual ~RimPlotInterface() = default;

    bool isStandalonePlot() const;

    virtual RiuQwtPlotWidget* viewer()                   = 0;
    virtual bool              isChecked() const          = 0;
    virtual void              setChecked( bool checked ) = 0;

    virtual QString description() const = 0;

    virtual int rowSpan() const
    {
        return 1;
    }
    virtual int colSpan() const
    {
        return 1;
    }
    virtual void setRowSpan( RowOrColSpan rowSpan ) {}
    virtual void setColSpan( RowOrColSpan colSpan ) {}

    virtual bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const = 0;
    virtual bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                int                         oldFontSize,
                                int                         fontSize,
                                bool                        forceChange = false )                                                    = 0;

    virtual void setAutoScaleXEnabled( bool enabled ) = 0;
    virtual void setAutoScaleYEnabled( bool enabled ) = 0;
    virtual void updateAxes()                         = 0;

    virtual void updateZoomInQwt()   = 0;
    virtual void updateZoomFromQwt() = 0;

    virtual QString asciiDataForPlotExport() const;

    virtual void createPlotWidget() = 0;
    virtual void detachAllCurves()  = 0;

    virtual caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const = 0;

    virtual void loadDataAndUpdate() = 0;

    virtual void onAxisSelected( int axis, bool toggle ) {}

    virtual void removeFromMdiAreaAndCollection() {}
    virtual void updateAfterInsertingIntoMultiPlot() {}

protected:
    void         updatePlotWindowLayout();
    virtual void onRowOrColSpanChange() {}
};

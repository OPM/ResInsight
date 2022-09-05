/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RiaPlotDefines.h"

#include "RiuPlotAxis.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimPlotAxisAnnotation;

class RimPlotAxisPropertiesInterface : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum AxisTitlePositionType
    {
        AXIS_TITLE_CENTER,
        AXIS_TITLE_END
    };

    enum class LegendTickmarkCount
    {
        TICKMARK_VERY_FEW,
        TICKMARK_FEW,
        TICKMARK_DEFAULT,
        TICKMARK_MANY,
    };
    using LegendTickmarkCountEnum = caf::AppEnum<LegendTickmarkCount>;

    caf::Signal<> settingsChanged;

public:
    RimPlotAxisPropertiesInterface();

    virtual std::vector<RimPlotAxisAnnotation*> annotations() const                                   = 0;
    virtual void                                appendAnnotation( RimPlotAxisAnnotation* annotation ) = 0;
    virtual void                                removeAllAnnotations()                                = 0;
    virtual RiuPlotAxis                         plotAxisType() const                                  = 0;

    virtual double visibleRangeMin() const = 0;
    virtual double visibleRangeMax() const = 0;

    virtual void setVisibleRangeMin( double value ) = 0;
    virtual void setVisibleRangeMax( double value ) = 0;

    virtual bool isAutoZoom() const                 = 0;
    virtual void setAutoZoom( bool enableAutoZoom ) = 0;

    virtual bool isActive() const = 0;

    virtual const QString objectName() const    = 0;
    virtual const QString axisTitleText() const = 0;

    virtual bool isAxisInverted() const;

    virtual bool isLogarithmicScaleEnabled() const;

    virtual LegendTickmarkCount majorTickmarkCount() const                         = 0;
    virtual void                setMajorTickmarkCount( LegendTickmarkCount count ) = 0;

    static int tickmarkCountFromEnum( LegendTickmarkCount count );

public:
    virtual AxisTitlePositionType titlePosition() const  = 0;
    virtual int                   titleFontSize() const  = 0;
    virtual int                   valuesFontSize() const = 0;
};

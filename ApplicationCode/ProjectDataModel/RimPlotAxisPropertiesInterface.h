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

#include "cafAppEnum.h"

class RimPlotAxisPropertiesInterface
{
public:
    enum AxisTitlePositionType
    {
        AXIS_TITLE_CENTER,
        AXIS_TITLE_END
    };

    enum class LegendTickmarkCount
    {
        TICKMARK_FEW,
        TICKMARK_DEFAULT,
        TICKMARK_MANY,
    };
    using LegendTickmarkCountEnum = caf::AppEnum<LegendTickmarkCount>;

public:
    virtual AxisTitlePositionType titlePosition() const  = 0;
    virtual int                   titleFontSize() const  = 0;
    virtual int                   valuesFontSize() const = 0;
};

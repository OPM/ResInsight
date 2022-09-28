/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimWellLogCurve.h"

//==================================================================================================
///
///
//==================================================================================================
class RimRftTopologyCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimRftTopologyCurve();

    QString wellName() const override;
    QString wellLogChannelUiName() const override;
    QString wellLogChannelUnits() const override;

protected:
    QString createCurveAutoName() override;

    void onLoadDataAndUpdate( bool updateParentPlot ) override;
};

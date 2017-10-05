/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RifEclipseRftAddress.h"

#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimEclipseResultCase;
class RifReaderEclipseRft;
class RiuLineSegmentQwtPlotCurve;
//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogRftCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogRftCurve();
    virtual ~RimWellLogRftCurve();

    virtual QString                 wellName() const override;
    virtual QString                 wellLogChannelName() const override;

    std::vector<double>             xValues();
    std::vector<double>             depthValues();

    int currentTimeStep() const;

    void setEclipseResultCase(RimEclipseResultCase* eclipseResultCase);
    RimEclipseResultCase* eclipseResultCase() const;

protected:
    // Overrides from RimWellLogPlotCurve
    virtual QString createCurveAutoName() override;
    virtual void onLoadDataAndUpdate(bool updateParentPlot) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);

private:
    RifReaderEclipseRft* rftReader() const;

private:
    RiuLineSegmentQwtPlotCurve*                     m_qwtPlotCurve;

    caf::PdmPtrField<RimEclipseResultCase*>         m_eclipseResultCase;
    caf::PdmField<time_t>                           m_timeStep;
    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<QString>                          m_wellLogChannelName;
};



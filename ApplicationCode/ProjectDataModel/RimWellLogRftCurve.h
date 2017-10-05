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

#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cvfObject.h"

class RifReaderEclipseRft;
class RimEclipseResultCase;
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

    void                            setEclipseResultCase(RimEclipseResultCase* eclipseResultCase);
    RimEclipseResultCase*           eclipseResultCase() const;

    void                            setRftAddress(RifEclipseRftAddress address);

protected:
    // Overrides from RimWellLogPlotCurve
    virtual QString createCurveAutoName() override;
    virtual void    onLoadDataAndUpdate(bool updateParentPlot) override;

    // Pdm overrrides
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    RifReaderEclipseRft* rftReader() const;

    std::vector<double>  xValues() const;
    std::vector<double>  depthValues() const;

private:
    RiuLineSegmentQwtPlotCurve*                     m_qwtPlotCurve;

    caf::PdmPtrField<RimEclipseResultCase*>         m_eclipseResultCase;
    caf::PdmField<QDateTime>                        m_timeStep;
    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<QString>                          m_wellLogChannelName;
};



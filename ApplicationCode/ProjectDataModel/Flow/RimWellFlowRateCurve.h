/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

class RimEclipseResultCase;
class RimWellAllocationPlot;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellFlowRateCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellFlowRateCurve();
    ~RimWellFlowRateCurve() override;
    
    void setFlowValuesPrDepthValue(const QString& curveName , const std::vector<double>& depthValues, const std::vector<double>& flowRates);
    void updateStackedPlotData();

    QString wellName() const override;
    QString wellLogChannelName() const override;

    void            setGroupId(int groupId);
    int             groupId() const;

    void            setDoFillCurve(bool doFill);

protected:
    QString createCurveAutoName() override;
    void onLoadDataAndUpdate(bool updateParentPlot) override;
    void updateCurveAppearance() override;

    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    bool isUsingConnectionNumberDepthType() const;
    RimWellAllocationPlot* wellAllocationPlot() const;

    QString m_curveAutoName;

    int     m_groupId;
    bool    m_doFillCurve;
};


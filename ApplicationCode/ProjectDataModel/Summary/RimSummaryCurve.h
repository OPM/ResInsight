/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

#include "RiaDefines.h"
#include "RifEclipseSummaryAddressQMetaType.h"
#include "RimPlotCurve.h"

#include "cafAppEnum.h"

class RifSummaryReaderInterface;
class RimSummaryCase;
class RimSummaryFilter;
class RiuLineSegmentQwtPlotCurve;
class RimSummaryCurveAutoName;
class RimSummaryAddress;


//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurve();
    virtual ~RimSummaryCurve();

    void                                    setAsCrossPlotCurve();

    void                                    setSummaryCase(RimSummaryCase* sumCase);
    RimSummaryCase*                         summaryCase() const; 

    RifEclipseSummaryAddress                summaryAddress();
    void                                    setSummaryAddress(const RifEclipseSummaryAddress& address);
    std::string                             unitName();

    std::vector<double>                     yValues() const;
    const std::vector<time_t>&              timeSteps() const;

    void                                    setYAxis(RiaDefines::PlotAxis plotAxis);
    RiaDefines::PlotAxis                    yAxis() const;
    void                                    updateQwtPlotAxis();

    void                                    applyCurveAutoNameSettings(const RimSummaryCurveAutoName& autoNameSettings);

protected:
    // RimPlotCurve overrides

    virtual QString                         createCurveAutoName() override;
    virtual void                            updateZoomInParentPlot() override;
    virtual void                            onLoadDataAndUpdate(bool updateParentPlot) override;

private:
    RifSummaryReaderInterface*              summaryReader() const;
    bool                                    curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values) const;

    void                                    calculateCurveInterpolationFromAddress();

    // Overridden PDM methods
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    static void                             appendOptionItemsForSummaryAddresses(QList<caf::PdmOptionItemInfo>* options,
                                                                                 RimSummaryCase* summaryCase,
                                                                                 RimSummaryFilter* summaryFilter);

private:
    // Y values
    caf::PdmPtrField<RimSummaryCase*>       m_yValuesSummaryCase;
    caf::PdmChildField<RimSummaryAddress*>  m_yValuesCurveVariable;
    caf::PdmField<QString>                  m_yValuesSelectedVariableDisplayField;
    caf::PdmChildField<RimSummaryFilter*>   m_yValuesSummaryFilter;
    caf::PdmField<RifEclipseSummaryAddress> m_yValuesUiFilterResultSelection;

    // X values
    caf::PdmPtrField<RimSummaryCase*>       m_xValuesSummaryCase;
    caf::PdmChildField<RimSummaryAddress*>  m_xValuesCurveVariable;
    caf::PdmField<QString>                  m_xValuesSelectedVariableDisplayField;
    caf::PdmChildField<RimSummaryFilter*>   m_xValuesSummaryFilter;
    caf::PdmField<RifEclipseSummaryAddress> m_xValuesUiFilterResultSelection;

    caf::PdmField<bool>                     m_isCrossPlot;

    caf::PdmChildField<RimSummaryCurveAutoName*>        m_curveNameConfig;
    caf::PdmField<caf::AppEnum< RiaDefines::PlotAxis>>  m_plotAxis;
};

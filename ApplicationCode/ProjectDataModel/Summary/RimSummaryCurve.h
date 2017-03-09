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

#include "RifEclipseSummaryAddress.h"
#include "RimDefines.h"
#include "RimPlotCurve.h"

#include "cafAppEnum.h"


class RifReaderEclipseSummary;
class RimSummaryCase;
class RimSummaryFilter;
class RiuLineSegmentQwtPlotCurve;
class RimSummaryCurveAutoName;

class RimSummaryAddress: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryAddress();;
    virtual ~RimSummaryAddress();

    void setAddress(const RifEclipseSummaryAddress& addr);
    RifEclipseSummaryAddress address();

private:

    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory> >
        m_category;
    caf::PdmField<QString>                  m_quantityName;
    caf::PdmField<int>                      m_regionNumber;
    caf::PdmField<int>                      m_regionNumber2;
    caf::PdmField<QString>                  m_wellGroupName;
    caf::PdmField<QString>                  m_wellName;
    caf::PdmField<int>                      m_wellSegmentNumber;
    caf::PdmField<QString>                  m_lgrName;
    caf::PdmField<int>                      m_cellI;
    caf::PdmField<int>                      m_cellJ;
    caf::PdmField<int>                      m_cellK;

};

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

    void                                    setSummaryCase(RimSummaryCase* sumCase);
    RimSummaryCase*                         summaryCase(); 

    RifEclipseSummaryAddress                summaryAddress();
    void                                    setSummaryAddress(const RifEclipseSummaryAddress& address);
    std::string                             unitName();

    std::vector<double>                     yValues() const;
    const std::vector<time_t>&              timeSteps() const;

    void                                    setYAxis(RimDefines::PlotAxis plotAxis);
    RimDefines::PlotAxis                    yAxis() const;
    void                                    updateQwtPlotAxis();

    void                                    applyCurveAutoNameSettings(const RimSummaryCurveAutoName& autoNameSettings);

protected:
    // RimPlotCurve overrides

    virtual QString                         createCurveAutoName() override;
    virtual void                            updateZoomInParentPlot() override;
    virtual void                            onLoadDataAndUpdate() override;

private:
    RifReaderEclipseSummary*                summaryReader() const;
    bool                                    curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values) const;

    // Overridden PDM methods
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    // Fields
    caf::PdmPtrField<RimSummaryCase*>       m_summaryCase;
    caf::PdmChildField<RimSummaryAddress*>  m_curveVariable;
    caf::PdmField<QString>                  m_selectedVariableDisplayField;

    caf::PdmChildField<RimSummaryCurveAutoName*>   m_curveNameConfig;

    caf::PdmField< caf::AppEnum< RimDefines::PlotAxis > > m_plotAxis;

    // Filter fields
    caf::PdmChildField<RimSummaryFilter*>   m_summaryFilter;
    caf::PdmField<int>                      m_uiFilterResultSelection;
};

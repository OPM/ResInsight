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

#include "RimPlotCurve.h"
#include "RifEclipseSummaryAddress.h"
#include "cafAppEnum.h"

class RimSummaryCase;
class RifReaderEclipseSummary;
class RiuLineSegmentQwtPlotCurve;

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
    void                                    setVariable(QString varName);

    enum SummaryFilterType 
    {
        SUM_FILTER_ANY,
        SUM_FILTER_FIELD,
        SUM_FILTER_AQUIFER,
        SUM_FILTER_NETWORK,
        SUM_FILTER_MISC,
        SUM_FILTER_REGION,
        SUM_FILTER_REGION_2_REGION,
        SUM_FILTER_WELL_GROUP,
        SUM_FILTER_WELL,
        SUM_FILTER_WELL_COMPLETION,
        SUM_FILTER_WELL_COMPLETION_LGR,
        SUM_FILTER_WELL_LGR,
        SUM_FILTER_WELL_SEGMENT,
        SUM_FILTER_WELL_SEGMENT_RIVER,
        SUM_FILTER_BLOCK,
        SUM_FILTER_BLOCK_LGR,
    };

protected:
    // RimPlotCurve overrides

    virtual QString                         createCurveAutoName() override;
    virtual void                            zoomAllParentPlot()   override;
    virtual void                            onLoadDataAndUpdate() override;

private:
    RifReaderEclipseSummary*                summaryReader();
    void                                    curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values);

    // Overridden PDM methods
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    // Fields
    caf::PdmPtrField<RimSummaryCase*>       m_summaryCase;
    caf::PdmField<QString>                  m_variableName; // Obsolete


    // Filter fields
    caf::PdmField<caf::AppEnum<SummaryFilterType> >
                                            m_filterType;
    caf::PdmField<QString>                  m_filterQuantityName;
    caf::PdmField<QString>                  m_regionNumberFilter;
    caf::PdmField<QString>                  m_regionNumber2Filter;
    caf::PdmField<QString>                  m_wellGroupNameFilter;
    caf::PdmField<QString>                  m_wellNameFilter;
    caf::PdmField<QString>                  m_wellSegmentNumberFilter;
    caf::PdmField<QString>                  m_lgrNameFilter;
    caf::PdmField<QString>                  m_cellIJKFilter;

    caf::PdmField<int>                      m_uiFilterResultSelection;
    caf::PdmChildField<RimSummaryAddress*>  m_curveVariable;



};

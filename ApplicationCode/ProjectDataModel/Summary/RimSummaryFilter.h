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

#include "RifEclipseSummaryAddress.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimSummaryFilter: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    enum SummaryFilterType
    {
        SUM_FILTER_VAR_STRING,
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
        SUM_FILTER_BLOCK,
        SUM_FILTER_BLOCK_LGR,
    };

    RimSummaryFilter();
    virtual ~RimSummaryFilter();

    void            updateFromAddress(const RifEclipseSummaryAddress& address);
    void            setCompleteVarStringFilter(const QString& stringFilter);

    bool            isIncludedByFilter(const RifEclipseSummaryAddress& addr) const;

    virtual void    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    friend class RimSummaryCurve;
    friend class RimSummaryCurveFilter;

    static bool     isSumVarTypeMatchingFilterType(SummaryFilterType sumFilterType, RifEclipseSummaryAddress::SummaryVarCategory sumVarType);

    virtual void    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmField<caf::AppEnum<SummaryFilterType> >
                                            m_filterType;
    caf::PdmField<QString>                  m_completeVarStringFilter;

    caf::PdmField<QString>                  m_filterQuantityName;
    caf::PdmField<QString>                  m_regionNumberFilter;
    caf::PdmField<QString>                  m_regionNumber2Filter;
    caf::PdmField<QString>                  m_wellGroupNameFilter;
    caf::PdmField<QString>                  m_wellNameFilter;
    caf::PdmField<QString>                  m_wellSegmentNumberFilter;
    caf::PdmField<QString>                  m_lgrNameFilter;
    caf::PdmField<QString>                  m_cellIJKFilter;
};

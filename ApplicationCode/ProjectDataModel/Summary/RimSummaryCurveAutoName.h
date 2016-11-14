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


class RifEclipseSummaryAddress;

class RimSummaryCurveAutoName : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryCurveAutoName();;

    QString         curveName(const RifEclipseSummaryAddress& summaryAddress) const;

    void            applySettings(const RimSummaryCurveAutoName& other);

    virtual void    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

protected:
    friend class RimSummaryCurve;
    friend class RimSummaryCurveFilter;

    virtual void    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    void            appendWellName(std::string& text, const RifEclipseSummaryAddress& summaryAddress) const;
    void            appendLgrName(std::string& text, const RifEclipseSummaryAddress& summaryAddress) const;

private:
    caf::PdmField<bool> m_vectorName;
    caf::PdmField<bool> m_unit;
    caf::PdmField<bool> m_regionNumber;
    caf::PdmField<bool> m_wellGroupName;
    caf::PdmField<bool> m_wellName;
    caf::PdmField<bool> m_wellSegmentNumber;
    caf::PdmField<bool> m_lgrName;
    caf::PdmField<bool> m_completion;

    caf::PdmField<bool> m_caseName;

    caf::PdmField<bool> m_showAdvancedProperties;
};


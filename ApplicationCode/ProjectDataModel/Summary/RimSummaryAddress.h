/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RifEclipseSummaryAddressQMetaType.h"
#include "RiaDefines.h"
#include "RimPlotCurve.h"

#include "cafAppEnum.h"

class RifSummaryReaderInterface;
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
    caf::PdmField<int>                      m_aquiferNumber;
    caf::PdmField<bool>                     m_isErrorResult;
};


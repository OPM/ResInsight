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

#include "RimSummaryAddress.h"

namespace caf
{

    template<>
    void caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::setUp()
    {
        addItem(RifEclipseSummaryAddress::SUMMARY_FIELD, "SUMMARY_FIELD", "Field");
        addItem(RifEclipseSummaryAddress::SUMMARY_AQUIFER, "SUMMARY_AQUIFER", "Aquifer");
        addItem(RifEclipseSummaryAddress::SUMMARY_NETWORK, "SUMMARY_NETWORK", "Network");
        addItem(RifEclipseSummaryAddress::SUMMARY_MISC, "SUMMARY_MISC", "Misc");
        addItem(RifEclipseSummaryAddress::SUMMARY_REGION, "SUMMARY_REGION", "Region");
        addItem(RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION, "SUMMARY_REGION_2_REGION", "Region-Region");
        addItem(RifEclipseSummaryAddress::SUMMARY_WELL_GROUP, "SUMMARY_WELL_GROUP", "Group");
        addItem(RifEclipseSummaryAddress::SUMMARY_WELL, "SUMMARY_WELL", "Well");
        addItem(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION, "SUMMARY_WELL_COMPLETION", "Completion");
        addItem(RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR, "SUMMARY_WELL_COMPLETION_LGR", "Lgr-Completion");
        addItem(RifEclipseSummaryAddress::SUMMARY_WELL_LGR, "SUMMARY_WELL_LGR", "Lgr-Well");
        addItem(RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT, "SUMMARY_SEGMENT", "Segment");
        addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK, "SUMMARY_BLOCK", "Block");
        addItem(RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR, "SUMMARY_BLOCK_LGR", "Lgr-Block");
        addItem(RifEclipseSummaryAddress::SUMMARY_CALCULATED, "SUMMARY_CALCULATED", "Calculated");
        addItem(RifEclipseSummaryAddress::SUMMARY_IMPORTED, "SUMMARY_IMPORTED", "Imported");
        addItem(RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS, "SUMMARY_ENSEMBLE_STATISTICS", "Ensemble Statistics");
        setDefault(RifEclipseSummaryAddress::SUMMARY_FIELD);
    }

}




CAF_PDM_SOURCE_INIT(RimSummaryAddress, "SummaryAddress");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::RimSummaryAddress()
{
    CAF_PDM_InitFieldNoDefault(&m_category,          "SummaryVarType",      "Type", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_quantityName,      "SummaryQuantityName", "Quantity", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionNumber,      "SummaryRegion",       "Region", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionNumber2,     "SummaryRegion2",      "Region2", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellGroupName,     "SummaryWellGroup",    "Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellName,          "SummaryWell",         "Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellSegmentNumber, "SummaryWellSegment",  "Well Segment", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_lgrName,           "SummaryLgr",          "Grid", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellI,             "SummaryCellI",        "I", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellJ,             "SummaryCellJ",        "J", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellK,             "SummaryCellK",        "K", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_aquiferNumber,     "SummaryAquifer",      "Aquifer", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_isErrorResult,     "IsErrorResult",       "Is Error Result", "", "", "");

    m_category = RifEclipseSummaryAddress::SUMMARY_INVALID;
    m_regionNumber = -1;
    m_regionNumber2 = -1;
    m_wellSegmentNumber = -1;
    m_cellI = -1;
    m_cellJ = -1;
    m_cellK = -1;
    m_aquiferNumber = -1;
    m_isErrorResult = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAddress::~RimSummaryAddress()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryAddress::setAddress(const RifEclipseSummaryAddress& addr)
{
    m_category                      = addr.category();
    m_quantityName                  = addr.quantityName().c_str();
    m_regionNumber                  = addr.regionNumber();
    m_regionNumber2                 = addr.regionNumber2();
    m_wellGroupName                 = addr.wellGroupName().c_str();
    m_wellName                      = addr.wellName().c_str();
    m_wellSegmentNumber             = addr.wellSegmentNumber();
    m_lgrName                       = addr.lgrName().c_str();
    m_aquiferNumber                 = addr.aquiferNumber();
    m_isErrorResult                 = addr.isErrorResult();

    m_cellI = addr.cellI(); m_cellJ = addr.cellJ(); m_cellK = addr.cellK();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryAddress::address()
{
    return RifEclipseSummaryAddress( m_category(),
                                     m_quantityName().toStdString(),
                                     m_regionNumber(),
                                     m_regionNumber2(),
                                     m_wellGroupName().toStdString(),
                                     m_wellName().toStdString(),
                                     m_wellSegmentNumber(),
                                     m_lgrName().toStdString(),
                                     m_cellI(), m_cellJ(), m_cellK(),
                                     m_aquiferNumber,
                                     m_isErrorResult);
}


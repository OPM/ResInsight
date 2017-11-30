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

#include "RimSummaryFilter.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"

namespace caf
{

template<>
void caf::AppEnum<RimSummaryFilter::SummaryFilterType>::setUp()
{
    addItem(RimSummaryFilter::SUM_FILTER_VAR_STRING, "SUM_FILTER_VAR_STRING", "All");
    addItem(RimSummaryFilter::SUM_FILTER_FIELD, "SUM_FILTER_FIELD", "Field");
    addItem(RimSummaryFilter::SUM_FILTER_WELL, "SUM_FILTER_WELL", "Well");
    addItem(RimSummaryFilter::SUM_FILTER_WELL_GROUP, "SUM_FILTER_WELL_GROUP", "Group");
    addItem(RimSummaryFilter::SUM_FILTER_WELL_COMPLETION, "SUM_FILTER_WELL_COMPLETION", "Completion");
    addItem(RimSummaryFilter::SUM_FILTER_WELL_SEGMENT, "SUM_FILTER_SEGMENT", "Segment");
    addItem(RimSummaryFilter::SUM_FILTER_BLOCK, "SUM_FILTER_BLOCK", "Block");
    addItem(RimSummaryFilter::SUM_FILTER_REGION, "SUM_FILTER_REGION", "Region");
    addItem(RimSummaryFilter::SUM_FILTER_REGION_2_REGION, "SUM_FILTER_REGION_2_REGION", "Region-Region");
    addItem(RimSummaryFilter::SUM_FILTER_WELL_LGR, "SUM_FILTER_WELL_LGR", "Lgr-Well");
    addItem(RimSummaryFilter::SUM_FILTER_WELL_COMPLETION_LGR, "SUM_FILTER_WELL_COMPLETION_LGR", "Lgr-Completion");
    addItem(RimSummaryFilter::SUM_FILTER_BLOCK_LGR, "SUM_FILTER_BLOCK_LGR", "Lgr-Block");
    addItem(RimSummaryFilter::SUM_FILTER_MISC, "SUM_FILTER_MISC", "Misc");
    addItem(RimSummaryFilter::SUM_FILTER_AQUIFER, "SUM_FILTER_AQUIFER", "Aquifer");
    addItem(RimSummaryFilter::SUM_FILTER_NETWORK, "SUM_FILTER_NETWORK", "Network");
    addItem(RimSummaryFilter::SUM_FILTER_ANY, "SUM_FILTER_ANY", "All (Advanced)");
    setDefault(RimSummaryFilter::SUM_FILTER_VAR_STRING);
}

}

CAF_PDM_SOURCE_INIT(RimSummaryFilter, "SummaryFilterSettings");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryFilter::RimSummaryFilter()
{
    CAF_PDM_InitObject("Summary Filter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_filterType, "SummaryFilterType", "Search", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_completeVarStringFilter, "SummaryCompleteVarStringFilter", "Filter", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_filterQuantityName, "SummaryVarQuantityFilter", "Vector name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionNumberFilter, "SummaryRegionNumberFilter", "Region number", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionNumber2Filter, "SummaryRegionNumber2Filter", "2. Region number", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellGroupNameFilter, "SummaryWellGroupNameFilter", "Group name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellNameFilter, "SummaryWellNameFilter", "Well name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellSegmentNumberFilter, "SummaryWellSegmentNumberFilter", "Segment number", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_lgrNameFilter, "SummaryLgrNameFilter", "Lgr name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_cellIJKFilter, "SummaryCellIJKFilter", "I, J, K", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryFilter::~RimSummaryFilter()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString cellIJKString(int cellI, int cellJ, int cellK)
{
    QString ijkString;
    if (cellI >= 0 && cellJ >= 0 && cellK >= 0)
    {
        ijkString = QString::number(cellI) + ", " + QString::number(cellJ) + ", " + QString::number(cellK);
    }

    return ijkString;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryFilter::updateFromAddress(const RifEclipseSummaryAddress& address)
{
    RifEclipseSummaryAddress::SummaryVarCategory category = address.category();

    m_filterQuantityName = QString::fromStdString(address.quantityName());

    switch (category)
    {
    case RifEclipseSummaryAddress::SUMMARY_INVALID:
        m_filterType = SUM_FILTER_VAR_STRING;
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_FIELD:
        m_filterType = SUM_FILTER_FIELD;
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_AQUIFER:
        m_filterType = SUM_FILTER_AQUIFER;
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_NETWORK:
        m_filterType = SUM_FILTER_NETWORK;
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_MISC:
        m_filterType = SUM_FILTER_MISC;
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_REGION:
        m_filterType        = SUM_FILTER_REGION;
        m_regionNumberFilter= QString("%1").arg(address.regionNumber());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        m_filterType          = SUM_FILTER_REGION_2_REGION;
        m_regionNumberFilter  = QString("%1").arg(address.regionNumber());
        m_regionNumber2Filter = QString("%1").arg(address.regionNumber2());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        m_filterType          = SUM_FILTER_WELL_GROUP;
        m_wellGroupNameFilter = QString::fromStdString(address.wellGroupName());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_WELL:
        m_filterType     = SUM_FILTER_WELL;
        m_wellNameFilter = QString::fromStdString(address.wellName());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        m_filterType     = SUM_FILTER_WELL_COMPLETION;
        m_wellNameFilter = QString::fromStdString(address.wellName());
        m_cellIJKFilter  = cellIJKString(address.cellI(), address.cellJ(), address.cellK());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        m_filterType    = SUM_FILTER_WELL_LGR;
        m_wellNameFilter= QString::fromStdString(address.wellName());
        m_lgrNameFilter = QString::fromStdString(address.lgrName());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        m_filterType     = SUM_FILTER_WELL_COMPLETION_LGR;
        m_wellNameFilter = QString::fromStdString(address.wellName());
        m_lgrNameFilter  = QString::fromStdString(address.lgrName());
        m_cellIJKFilter  = cellIJKString(address.cellI(), address.cellJ(), address.cellK());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        m_filterType              = SUM_FILTER_WELL_SEGMENT;
        m_wellNameFilter          = QString::fromStdString(address.wellName());
        m_wellSegmentNumberFilter = QString("%1").arg(address.wellSegmentNumber());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        m_filterType    = SUM_FILTER_BLOCK;
        m_cellIJKFilter = cellIJKString(address.cellI(), address.cellJ(), address.cellK());
        break;
    
    case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        m_filterType    = SUM_FILTER_BLOCK_LGR;
        m_lgrNameFilter = QString::fromStdString(address.lgrName());
        m_cellIJKFilter = cellIJKString(address.cellI(), address.cellJ(), address.cellK());
        break;

    default:
        break;
    }
}

bool isNumberMatch(QString numericalFilterString, int number)
{
    if(numericalFilterString.isEmpty()) return true;

    if (numericalFilterString.trimmed() == "*") 
    {
        if(number >= 0) return true;
        else return false;
    }

    // Todo: Ranges, and lists
    int filterNumber = numericalFilterString.toInt();
    return number == filterNumber;
}

bool isStringMatch(QString filterString, std::string value)
{
    if(filterString.isEmpty()) return true;
    if(filterString.trimmed() == "*")
    {
        if(!value.empty()) return true;
        else return false;
    }

    QRegExp searcher(filterString, Qt::CaseInsensitive, QRegExp::WildcardUnix);
    QString qstrValue = QString::fromStdString(value);
    return searcher.exactMatch(qstrValue);
}

bool isIJKMatch(QString filterString, int cellI, int cellJ, int cellK)
{
    if(filterString.isEmpty()) return true;
    if(filterString.trimmed() == "*")
    {
        if(cellI >= 0 && cellJ >= 0 && cellK >= 0) return true;
        else return false;
    }

     QString ijkString = cellIJKString(cellI, cellJ, cellK);

    // Todo: Ranges, and lists
    QRegExp searcher(filterString, Qt::CaseInsensitive, QRegExp::WildcardUnix);

    return searcher.exactMatch(ijkString);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryFilter::isIncludedByFilter(const RifEclipseSummaryAddress& addr) const
{
    if(!isSumVarTypeMatchingFilterType(m_filterType(), addr.category())) return false;

    if(m_filterType() == SUM_FILTER_VAR_STRING)
    {
        return isStringMatch(m_completeVarStringFilter(), addr.uiText());
    }

    if(!isStringMatch(m_filterQuantityName(), addr.quantityName())) return false;

    if(m_filterType() == SUM_FILTER_ANY)
    {
        return (isNumberMatch(m_regionNumberFilter(), addr.regionNumber())
                &&  isNumberMatch(m_regionNumber2Filter(), addr.regionNumber2())
                &&  isStringMatch(m_wellGroupNameFilter(), addr.wellGroupName())
                &&  isStringMatch(m_wellNameFilter(), addr.wellName())
                &&  isStringMatch(m_lgrNameFilter(), addr.lgrName())
                &&  isNumberMatch(m_wellSegmentNumberFilter(), addr.wellSegmentNumber())
                &&  isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK()));
    }

    switch(addr.category())
    {
        case RifEclipseSummaryAddress::SUMMARY_REGION:
        {
            return isNumberMatch(m_regionNumberFilter(), addr.regionNumber());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION:
        {
            return  isNumberMatch(m_regionNumberFilter(), addr.regionNumber())
                && isNumberMatch(m_regionNumber2Filter(), addr.regionNumber2());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP:
        {
            return  isStringMatch(m_wellGroupNameFilter(), addr.wellGroupName());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName())
                && isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName())
                && isStringMatch(m_lgrNameFilter(), addr.lgrName());
        }
        break;

        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName())
                && isStringMatch(m_lgrNameFilter(), addr.lgrName())
                && isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT:
        {
            return  isStringMatch(m_wellNameFilter(), addr.wellName())
                && isNumberMatch(m_wellSegmentNumberFilter(), addr.wellSegmentNumber());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK:
        {
            return  isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR:
        {
            return  isStringMatch(m_lgrNameFilter(), addr.lgrName())
                && isIJKMatch(m_cellIJKFilter(), addr.cellI(), addr.cellJ(), addr.cellK());
        }
        break;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryFilter::isSumVarTypeMatchingFilterType(SummaryFilterType sumFilterType, RifEclipseSummaryAddress::SummaryVarCategory sumVarType)
{
    if(sumVarType == RifEclipseSummaryAddress::SUMMARY_INVALID) return false;
    if(sumFilterType == SUM_FILTER_ANY || sumFilterType == SUM_FILTER_VAR_STRING) return true;

    switch(sumVarType)
    {
        case RifEclipseSummaryAddress::SUMMARY_FIELD: { return (sumFilterType == SUM_FILTER_FIELD); } break;
        case RifEclipseSummaryAddress::SUMMARY_AQUIFER: { return (sumFilterType == SUM_FILTER_AQUIFER);  } break;
        case RifEclipseSummaryAddress::SUMMARY_NETWORK: { return (sumFilterType == SUM_FILTER_NETWORK);  } break;
        case RifEclipseSummaryAddress::SUMMARY_MISC: { return (sumFilterType == SUM_FILTER_MISC);  } break;
        case RifEclipseSummaryAddress::SUMMARY_REGION: { return (sumFilterType == SUM_FILTER_REGION);  } break;
        case RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION: { return (sumFilterType == SUM_FILTER_REGION_2_REGION);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_GROUP: { return (sumFilterType == SUM_FILTER_WELL_GROUP);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL: { return (sumFilterType == SUM_FILTER_WELL);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION: { return (sumFilterType == SUM_FILTER_WELL_COMPLETION);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_LGR: { return (sumFilterType == SUM_FILTER_WELL_LGR);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION_LGR: { return (sumFilterType == SUM_FILTER_WELL_COMPLETION_LGR);  } break;
        case RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT: { return (sumFilterType == SUM_FILTER_WELL_SEGMENT);  } break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK: { return (sumFilterType == SUM_FILTER_BLOCK);  } break;
        case RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR: { return (sumFilterType == SUM_FILTER_BLOCK_LGR);  } break;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_filterType);

    caf::PdmUiGroup* curveVarFilterGroup = nullptr;

    if(m_filterType() == SUM_FILTER_VAR_STRING)
    {
        uiOrdering.add(&m_completeVarStringFilter);
    }
    else
    {
        caf::PdmUiOrdering* curveVarFilterGroup = &uiOrdering;//uiOrdering.addNewGroup("Search Options");

        curveVarFilterGroup->add(&m_filterQuantityName);

        switch(m_filterType())
        {
            case SUM_FILTER_ANY:
            {
                curveVarFilterGroup->add(&m_wellNameFilter);
                curveVarFilterGroup->add(&m_wellGroupNameFilter);
                curveVarFilterGroup->add(&m_regionNumberFilter);
                curveVarFilterGroup->add(&m_regionNumber2Filter);
                curveVarFilterGroup->add(&m_wellSegmentNumberFilter);
                curveVarFilterGroup->add(&m_lgrNameFilter);
                curveVarFilterGroup->add(&m_cellIJKFilter);
            }
            break;
            case SUM_FILTER_REGION:
            {
                curveVarFilterGroup->add(&m_regionNumberFilter);
            }
            break;
            case SUM_FILTER_REGION_2_REGION:
            {
                curveVarFilterGroup->add(&m_regionNumberFilter);
                curveVarFilterGroup->add(&m_regionNumber2Filter);

            }
            break;
            case SUM_FILTER_WELL_GROUP:
            {
                curveVarFilterGroup->add(&m_wellGroupNameFilter);

            }
            break;
            case SUM_FILTER_WELL:
            {
                curveVarFilterGroup->add(&m_wellNameFilter);

            }
            break;
            case SUM_FILTER_WELL_COMPLETION:
            {
                curveVarFilterGroup->add(&m_wellNameFilter);
                curveVarFilterGroup->add(&m_cellIJKFilter);

            }
            break;
            case SUM_FILTER_WELL_LGR:
            {
                curveVarFilterGroup->add(&m_wellNameFilter);
                curveVarFilterGroup->add(&m_lgrNameFilter);
            }
            break;
            case SUM_FILTER_WELL_COMPLETION_LGR:
            {
                curveVarFilterGroup->add(&m_wellNameFilter);
                curveVarFilterGroup->add(&m_lgrNameFilter);
                curveVarFilterGroup->add(&m_cellIJKFilter);
            }
            break;
            case SUM_FILTER_WELL_SEGMENT:
            {
                curveVarFilterGroup->add(&m_wellNameFilter);
                curveVarFilterGroup->add(&m_wellSegmentNumberFilter);
            }
            break;
            case SUM_FILTER_BLOCK:
            {
                curveVarFilterGroup->add(&m_cellIJKFilter);
            }
            break;
            case SUM_FILTER_BLOCK_LGR:
            {
                curveVarFilterGroup->add(&m_lgrNameFilter);
                curveVarFilterGroup->add(&m_cellIJKFilter);
            }
            break;

        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    caf::PdmObject* parent = dynamic_cast<caf::PdmObject*>(this->parentField()->ownerObject());

    if (parent)
    {
        parent->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryFilter::setCompleteVarStringFilter(const QString& stringFilter)
{
    m_filterType = SUM_FILTER_VAR_STRING;
    m_completeVarStringFilter = stringFilter;
}


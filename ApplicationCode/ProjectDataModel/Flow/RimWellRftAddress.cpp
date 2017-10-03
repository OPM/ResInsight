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

#include "RimWellRftAddress.h"
#include "cafAppEnum.h"
#include <QString>
#include <QTextStream>

namespace caf
{

    template<>
    void caf::AppEnum<RftSourceType>::setUp()
    {
        addItem(RftSourceType::RFT, "RFT", "RFT Cases");
        addItem(RftSourceType::GRID, "GRID", "Grid Cases");
        addItem(RftSourceType::OBSERVED, "OBSERVED", "Observed Data");
        setDefault(RftSourceType::NONE);
    }
}

RimWellRftAddress::RimWellRftAddress(RftSourceType sourceType , int caseId)
{
    m_sourceType = sourceType;
    m_caseId = caseId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RftSourceType RimWellRftAddress::sourceType() const
{
    return m_sourceType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellRftAddress::caseId() const
{
    return m_caseId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellRftAddress::uiText() const
{
    return m_caseId >= 0 ?
        QString("%1 %2").arg(sourceTypeUiText(m_sourceType), QString::number(m_caseId)) :
        QString("%1").arg(sourceTypeUiText(m_sourceType));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellRftAddress::sourceTypeUiText(RftSourceType sourceType)
{
    switch (sourceType)
    {
    case RftSourceType::RFT:       return QString("RFT Cases");
    case RftSourceType::GRID:      return QString("Grid Cases");
    case RftSourceType::OBSERVED:  return QString("Observed Data");
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator==(const RimWellRftAddress& addr1, const RimWellRftAddress& addr2)
{
    return addr1.sourceType() == addr2.sourceType() && addr1.caseId() == addr2.caseId();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator << (QTextStream& str, const RimWellRftAddress& addr)
{
    str << RimWellRftAddress::sourceTypeUiText(addr.sourceType()) << " " << addr.caseId();
    return str;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator >> (QTextStream& str, RimWellRftAddress& source)
{
    QString sourceTypeString;
    int caseId;

    str >> sourceTypeString;
    str >> caseId;

    if (QString::compare(sourceTypeString, RimWellRftAddress::sourceTypeUiText(RftSourceType::RFT)) == 0)
    {
        source.m_sourceType = RftSourceType::RFT;
    }
    else if (QString::compare(sourceTypeString, RimWellRftAddress::sourceTypeUiText(RftSourceType::GRID)) == 0)
    {
        source.m_sourceType = RftSourceType::GRID;
    }
    else if (QString::compare(sourceTypeString, RimWellRftAddress::sourceTypeUiText(RftSourceType::OBSERVED)) == 0)
    {
        source.m_sourceType = RftSourceType::OBSERVED;
    }
    source.m_caseId = caseId;
    return str;
}

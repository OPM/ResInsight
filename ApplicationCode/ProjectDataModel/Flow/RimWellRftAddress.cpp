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
#include "RimEclipseCase.h"
#include "RimWellLogFile.h"

#include "cafAppEnum.h"
#include "cvfAssert.h"
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellRftAddress::RimWellRftAddress() : m_sourceType(RftSourceType::NONE)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellRftAddress::RimWellRftAddress(RftSourceType sourceType, RimEclipseCase* eclCase)
{
    CVF_ASSERT(sourceType == RftSourceType::RFT || sourceType == RftSourceType::GRID);
    CVF_ASSERT(eclCase != nullptr);

    m_sourceType = sourceType;
    m_eclCase = eclCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellRftAddress::RimWellRftAddress(RftSourceType sourceType, RimWellLogFile* wellLogFile)
{
    CVF_ASSERT(sourceType == RftSourceType::OBSERVED);

    m_sourceType = sourceType;
    m_wellLogFile = wellLogFile;
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
RimEclipseCase* RimWellRftAddress::eclCase() const
{
    return m_eclCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RimWellRftAddress::wellLogFile() const
{
    return m_wellLogFile;
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
    return addr1.sourceType() == addr2.sourceType() 
        && addr1.eclCase() == addr2.eclCase() 
        && addr1.wellLogFile() == addr2.wellLogFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator << (QTextStream& str, const RimWellRftAddress& addr)
{
    // Not implemented
    CVF_ASSERT(false);
    return str;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator >> (QTextStream& str, RimWellRftAddress& source)
{
    // Not implemented
    CVF_ASSERT(false);
    return str;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator<(const RimWellRftAddress& addr1, const RimWellRftAddress& addr2)
{
    return (addr1.m_sourceType < addr2.m_sourceType) ||
        (addr1.m_sourceType == addr2.m_sourceType && 
         addr1.eclCase() != nullptr && addr2.eclCase() != nullptr ? addr1.eclCase()->caseId() < addr2.eclCase()->caseId() :
         addr1.wellLogFile() != nullptr && addr2.wellLogFile() != nullptr ?  addr1.wellLogFile()->fileName() < addr2.wellLogFile()->fileName() :
         addr1.wellLogFile() < addr2.wellLogFile());
}

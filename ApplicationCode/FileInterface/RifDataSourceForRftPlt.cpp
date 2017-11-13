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

#include "RifDataSourceForRftPlt.h"
#include "RimEclipseCase.h"
#include "RimWellLogFile.h"

#include "cafAppEnum.h"
#include "cvfAssert.h"
#include <QString>
#include <QTextStream>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt() : m_sourceType(SourceType::NONE)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt(SourceType sourceType, RimEclipseCase* eclCase)
{
    CVF_ASSERT(sourceType == SourceType::RFT || sourceType == SourceType::GRID);
    CVF_ASSERT(eclCase != nullptr);

    m_sourceType = sourceType;
    m_eclCase = eclCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt(SourceType sourceType, RimWellLogFile* wellLogFile)
{
    CVF_ASSERT(sourceType == SourceType::OBSERVED);

    m_sourceType = sourceType;
    m_wellLogFile = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::SourceType RifDataSourceForRftPlt::sourceType() const
{
    return m_sourceType;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RifDataSourceForRftPlt::eclCase() const
{
    return m_eclCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RifDataSourceForRftPlt::wellLogFile() const
{
    return m_wellLogFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifDataSourceForRftPlt::sourceTypeUiText(SourceType sourceType)
{
    switch (sourceType)
    {
    case SourceType::RFT:       return QString("RFT Cases");
    case SourceType::GRID:      return QString("Grid Cases");
    case SourceType::OBSERVED:  return QString("Observed Data");
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator==(const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2)
{
    return addr1.sourceType() == addr2.sourceType() 
        && addr1.eclCase() == addr2.eclCase() 
        && addr1.wellLogFile() == addr2.wellLogFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator << (QTextStream& str, const RifDataSourceForRftPlt& addr)
{
    // Not implemented
    CVF_ASSERT(false);
    return str;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStream& operator >> (QTextStream& str, RifDataSourceForRftPlt& source)
{
    // Not implemented
    CVF_ASSERT(false);
    return str;
}

//--------------------------------------------------------------------------------------------------
/// This sort order controls the plot order in PLT plot. (Layer-wise)
/// Observed data is supposed to be the bottom layers (first)
//--------------------------------------------------------------------------------------------------
bool operator<(const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2)
{
    if ( addr1.m_sourceType != addr2.m_sourceType )
    {
        return addr1.m_sourceType < addr2.m_sourceType;
    }

    if ( addr1.m_sourceType == RifDataSourceForRftPlt::NONE ) return false; // 
    
    if (addr1.m_sourceType == RifDataSourceForRftPlt::OBSERVED) 
    { 
        if(addr1.wellLogFile() && addr2.wellLogFile()) 
        {
            return addr1.wellLogFile()->fileName() < addr2.wellLogFile()->fileName();
        }
        else
        {
            return addr1.wellLogFile() < addr2.wellLogFile();
        }
    }
    else 
    {
        if ( addr1.eclCase() && addr2.eclCase() )
        {
            return addr1.eclCase()->caseId() < addr2.eclCase()->caseId();
        }
        else
        {
            return addr1.eclCase() < addr2.eclCase();
        }
    }
}
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator<(const RifWellRftAddress& addr1, const RifWellRftAddress& addr2)
{
    return (addr1.m_sourceType < addr2.m_sourceType) ||
        (addr1.m_sourceType == addr2.m_sourceType && 
         addr1.eclCase() != nullptr && addr2.eclCase() != nullptr ? addr1.eclCase()->caseId() < addr2.eclCase()->caseId() :
         addr1.wellLogFile() != nullptr && addr2.wellLogFile() != nullptr ?  addr1.wellLogFile()->fileName() < addr2.wellLogFile()->fileName() :
         addr1.wellLogFile() < addr2.wellLogFile());
}
#endif
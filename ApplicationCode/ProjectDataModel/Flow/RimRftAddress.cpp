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

#include "RimRftAddress.h"
#include "RimEclipseCase.h"
#include "RimWellLogFile.h"

#include "cafAppEnum.h"
#include "cvfAssert.h"
#include <QString>
#include <QTextStream>

namespace caf
{

    template<>
    void caf::AppEnum<RifDataSourceForRftPlt::SourceType>::setUp()
    {
        addItem(RifDataSourceForRftPlt::SourceType::RFT, "RFT", "RFT Cases");
        addItem(RifDataSourceForRftPlt::SourceType::GRID, "GRID", "Grid Cases");
        addItem(RifDataSourceForRftPlt::SourceType::OBSERVED, "OBSERVED", "Observed Data");
        setDefault(RifDataSourceForRftPlt::SourceType::NONE);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CAF_PDM_SOURCE_INIT(RimRftAddress, "RftAddress");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRftAddress::RimRftAddress()
{
    InitPdmObject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRftAddress::RimRftAddress(const RifDataSourceForRftPlt& addr)
{
    InitPdmObject();
    setAddress(addr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRftAddress::setAddress(const RifDataSourceForRftPlt& address)
{
    m_sourceType = address.sourceType();
    m_eclCase = address.eclCase();
    m_wellLogFile = address.wellLogFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt RimRftAddress::address() const
{
    return m_eclCase() != nullptr ?
        RifDataSourceForRftPlt(m_sourceType(), m_eclCase()) :
        RifDataSourceForRftPlt(m_sourceType(), m_wellLogFile());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRftAddress::InitPdmObject()
{
    CAF_PDM_InitFieldNoDefault(&m_sourceType, "SourceType", "SourceType", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_eclCase, "EclipseCase", "EclipseCase", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellLogFile, "WellLogFile", "WellLogFile", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRftAddress& RimRftAddress::operator=(const RimRftAddress& other)
{
    m_sourceType = other.m_sourceType();
    m_eclCase = other.m_eclCase();
    m_wellLogFile = other.m_wellLogFile();
    return *this;
}

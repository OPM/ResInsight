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
    void caf::AppEnum<RifWellRftAddress::SourceType>::setUp()
    {
        addItem(RifWellRftAddress::SourceType::RFT, "RFT", "RFT Cases");
        addItem(RifWellRftAddress::SourceType::GRID, "GRID", "Grid Cases");
        addItem(RifWellRftAddress::SourceType::OBSERVED, "OBSERVED", "Observed Data");
        setDefault(RifWellRftAddress::SourceType::NONE);
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
    CAF_PDM_InitFieldNoDefault(&m_sourceType, "SourceType", "SourceType", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_eclCase, "EclipseCase", "EclipseCase", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellLogFile, "WellLogFile", "WellLogFile", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimRftAddress::setAddress(const RifWellRftAddress& address)
{
    m_sourceType = address.sourceType();
    m_eclCase = address.eclCase();
    m_wellLogFile = address.wellLogFile();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifWellRftAddress RimRftAddress::address() const
{
    return m_eclCase() != nullptr ?
        RifWellRftAddress(m_sourceType(), m_eclCase()) :
        RifWellRftAddress(m_sourceType(), m_wellLogFile());
}

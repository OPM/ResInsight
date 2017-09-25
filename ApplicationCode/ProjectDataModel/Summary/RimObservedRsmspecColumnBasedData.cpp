/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimObservedRsmspecColumnBasedData.h"

#include "RifColumnBasedRsmspecParser.h"
#include "RifReaderRmspecColumnBasedData.h"
#include "RifSummaryReaderInterface.h"

#include "cafUtils.h"

CAF_PDM_SOURCE_INIT(RimObservedRsmspecColumnBasedData, "RimObservedRsmspecColumnBasedData");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedRsmspecColumnBasedData::RimObservedRsmspecColumnBasedData()
{
    CAF_PDM_InitObject("Observed RSMSPEC Column Based Data File", ":/Default.png", "", "");
    m_summaryHeaderFilename.uiCapability()->setUiName("File");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimObservedRsmspecColumnBasedData::~RimObservedRsmspecColumnBasedData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimObservedRsmspecColumnBasedData::createSummaryReaderInterface()
{
    if (caf::Utils::fileExists(this->summaryHeaderFilename()))
    {
        m_summeryReader = new RifReaderRmspecColumnBasedData();
        if (!m_summeryReader->open(this->summaryHeaderFilename()))
        {
            m_summeryReader = nullptr;
        }
    }
    else
    {
        m_summeryReader = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimObservedRsmspecColumnBasedData::summaryReader()
{
    return m_summeryReader.p();
}

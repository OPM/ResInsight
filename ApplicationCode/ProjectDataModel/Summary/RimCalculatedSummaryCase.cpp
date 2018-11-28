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

#include "RimCalculatedSummaryCase.h"

#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"

CAF_PDM_SOURCE_INIT(RimCalculatedSummaryCase,"CalculatedSummaryCase");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculatedSummaryCase::RimCalculatedSummaryCase()
{
    CAF_PDM_InitObject("Calculated",":/SummaryCase48x48.png","","");

    m_calculatedCurveReader = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculatedSummaryCase::~RimCalculatedSummaryCase()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCalculatedSummaryCase::caseName() const
{
    return "Calculated";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculatedSummaryCase::createSummaryReaderInterface()
{
    if (!m_calculatedCurveReader)
    {
        RimSummaryCalculationCollection* calculationCollection = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(calculationCollection);

        m_calculatedCurveReader.reset(new RifCalculatedSummaryCurveReader(calculationCollection));
    
        m_calculatedCurveReader->buildMetaData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimCalculatedSummaryCase::summaryReader()
{
    if (!m_calculatedCurveReader) createSummaryReaderInterface();

    return m_calculatedCurveReader.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculatedSummaryCase::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    // Nothing to do here
}






//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculatedSummaryCase::buildMetaData()
{
    if (!m_calculatedCurveReader) createSummaryReaderInterface();

    m_calculatedCurveReader->buildMetaData();
}

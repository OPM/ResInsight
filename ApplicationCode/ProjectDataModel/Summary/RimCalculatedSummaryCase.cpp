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
    CAF_PDM_InitObject("Calculated Summary Case",":/SummaryCase48x48.png","","");

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
QString RimCalculatedSummaryCase::caseName()
{
    return "Calculated Summary Case";
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

        m_calculatedCurveReader = new RifCalculatedSummaryCurveReader(calculationCollection);
    
        m_calculatedCurveReader->buildMetaData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimCalculatedSummaryCase::summaryReader()
{
    if (!m_calculatedCurveReader) createSummaryReaderInterface();

    return m_calculatedCurveReader;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculatedSummaryCase::RifCalculatedSummaryCurveReader::RifCalculatedSummaryCurveReader(RimSummaryCalculationCollection* calculationCollection)
    : m_calculationCollection(calculationCollection)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimCalculatedSummaryCase::RifCalculatedSummaryCurveReader::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    RimSummaryCalculation* calc = findCalculationByName(resultAddress);
    if (calc)
    {
        return calc->timeSteps();
    }

    static std::vector<time_t> dummy;

    return dummy;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCalculatedSummaryCase::RifCalculatedSummaryCurveReader::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    RimSummaryCalculation* calc = findCalculationByName(resultAddress);
    if (calc)
    {
        *values = calc->values();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RimCalculatedSummaryCase::RifCalculatedSummaryCurveReader::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    return "Calculated Curve Unit";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculatedSummaryCase::RifCalculatedSummaryCurveReader::buildMetaData()
{
    m_allResultAddresses.clear();

    for (RimSummaryCalculation* calc : m_calculationCollection->calculations())
    {
        m_allResultAddresses.push_back(RifEclipseSummaryAddress::calculatedCurveAddress(calc->description().toStdString()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RimCalculatedSummaryCase::RifCalculatedSummaryCurveReader::findCalculationByName(const RifEclipseSummaryAddress& resultAddress) const
{
    if (m_calculationCollection && resultAddress.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED)
    {
        QString calculatedName = QString::fromStdString(resultAddress.quantityName());

        for (RimSummaryCalculation* calc : m_calculationCollection->calculations())
        {
            if (calc->description() == calculatedName)
            {
                return calc;
            }
        }
    }

    return nullptr;
}

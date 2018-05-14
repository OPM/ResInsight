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

#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition()
{
    m_curveDefinition = std::make_pair(nullptr, RifEclipseSummaryAddress());
    m_ensemble        = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition(RimSummaryCase* summaryCase,
                                                     const RifEclipseSummaryAddress& summaryAddress,
                                                     RimSummaryCaseCollection* ensemble)
{
    m_curveDefinition = std::make_pair(summaryCase, summaryAddress);
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiaSummaryCurveDefinition::summaryCase() const
{
    return m_curveDefinition.first;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RiaSummaryCurveDefinition::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RifEclipseSummaryAddress& RiaSummaryCurveDefinition::summaryAddress() const
{
    return m_curveDefinition.second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::isEnsembleCurve() const
{
    return m_ensemble != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::isValid() const
{
    return m_curveDefinition.first != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::resultValues(const RiaSummaryCurveDefinition& curveDefinition, std::vector<double>* values)
{
    CVF_ASSERT(values);

    if (!curveDefinition.summaryAddress().isValid()) return;
    if (!curveDefinition.summaryCase()) return;
    
    RifSummaryReaderInterface* reader = curveDefinition.summaryCase()->summaryReader();
    if (!reader) return;

    reader->values(curveDefinition.summaryAddress(), values);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RiaSummaryCurveDefinition::timeSteps(const RiaSummaryCurveDefinition& curveDefinition)
{
    static std::vector<time_t> dummy;

    if (!curveDefinition.summaryAddress().isValid()) return dummy;
    if (!curveDefinition.summaryCase()) return dummy;

    RifSummaryReaderInterface* reader = curveDefinition.summaryCase()->summaryReader();
    if (!reader) return dummy;

    return reader->timeSteps(curveDefinition.summaryAddress());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaSummaryCurveDefinition::curveDefinitionText() const
{
    return RiaSummaryCurveDefinition::curveDefinitionText(summaryCase(), summaryAddress());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaSummaryCurveDefinition::curveDefinitionText(RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& summaryAddress)
{
    QString txt;

    if (summaryCase)
    {
        txt += summaryCase->caseName();
        txt += ", ";
    }

    txt += QString::fromStdString(summaryAddress.uiText());

    return txt;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::operator<(const RiaSummaryCurveDefinition& other) const
{
    if (m_curveDefinition.first == other.summaryCase())
    {
        return (m_curveDefinition.second < other.summaryAddress());
    }

    return (m_curveDefinition.first < other.summaryCase());
}


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

#include "RimSummaryCurveCollection.h"

//#include "RiaApplication.h"
//
#include "RifReaderEclipseSummary.h"
//
//#include "RigSummaryCaseData.h"
//
//#include "RiaDefines.h"
//#include "RimEclipseResultCase.h"
//#include "RimProject.h"
#include "RimSummaryCase.h"
//#include "RimSummaryCurve.h"
//#include "RimSummaryCurveAppearanceCalculator.h"
//#include "RimSummaryCurveAutoName.h"
//#include "RimSummaryFilter.h"
//#include "RimSummaryPlot.h"
//#include "RimSummaryPlotCollection.h"
//
//#include "RiuLineSegmentQwtPlotCurve.h"
//#include "RiuSummaryQwtPlot.h"
//
//#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"
//
//#include "cafPdmUiListEditor.h"
//#include "cafPdmUiPushButtonEditor.h"
//
//
//QTextStream& operator << (QTextStream& str, const std::vector<RifEclipseSummaryAddress>& sobj)
//{
//    CVF_ASSERT(false);
//    return str;
//}
//
//QTextStream& operator >> (QTextStream& str, std::vector<RifEclipseSummaryAddress>& sobj)
//{
//    CVF_ASSERT(false);
//    return str;
//}
//
//
CAF_PDM_SOURCE_INIT(RimSummaryCurveCollection, "RimSummaryCurveCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection::RimSummaryCurveCollection()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection::~RimSummaryCurveCollection()
{
}

void RimSummaryCurveCollection::createCurves(RimSummaryCase* summaryCase, const QString& stringFilter)
{
    if (summaryCase)
    {
        std::vector<RimSummaryCase*> selectedCases;
        selectedCases.push_back(summaryCase);

        //m_summaryFilter->setCompleteVarStringFilter(stringFilter);

        std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > newCurveDefinitions;

        //createSetOfCasesAndResultAdresses(selectedCases, *m_summaryFilter, &newCurveDefinitions);

        //createCurvesFromCurveDefinitions(newCurveDefinitions);
    }
}

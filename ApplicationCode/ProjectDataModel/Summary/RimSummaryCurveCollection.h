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


#pragma once

//#include "cafPdmField.h"
#include "cafPdmObject.h"
//#include "cafPdmPointer.h"
//#include "cafPdmPtrField.h"
//#include "cafPdmChildField.h"
//#include "cafPdmChildArrayField.h"
//#include "cafAppEnum.h"
//#include "cafPdmPtrArrayField.h"
//
//#include "RifEclipseSummaryAddress.h"
//
//#include "RiaDefines.h"
//#include "RimSummaryCurveAppearanceCalculator.h"
//
//class QwtPlot;
//class QwtPlotCurve;
//class RifReaderEclipseSummary;
class RimSummaryCase;
//class RimSummaryCurve;
//class RimSummaryFilter;
//class RiuLineSegmentQwtPlotCurve;
//class RimSummaryCurveAutoName;
//
//
//Q_DECLARE_METATYPE(RifEclipseSummaryAddress);

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurveCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveCollection();
    virtual ~RimSummaryCurveCollection();

    void createCurves(RimSummaryCase* summaryCase, const QString& stringFilter);
};


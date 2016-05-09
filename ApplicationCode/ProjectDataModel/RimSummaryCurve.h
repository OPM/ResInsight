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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"


class RimEclipseResultCase;
class RifReaderEclipseSummary;
class RiuLineSegmentQwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryCurve();
    virtual ~RimSummaryCurve();

    caf::PdmPtrField<RimEclipseResultCase*> m_eclipseCase;

    caf::PdmField<QString> m_variableName;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);

    void curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values);

private:
    RifReaderEclipseSummary* summaryReader();

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");

private:
    RiuLineSegmentQwtPlotCurve*     m_qwtPlotCurve;

};

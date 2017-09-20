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


#pragma once


#include "RiaDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

#include "QPointer"
#include "QList"

class QwtPlot;
class QwtPlotCurve;
class RimSummaryCase;
class RimSummaryCurve;
class RiuLineSegmentQwtPlotCurve;

//==================================================================================================
///  
//==================================================================================================
class RimSummaryCurveCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveCollection();
    virtual ~RimSummaryCurveCollection();

    bool                                    isCurvesVisible();

    void                                    loadDataAndUpdate();
    void                                    setParentQwtPlot(QwtPlot* plot);
    void                                    detachQwtCurves();

    RimSummaryCurve*                        findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const;

    void                                    addCurve(RimSummaryCurve* curve);
    void                                    deleteCurve(RimSummaryCurve* curve);

    std::vector<RimSummaryCurve*>           curves();
    void                                    deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase);
    void                                    deleteAllCurves();
    void                                    updateCaseNameHasChanged();

private:
    caf::PdmFieldHandle*                    objectToggleField();
    

private:
    caf::PdmPtrArrayField<RimSummaryCase*>      m_selectedSummaryCases;

    caf::PdmField<bool>                         m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*>   m_curves;

};


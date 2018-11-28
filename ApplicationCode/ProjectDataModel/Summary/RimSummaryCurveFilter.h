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
#include "cafPdmChildField.h"
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"
#include "cafPdmPtrArrayField.h"

#include "RifEclipseSummaryAddressQMetaType.h"

#include "RiaDefines.h"
#include "RimSummaryCurveAppearanceCalculator.h"

class RimSummaryFilter;
class RimSummaryCurveAutoName;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurveFilter_OBSOLETE : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveFilter_OBSOLETE();
    ~RimSummaryCurveFilter_OBSOLETE() override;

    std::vector<RimSummaryCurve*>           curves();
    void                                    clearCurvesWithoutDelete();

private:
    //QPointer<QwtPlot>                       m_parentQwtPlot;

    // Fields
    caf::PdmField<bool>                       m_showCurves;
    caf::PdmPtrArrayField<RimSummaryCase*>    m_selectedSummaryCases;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;

    caf::PdmField< caf::AppEnum< RiaDefines::PlotAxis > > m_plotAxis;
    caf::PdmField<bool>                       m_showLegend;


    // Filter fields
    caf::PdmChildField<RimSummaryFilter*>   m_summaryFilter;
    caf::PdmField<std::vector<RifEclipseSummaryAddress> > 
                                            m_uiFilterResultMultiSelection;

    caf::PdmChildField<RimSummaryCurveAutoName*>   m_curveNameConfig;

    caf::PdmField<bool>                     m_autoApplyChangesToPlot;
    caf::PdmField<bool>                     m_applyButtonField;

    caf::PdmField<bool>                     m_useAutoAppearanceAssignment;
    typedef caf::AppEnum<RimSummaryCurveAppearanceCalculator::CurveAppearanceType> AppearanceTypeAppEnum;
    caf::PdmField< AppearanceTypeAppEnum >  m_caseAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_variableAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_wellAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_groupAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_regionAppearanceType;

    std::vector< caf::PdmPointer<RimSummaryCase> > m_selectionCache;
};


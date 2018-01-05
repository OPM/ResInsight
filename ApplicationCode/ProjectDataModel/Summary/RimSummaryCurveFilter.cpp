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

#include "RimSummaryCurveFilter.h"

#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryFilter.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"

// See also corresponding fake implementations in RimSummaryCurve

QTextStream& operator << (QTextStream& str, const std::vector<RifEclipseSummaryAddress>& sobj)
{
    CVF_ASSERT(false);
    return str;
}

QTextStream& operator >> (QTextStream& str, std::vector<RifEclipseSummaryAddress>& sobj)
{
    CVF_ASSERT(false);
    return str;
}


CAF_PDM_SOURCE_INIT(RimSummaryCurveFilter_OBSOLETE, "SummaryCurveFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveFilter_OBSOLETE::RimSummaryCurveFilter_OBSOLETE()
{
    CAF_PDM_InitObject("Curve Filter", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCases, "SummaryCases", "Cases", "", "", "");
    m_selectedSummaryCases.uiCapability()->setUiTreeChildrenHidden(true);
    m_selectedSummaryCases.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_selectedSummaryCases.uiCapability()->setAutoAddingOptionFromValue(false);
    m_selectedSummaryCases.xmlCapability()->setIOWritable(false);
    m_selectedSummaryCases.xmlCapability()->setIOReadable(false);
    m_selectedSummaryCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitFieldNoDefault(&m_summaryFilter, "VarListFilter", "Filter", "", "", "");
    m_summaryFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_summaryFilter.uiCapability()->setUiHidden(true);

    m_summaryFilter = new RimSummaryFilter;

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultMultiSelection, "FilterResultSelection", "Filter Result", "", "Ctrl-A : Select All", "");
    m_uiFilterResultMultiSelection.xmlCapability()->setIOWritable(false);
    m_uiFilterResultMultiSelection.xmlCapability()->setIOReadable(false);
    m_uiFilterResultMultiSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultMultiSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_uiFilterResultMultiSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    
    CAF_PDM_InitFieldNoDefault(&m_curves, "FilteredCurves", "Filtered Curves", "", "", "");
    m_curves.uiCapability()->setUiHidden(true);
    m_curves.uiCapability()->setUiTreeChildrenHidden(false);

    CAF_PDM_InitFieldNoDefault(&m_applyButtonField, "ApplySelection", "", "", "", "");
    m_applyButtonField.xmlCapability()->setIOWritable(false);
    m_applyButtonField.xmlCapability()->setIOReadable(false);
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LEFT);

    CAF_PDM_InitField(&m_autoApplyChangesToPlot, "AutoApplyFilterChanges", true, "Auto Apply Changes", "", "", "");

    CAF_PDM_InitField(&m_showCurves, "IsActive", true, "Show Curves", "", "", "");
    m_showCurves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_useAutoAppearanceAssignment, "UseAutoAppearanceAssignment", true, "Auto", "", "", "" );

    CAF_PDM_InitFieldNoDefault(&m_caseAppearanceType,     "CaseAppearanceType",     "Case",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_variableAppearanceType, "VariableAppearanceType", "Vector", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellAppearanceType,     "WellAppearanceType",     "Well",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_groupAppearanceType,    "GroupAppearanceType",    "Group",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionAppearanceType,   "RegionAppearanceType",   "Region", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotAxis, "PlotAxis", "Axis", "", "", "");
    CAF_PDM_InitField(&m_showLegend, "ShowLegend", true, "Contribute To Legend", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curveNameConfig, "SummaryCurveNameConfig", "SummaryCurveNameConfig", "", "", "");
    m_curveNameConfig.uiCapability()->setUiHidden(true);
    m_curveNameConfig.uiCapability()->setUiTreeChildrenHidden(true);

    m_curveNameConfig = new RimSummaryCurveAutoName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveFilter_OBSOLETE::~RimSummaryCurveFilter_OBSOLETE()
{
    m_curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryCurveFilter_OBSOLETE::curves()
{
    std::vector<RimSummaryCurve*> myCurves;
    for ( RimSummaryCurve* curve: m_curves)
    {
        myCurves.push_back(curve);
    }

    return myCurves;    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter_OBSOLETE::clearCurvesWithoutDelete()
{
    m_curves.clear();
}

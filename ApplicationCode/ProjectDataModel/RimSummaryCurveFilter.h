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

#include "RimPlotCurve.h"
#include "RifEclipseSummaryAddress.h"
#include "cafAppEnum.h"

#include "RimSummaryCurve.h"
#include "RimSummaryFilter.h"
#include "cafPdmPtrArrayField.h"
#include "RimSummaryCurveAppearanceCalculator.h"

class RimSummaryCase;
class RifReaderEclipseSummary;
class RiuLineSegmentQwtPlotCurve;

Q_DECLARE_METATYPE(RifEclipseSummaryAddress);

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurveFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryCurveFilter();
    virtual ~RimSummaryCurveFilter();

    void                                    createCurves(RimSummaryCase* summaryCase, const QString& stringFilter);

    void                                    loadDataAndUpdate();
    void                                    setParentQwtPlot(QwtPlot* plot);
    void                                    detachQwtCurves();

    RimSummaryCurve*                        findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const;
    std::set<std::string>                   unitNames();

    void                                    updateCaseNameHasChanged();

private:
    void                                    syncCurvesFromUiSelection();
    void                                    createCurvesFromCurveDefinitions(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefinitions);
    void                                    syncUiSelectionFromCurves();
    std::set<RifEclipseSummaryAddress>      findPossibleSummaryAddresses();
    
    void                                    createSetOfCasesAndResultAdresses(
                                                   const std::vector<RimSummaryCase*>& cases,
                                                   const RimSummaryFilter& filter,
                                                   std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >* curveDefinitions) const;

    // Overridden PDM methods
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly); 
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;

    QPointer<QwtPlot>                       m_parentQwtPlot;

    // Fields
    caf::PdmPtrArrayField<RimSummaryCase*>    m_selectedSummaryCases;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;

    caf::PdmField<QString>                  m_selectedVariableDisplayField;

    // Filter fields
    caf::PdmChildField<RimSummaryFilter*>   m_summaryFilter;
    caf::PdmField<std::vector<RifEclipseSummaryAddress> > 
                                            m_uiFilterResultMultiSelection;

    caf::PdmField<bool>                     m_applyButtonField;

    caf::PdmField<bool>                     m_useAutoAppearanceAssignment;
    typedef caf::AppEnum<RimSummaryCurveAppearanceCalculator::CurveAppearanceType> AppearanceTypeAppEnum;
    caf::PdmField< AppearanceTypeAppEnum >  m_caseAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_variableAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_wellAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_groupAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_regionAppearanceType;

};


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

#include "RifEclipseSummaryAddress.h"
#include "RimSummaryCurveAppearanceCalculator.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"


#define OBSERVED_DATA_AVALUE_POSTFIX    "_OBSDATA"

class RimSummaryCase;
class RimSummaryCurveAutoName;
class RimSummaryPlot;

//==================================================================================================
///  
///  
//==================================================================================================
class RicSummaryCurveCreator : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

private:
    typedef caf::AppEnum<RimSummaryCurveAppearanceCalculator::CurveAppearanceType> AppearanceTypeAppEnum;

    class SummaryIdentifierAndField
    {
    public:
        SummaryIdentifierAndField() :
            m_summaryIdentifier((RifEclipseSummaryAddress::SummaryIdentifierType)0),
            m_pdmField(nullptr) 
            {}

        SummaryIdentifierAndField(RifEclipseSummaryAddress::SummaryIdentifierType summaryIdentifier) :
            m_summaryIdentifier(summaryIdentifier),
            m_pdmField(new caf::PdmField<std::vector<QString>>()) 
            {}

        virtual ~SummaryIdentifierAndField()                      { delete m_pdmField; }

        RifEclipseSummaryAddress::SummaryIdentifierType summaryIdentifier() const { return m_summaryIdentifier; }
        caf::PdmField<std::vector<QString>>*            pdmField()                { return m_pdmField; }

    private:
        RifEclipseSummaryAddress::SummaryIdentifierType m_summaryIdentifier;
        caf::PdmField<std::vector<QString>> *           m_pdmField;
    };

public:
    RicSummaryCurveCreator();
    virtual ~RicSummaryCurveCreator();

    RimSummaryPlot*                         previewPlot() { return m_previewPlot;}
    void                                    updateFromSummaryPlot(RimSummaryPlot* targetPlot);

    bool                                    isCloseButtonPressed() const;
    void                                    clearCloseButton();
    void                                    updateCurveNames();

private:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, 
                                                             const QVariant& oldValue, 
                                                             const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName,
                                                                  caf::PdmUiEditorAttribute* attribute) override;


    std::set<RifEclipseSummaryAddress>      findPossibleSummaryAddresses(const std::vector<RimSummaryCase*> &selectedCases, 
                                                                         const SummaryIdentifierAndField *identifierAndField);
    std::set<RifEclipseSummaryAddress>      findPossibleSummaryAddressesFromSelectedCases(const SummaryIdentifierAndField *identifierAndField);
    std::set<RifEclipseSummaryAddress>      findPossibleSummaryAddressesFromSelectedObservedData(const SummaryIdentifierAndField *identifierAndField);

    std::vector<SummaryIdentifierAndField*> buildControllingFieldList(const SummaryIdentifierAndField *identifierAndField);
    SummaryIdentifierAndField*              lookupIdentifierAndFieldFromFieldHandle(const caf::PdmFieldHandle* pdmFieldHandle);
    SummaryIdentifierAndField*              lookupControllingField(const SummaryIdentifierAndField *dependentField);
    bool                                    isAddressCompatibleWithControllingFieldSelection(const RifEclipseSummaryAddress &address, 
                                                              const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList);
    std::set<RifEclipseSummaryAddress>      buildAddressListFromSelections();
    void                                    buildAddressListForCategoryRecursively(RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                                   std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr,
                                                                                   std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>>& identifierPath,
                                                                                   std::set<RifEclipseSummaryAddress>& addressSet);

    void                                    loadDataAndUpdatePlot();
    void                                    syncPreviewCurvesFromUiSelection();
    void                                    updatePreviewCurvesFromCurveDefinitions(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& allCurveDefsToDisplay, 
                                                                                    const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefsToAdd,
                                                                                    const std::set<RimSummaryCurve*>& curvesToDelete);
    std::set<std::string>                   getAllSummaryCaseNames();
    std::set<std::string>                   getAllSummaryWellNames();

    void                                    populateCurveCreator(const RimSummaryPlot& sourceSummaryPlot);
    void                                    updateTargetPlot();
    static void                             copyCurveAndAddToPlot(const RimSummaryCurve *curve, RimSummaryPlot *plot, bool forceVisible = false);

    void                                    resetAllFields();
    void                                    updateEditorsConnectedToPreviewPlot();
    void                                    initCurveAppearanceCalculator(RimSummaryCurveAppearanceCalculator& curveAppearanceCalc);
    void                                    applyAppearanceToAllPreviewCurves();
    void                                    updateAppearanceEditor();
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>>
                                            allPreviewCurveDefs() const;
    void                                    createNewPlot();
    bool                                    isObservedData(RimSummaryCase *sumCase) const;

private:
    caf::PdmPtrArrayField<RimSummaryCase*>                                                              m_selectedCases;

    caf::PdmField<std::vector<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>>              m_selectedSummaryCategories;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>                           m_currentSummaryCategory;
    
    std::map<RifEclipseSummaryAddress::SummaryVarCategory, std::vector<SummaryIdentifierAndField*>>     m_identifierFieldsMap;

    caf::PdmPtrField<RimSummaryPlot*>                                                                   m_targetPlot;
    
    RimSummaryPlot*                                                                                     m_previewPlot;

    caf::PdmField<bool>                                                                                 m_useAutoAppearanceAssignment;
    caf::PdmField<bool>                                                                                 m_appearanceApplyButton;
    caf::PdmField< AppearanceTypeAppEnum >                                                              m_caseAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >                                                              m_variableAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >                                                              m_wellAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >                                                              m_groupAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >                                                              m_regionAppearanceType;

    caf::PdmChildField<RimSummaryCurveAutoName*>                                                        m_curveNameConfig;

    caf::PdmField<bool> m_okButtonField;
    caf::PdmField<bool> m_applyButtonField;
    caf::PdmField<bool> m_closeButtonField;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <functional>

#define OBSERVED_DATA_AVALUE_POSTFIX    "_OBSDATA"

class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryCurveAutoName;
class RimSummaryPlot;
class RiaSummaryCurveDefinition;
class SummaryIdentifierAndField;


using SummarySource = caf::PdmObject;


//==================================================================================================
///  
///  
//==================================================================================================
class RiuSummaryCurveDefSelection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiuSummaryCurveDefSelection();
    virtual ~RiuSummaryCurveDefSelection();

    void                                    setSelectedCurveDefinitions(const std::vector<RiaSummaryCurveDefinition>& curveDefinitions);
    std::vector<RiaSummaryCurveDefinition>  allCurveDefinitionsFromSelection() const;
    std::vector<RiaSummaryCurveDefinition>  selection() const;

    void                                    setMultiSelectionMode(bool multiSelectionMode);
    void                                    hideEnsembles(bool hide);
    void                                    hideSummaryCases(bool hide);
    void                                    setFieldChangedHandler(const std::function<void()>& handlerFunc);

    void                                    setDefaultSelection(const std::vector<SummarySource*>& defaultCases);

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
    std::set<RifEclipseSummaryAddress>      findPossibleSummaryAddressesFromCalculated();

    std::vector<SummaryIdentifierAndField*> buildControllingFieldList(const SummaryIdentifierAndField *identifierAndField) const;
    SummaryIdentifierAndField*              lookupIdentifierAndFieldFromFieldHandle(const caf::PdmFieldHandle* pdmFieldHandle) const;
    SummaryIdentifierAndField*              lookupControllingField(const SummaryIdentifierAndField *dependentField) const;
    bool                                    isAddressCompatibleWithControllingFieldSelection(const RifEclipseSummaryAddress &address, 
                                                                                             const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList) const;
    
    std::set<RifEclipseSummaryAddress>      buildAddressListFromSelections() const;
    void                                    buildAddressListForCategoryRecursively(RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                                   std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr,
                                                                                   std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>>& identifierPath,
                                                                                   std::set<RifEclipseSummaryAddress>& addressSet) const;

    void                                    resetAllFields();
    bool                                    isObservedData(const RimSummaryCase *sumCase) const;

    std::vector<SummarySource*>             selectedSummarySources() const;
    static RimSummaryCase*                  calculatedSummaryCase();

private:
    caf::PdmPtrArrayField<SummarySource*>                                                               m_selectedSources;

    caf::PdmField<std::vector<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>>              m_selectedSummaryCategories;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>                           m_currentSummaryCategory;
    
    std::map<RifEclipseSummaryAddress::SummaryVarCategory, std::vector<SummaryIdentifierAndField*>>     m_identifierFieldsMap;

    bool                                                                                                m_multiSelectionMode;
    
    bool                                                                                                m_hideEnsembles;
    bool                                                                                                m_hideSummaryCases;

    std::function<void()>                                                                               m_toggleChangedHandler;
};

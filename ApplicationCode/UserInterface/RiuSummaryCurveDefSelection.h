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
class RimSummaryCurveAutoName;
class RimSummaryPlot;
class RiaSummaryCurveDefinition;

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
    std::vector<RiaSummaryCurveDefinition>  selectedCurveDefinitions() const;
    void                                    setMultiSelectionMode(bool multiSelectionMode);
    void                                    setFieldChangedHandler(const std::function<void()>& handlerFunc);

private:
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
    bool                                    isObservedData(RimSummaryCase *sumCase) const;

    std::vector<RimSummaryCase*>            summaryCases() const;
    static RimSummaryCase*                  calculatedSummaryCase();

private:
    caf::PdmPtrArrayField<RimSummaryCase*>                                                              m_selectedCases;

    caf::PdmField<std::vector<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>>              m_selectedSummaryCategories;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>                           m_currentSummaryCategory;
    
    std::map<RifEclipseSummaryAddress::SummaryVarCategory, std::vector<SummaryIdentifierAndField*>>     m_identifierFieldsMap;

    bool                                                                                                m_multiSelectionMode;

    std::function<void()>                                                                               m_toggleChangedHandler;
};

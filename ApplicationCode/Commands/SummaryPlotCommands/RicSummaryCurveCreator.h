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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include "RifEclipseSummaryAddress.h"
#include "RimSummaryCurve.h"

class RimSummaryCase;


//==================================================================================================
///  
///  
//==================================================================================================
class RicSummaryCurveCreator : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum SummaryIdentifierType
    {
        INPUT_REGION_NUMBER,
        INPUT_REGION2_NUMBER,
        INPUT_WELL_NAME,
        INPUT_WELL_GROUP_NAME,
        INPUT_CELL_IJK,
        INPUT_LGR_NAME,
        INPUT_SEGMENT_NUMBER,
        INPUT_VECTOR_NAME
    };

private:
    class SummaryIdentifierAndField
    {
    public:
        SummaryIdentifierAndField() :
            m_summaryIdentifier((SummaryIdentifierType)0),
            m_pdmField(nullptr) {}
        SummaryIdentifierAndField(SummaryIdentifierType summaryIdentifier) :
            m_summaryIdentifier(summaryIdentifier),
            m_pdmField(new caf::PdmField<std::vector<QString>>()) {}
        virtual ~SummaryIdentifierAndField() { delete m_pdmField; }

        SummaryIdentifierType summaryIdentifier() const { return m_summaryIdentifier; }
        caf::PdmField<std::vector<QString>>* pdmField() { return m_pdmField; }
    private:
        SummaryIdentifierType m_summaryIdentifier;
        caf::PdmField<std::vector<QString>> *m_pdmField;
    };

public:
    RicSummaryCurveCreator();
    virtual ~RicSummaryCurveCreator();

private:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    std::set<RifEclipseSummaryAddress> findPossibleSummaryAddresses(const SummaryIdentifierAndField *identifierAndField);
    std::vector<SummaryIdentifierAndField*> buildControllingFieldList(const SummaryIdentifierAndField *identifierAndField);
    QString getIdentifierTextFromAddress(SummaryIdentifierType itemTypeInput, const RifEclipseSummaryAddress &address);
    SummaryIdentifierAndField* findIdentifierAndField(const caf::PdmFieldHandle* pdmFieldHandle);
    SummaryIdentifierAndField* lookupControllingField(const SummaryIdentifierAndField *identifierAndField);
    bool isAddressSelected(const RifEclipseSummaryAddress &address, const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList);

    void loadDataAndUpdatePlot();
    void syncCurvesFromUiSelection();

private:
    caf::PdmPtrArrayField<RimSummaryCase*>                                                              m_selectedCases;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>                           m_selectedSummaryCategory;
    std::map<RifEclipseSummaryAddress::SummaryVarCategory, std::vector<SummaryIdentifierAndField*>>     m_selectedIdentifiers;
    caf::PdmChildArrayField<RimSummaryCurve*>                                                           m_selectedCurves;
};

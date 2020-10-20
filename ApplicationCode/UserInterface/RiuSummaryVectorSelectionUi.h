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

#define OBSERVED_DATA_AVALUE_POSTFIX "_OBSDATA"

class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryCurveAutoName;
class RimSummaryPlot;
class RiaSummaryCurveDefinition;
class RiaCurveSetDefinition;
class SummaryIdentifierAndField;

using SummarySource = caf::PdmObject;

//==================================================================================================
///
///
//==================================================================================================
class RiuSummaryVectorSelectionUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiuSummaryVectorSelectionUi();
    ~RiuSummaryVectorSelectionUi() override;

    void setSelectedCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions );
    std::vector<RiaSummaryCurveDefinition> allCurveDefinitionsFromSelection() const;
    std::vector<RiaCurveSetDefinition>     allCurveSetDefinitionsFromSelections() const;
    std::vector<RiaSummaryCurveDefinition> selection() const;

    void setMultiSelectionMode( bool multiSelectionMode );
    void hideEnsembles( bool hide );
    void hideSummaryCases( bool hide );
    void enableIndividualEnsembleCaseSelection( bool enable );

    void setFieldChangedHandler( const std::function<void()>& handlerFunc );

    void setDefaultSelection( const std::vector<SummarySource*>& defaultCases );

private:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    std::set<RifEclipseSummaryAddress>
        findPossibleSummaryAddresses( const std::vector<SummarySource*>& selectedSources,
                                      const SummaryIdentifierAndField*   identifierAndField ) const;
    std::set<RifEclipseSummaryAddress>
        findPossibleSummaryAddressesFromSelectedCases( const SummaryIdentifierAndField* identifierAndField ) const;
    std::set<RifEclipseSummaryAddress>
                                       findPossibleSummaryAddressesFromSelectedObservedData( const SummaryIdentifierAndField* identifierAndField ) const;
    std::set<RifEclipseSummaryAddress> findPossibleSummaryAddressesFromCalculated() const;

    std::vector<SummaryIdentifierAndField*>
                               buildControllingFieldList( const SummaryIdentifierAndField* identifierAndField ) const;
    SummaryIdentifierAndField* lookupIdentifierAndFieldFromFieldHandle( const caf::PdmFieldHandle* pdmFieldHandle ) const;
    SummaryIdentifierAndField* lookupControllingField( const SummaryIdentifierAndField* dependentField ) const;
    bool                       isAddressCompatibleWithControllingFieldSelection(
                              const RifEclipseSummaryAddress&                address,
                              const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList ) const;

    std::set<RifEclipseSummaryAddress> buildAddressListFromSelections() const;
    void                               buildAddressListForCategoryRecursively(
                                      RifEclipseSummaryAddress::SummaryVarCategory                                      category,
                                      std::vector<SummaryIdentifierAndField*>::const_iterator                           identifierAndFieldItr,
                                      std::vector<std::pair<RifEclipseSummaryAddress::SummaryIdentifierType, QString>>& identifierPath,
                                      std::set<RifEclipseSummaryAddress>&                                               addressSet ) const;

    void resetAllFields();
    bool isObservedData( const RimSummaryCase* sumCase ) const;

    std::vector<SummarySource*> selectedSummarySources() const;
    static RimSummaryCase*      calculatedSummaryCase();

    void appendOptionItemsForSources( QList<caf::PdmOptionItemInfo>& options ) const;
    void appendOptionItemsForCategories( QList<caf::PdmOptionItemInfo>& options ) const;
    void appendOptionItemsForSubCategoriesAndVectors( QList<caf::PdmOptionItemInfo>& options,
                                                      SummaryIdentifierAndField*     identifierAndField ) const;

    void handleAddedSource( SummarySource* sourceAdded );
    void handleRemovedSource( SummarySource* sourceRemoved );

private:
    caf::PdmPtrArrayField<SummarySource*> m_selectedSources;
    std::vector<SummarySource*>           m_previouslySelectedSources;

    caf::PdmField<std::vector<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>> m_selectedSummaryCategories;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>>              m_currentSummaryCategory;

    std::map<RifEclipseSummaryAddress::SummaryVarCategory, std::vector<SummaryIdentifierAndField*>> m_identifierFieldsMap;

    bool m_multiSelectionMode;

    bool m_hideEnsembles;
    bool m_hideSummaryCases;
    bool m_showIndividualEnsembleCases;

    std::function<void()> m_toggleChangedHandler;

    size_t m_prevCurveCount;
    size_t m_prevCurveSetCount;
};

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

#include "RifEclipseSummaryAddressDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <functional>

#define OBSERVED_DATA_AVALUE_POSTFIX "_OBSDATA"

class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryCurveAutoName;
class RimSummaryPlot;
class RiaSummaryCurveDefinition;
class RiaCurveSetDefinition;
class SummaryIdentifierAndField;
class RifEclipseSummaryAddress;

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

    void                                   setSelectedCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions );
    std::vector<RiaSummaryCurveDefinition> allCurveDefinitionsFromSelection() const;
    std::vector<RiaCurveSetDefinition>     allCurveSetDefinitionsFromSelections() const;
    std::vector<RiaSummaryCurveDefinition> selection() const;

    void setMultiSelectionMode( bool multiSelectionMode );
    void hideEnsembles( bool hide );
    void hideSummaryCases( bool hide );
    void enableIndividualEnsembleCaseSelection( bool enable );

    void hideDifferenceVectors( bool hide );
    void hideHistoryVectors( bool hide );
    void hideVectorsWithoutHistory( bool hide );
    void hideCalculationIncompatibleCategories( bool hide );

    void setFieldChangedHandler( const std::function<void()>& handlerFunc );

    void setDefaultSelection( const std::vector<SummarySource*>& defaultCases );

    static QList<caf::PdmOptionItemInfo>
        optionsForSummaryDataSource( bool hideSummaryCases, bool hideEnsembles, bool showIndividualEnsembleCases );

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    std::set<RifEclipseSummaryAddress> findPossibleSummaryAddresses( const std::vector<SummarySource*>& selectedSources,
                                                                     const SummaryIdentifierAndField*   identifierAndField ) const;
    std::set<RifEclipseSummaryAddress> findPossibleSummaryAddressesFromSelectedCases( const SummaryIdentifierAndField* identifierAndField ) const;
    std::set<RifEclipseSummaryAddress>
        findPossibleSummaryAddressesFromSelectedObservedData( const SummaryIdentifierAndField* identifierAndField ) const;

    std::vector<SummaryIdentifierAndField*> buildControllingFieldList( const SummaryIdentifierAndField* identifierAndField ) const;
    SummaryIdentifierAndField*              lookupIdentifierAndFieldFromFieldHandle( const caf::PdmFieldHandle* pdmFieldHandle ) const;
    SummaryIdentifierAndField*              lookupControllingField( const SummaryIdentifierAndField* dependentField ) const;
    bool                                    isAddressCompatibleWithControllingFieldSelection( const RifEclipseSummaryAddress&                address,
                                                                                              const std::vector<SummaryIdentifierAndField*>& identifierAndFieldList ) const;

    std::set<RifEclipseSummaryAddress> buildAddressListFromSelections() const;
    void                               buildAddressListForCategoryRecursively( RifEclipseSummaryAddressDefines::SummaryCategory        category,
                                                                               std::vector<SummaryIdentifierAndField*>::const_iterator identifierAndFieldItr,
                                                                               std::vector<std::pair<RifEclipseSummaryAddressDefines::SummaryIdentifierType, QString>>& identifierPath,
                                                                               std::set<RifEclipseSummaryAddress>& addressSet ) const;

    void resetAllFields();
    bool isObservedData( const RimSummaryCase* sumCase ) const;

    std::vector<SummarySource*> selectedSummarySources() const;

    void appendOptionItemsForSources( QList<caf::PdmOptionItemInfo>& options ) const;
    void appendOptionItemsForCategories( QList<caf::PdmOptionItemInfo>& options ) const;
    void appendOptionItemsForSubCategoriesAndVectors( QList<caf::PdmOptionItemInfo>& options,
                                                      SummaryIdentifierAndField*     identifierAndField ) const;

private:
    caf::PdmPtrArrayField<SummarySource*> m_selectedSources;

    caf::PdmField<std::vector<caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>>> m_selectedSummaryCategories;
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>>              m_currentSummaryCategory;

    std::map<RifEclipseSummaryAddressDefines::SummaryCategory, std::vector<SummaryIdentifierAndField*>> m_identifierFieldsMap;

    bool m_multiSelectionMode;

    bool m_hideEnsembles;
    bool m_hideSummaryCases;
    bool m_showIndividualEnsembleCases;

    bool m_hideHistoryVectors;
    bool m_hideVectorsWithoutHistory;
    bool m_hideDifferenceVectors;

    bool m_hideCalculationIncompatibleCategories;

    std::function<void()> m_toggleChangedHandler;

    size_t m_prevCurveCount;
    size_t m_prevCurveSetCount;
};

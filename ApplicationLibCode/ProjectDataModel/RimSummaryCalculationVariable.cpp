/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimSummaryCalculationVariable.h"

#include "RiaApplication.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"

#include "RifEclipseSummaryAddressQMetaType.h"

#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"

#include "RiuDragDrop.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryCalculationVariable, "RimSummaryCalculationVariable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable::RimSummaryCalculationVariable()
{
    CAF_PDM_InitObject( "RimSummaryCalculationVariable", ":/octave.png" );

    CAF_PDM_InitFieldNoDefault( &m_button, "PushButton", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_button );

    CAF_PDM_InitFieldNoDefault( &m_case, "SummaryCase", "Summary Case" );
    CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "SummaryAddress", "Summary Address" );

    m_summaryAddress = new RimSummaryAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_button )
    {
        bool updateContainingEditor = false;

        {
            RiuSummaryVectorSelectionDialog dlg( nullptr );
            dlg.hideEnsembles();
            dlg.hideCalculationIncompatibleCategories();

            readDataFromApplicationStore( &dlg );

            if ( dlg.exec() == QDialog::Accepted )
            {
                std::vector<RiaSummaryCurveDefinition> curveSelection = dlg.curveSelection();
                if ( !curveSelection.empty() )
                {
                    m_case = curveSelection[0].summaryCaseY();
                    m_summaryAddress->setAddress( curveSelection[0].summaryAddressY() );

                    writeDataToApplicationStore();

                    updateContainingEditor = true;
                }
            }
        }

        if ( updateContainingEditor )
        {
            auto rimCalculation = firstAncestorOrThisOfTypeAsserted<RimSummaryCalculation>();

            // RimCalculation is pointed to by RicSummaryCurveCalculator in a PtrField
            // Update editors connected to RicSummaryCurveCalculator
            std::vector<caf::PdmObjectHandle*> referringObjects = rimCalculation->objectsWithReferringPtrFields();
            for ( auto o : referringObjects )
            {
                o->uiCapability()->updateConnectedEditors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculationVariable::displayString() const
{
    QString caseName;
    if ( m_case() ) caseName = m_case()->displayCaseName();

    return RiaSummaryCurveDefinition::curveDefinitionText( caseName, m_summaryAddress()->address() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCalculationVariable::summaryCase()
{
    return m_case();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddress* RimSummaryCalculationVariable::summaryAddress()
{
    return m_summaryAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::setSummaryAddress( const RimSummaryAddress& address )
{
    m_summaryAddress()->setAddress( address.address() );

    auto summaryCase = RiaSummaryTools::summaryCaseById( address.caseId() );

    // Use first summary case for ensemble addresses
    if ( address.isEnsemble() )
    {
        auto ensemble = RiaSummaryTools::ensembleById( address.ensembleId() );
        if ( ensemble )
        {
            summaryCase = ensemble->firstSummaryCase();
        }
    }

    if ( summaryCase ) m_case = summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::handleDroppedMimeData( const QMimeData* data, Qt::DropAction action, caf::PdmFieldHandle* destinationField )
{
    auto objects = RiuDragDrop::convertToObjects( data );
    if ( !objects.empty() )
    {
        auto address = dynamic_cast<RimSummaryAddress*>( objects.front() );
        if ( address ) setSummaryAddress( *address );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_addressUi );
    uiOrdering.add( &m_button );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTableViewPushButtonEditorAttribute* attr = dynamic_cast<caf::PdmUiTableViewPushButtonEditorAttribute*>( attribute );
    if ( attr )
    {
        attr->registerPushButtonTextForFieldKeyword( m_button.keyword(), "Edit" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::readDataFromApplicationStore( RiuSummaryVectorSelectionDialog* selectionDialog ) const
{
    if ( !selectionDialog ) return;

    auto sumCase    = m_case();
    auto sumAddress = m_summaryAddress->address();
    if ( !sumCase && !sumAddress.isValid() )
    {
        QVariant var = RiaApplication::instance()->cacheDataObject( "CalculatorSummaryAddress" );

        auto lastUsedAddress = var.value<RifEclipseSummaryAddress>();
        if ( lastUsedAddress.isValid() )
        {
            sumAddress = lastUsedAddress;
        }

        QString lastUsedSummaryCaseString = RiaApplication::instance()->cacheDataObject( "CalculatorSummaryCase" ).toString();

        auto* lastUsedSummaryCase =
            dynamic_cast<RimSummaryCase*>( caf::PdmReferenceHelper::objectFromReference( RimProject::current(), lastUsedSummaryCaseString ) );
        if ( lastUsedSummaryCase )
        {
            sumCase = lastUsedSummaryCase;
        }
    }

    selectionDialog->setCaseAndAddress( sumCase, sumAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::writeDataToApplicationStore() const
{
    QString refFromProjectToObject = caf::PdmReferenceHelper::referenceFromRootToObject( RimProject::current(), m_case );
    RiaApplication::instance()->setCacheDataObject( "CalculatorSummaryCase", refFromProjectToObject );

    QVariant sumAdrVar = QVariant::fromValue( m_summaryAddress->address() );
    RiaApplication::instance()->setCacheDataObject( "CalculatorSummaryAddress", sumAdrVar );
}

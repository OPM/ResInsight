/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCustomObjectiveFunctionWeight.h"

#include "RiaStdStringTools.h"
#include "Summary/RiaSummaryCurveDefinition.h"

#include "RimCustomObjectiveFunction.h"
#include "RimEnsembleCurveSet.h"
#include "RimObjectiveFunctionTools.h"
#include "RimSummaryAddress.h"

#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimCustomObjectiveFunctionWeight, "RimCustomObjectiveFunctionWeight" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionWeight::RimCustomObjectiveFunctionWeight()
{
    CAF_PDM_InitObject( "Custom Objective Function Weight", ":/ObjectiveFunctionWeight.svg" );

    CAF_PDM_InitFieldNoDefault( &m_title, "WeightTitle", "Title" );
    m_title.registerGetMethod( this, &RimCustomObjectiveFunctionWeight::title );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddressesUiField, "SelectedObjectiveSummaryVar", "Vector" );
    m_objectiveValuesSummaryAddressesUiField.xmlCapability()->disableIO();
    m_objectiveValuesSummaryAddressesUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddresses, "ObjectiveSummaryAddress", "Summary Address" );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSelectSummaryAddressPushButton, "SelectObjectiveSummaryAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_objectiveValuesSelectSummaryAddressPushButton );
    m_objectiveValuesSelectSummaryAddressPushButton = false;

    CAF_PDM_InitField( &m_weightValue, "WeightValue", 1.0, "Weight" );
    m_weightValue.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunction, "ObjectiveFunction", "Objective Function" );
    m_objectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomObjectiveFunctionWeight::title() const
{
    std::vector<RifEclipseSummaryAddress> addressVector;
    for ( RimSummaryAddress* address : m_objectiveValuesSummaryAddresses )
    {
        addressVector.push_back( address->address() );
    }
    return QString( "%0 * %1::%2%3%4" )
        .arg( m_weightValue(), 0, 'f', 2 )
        .arg( caf::AppEnum<RimObjectiveFunction::FunctionType>( m_objectiveFunction() ).text() )
        .arg( addressVector.size() > 1 ? "(" : "" )
        .arg( QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector, " + " ) ) )
        .arg( addressVector.size() > 1 ? ")" : "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionWeight::setSummaryAddress( RifEclipseSummaryAddress address )
{
    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
    summaryAddress->setAddress( address );
    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
    parentObjectiveFunction()->onWeightChanged();
    updateAddressesUiField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifEclipseSummaryAddress> RimCustomObjectiveFunctionWeight::summaryAddresses() const
{
    std::vector<RifEclipseSummaryAddress> addresses;
    for ( auto address : m_objectiveValuesSummaryAddresses )
    {
        addresses.push_back( address->address() );
    }
    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObjectiveFunction::FunctionType RimCustomObjectiveFunctionWeight::objectiveFunction() const
{
    return m_objectiveFunction();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimCustomObjectiveFunctionWeight::weightValue() const
{
    return m_weightValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCustomObjectiveFunctionWeight::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionWeight::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_objectiveValuesSummaryAddressesUiField )
    {
        updateAddressesUiField();
    }
    else if ( changedField == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimObjectiveFunctionTools::configureDialogForObjectiveFunctions( &dlg );
        RimSummaryEnsemble* candidateEnsemble = parentCurveSet()->summaryEnsemble();

        std::vector<RifEclipseSummaryAddress> candidateAddresses;
        for ( auto address : m_objectiveValuesSummaryAddresses().childrenByType() )
        {
            candidateAddresses.push_back( address->address() );
        }

        dlg.setEnsembleAndAddresses( candidateEnsemble, candidateAddresses );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_objectiveValuesSummaryAddresses.deleteChildren();
                for ( auto address : curveSelection )
                {
                    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                    summaryAddress->setAddress( address.summaryAddressY() );
                    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                }
            }
        }
        updateAddressesUiField();
        m_objectiveValuesSelectSummaryAddressPushButton = false;
        parentObjectiveFunction()->onWeightChanged();
    }
    else if ( changedField == &m_weightValue )
    {
        parentObjectiveFunction()->onWeightChanged();
    }
    else if ( changedField == &m_objectiveFunction )
    {
        parentObjectiveFunction()->onWeightChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionWeight::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_objectiveValuesSummaryAddressesUiField );
    uiOrdering.add( &m_objectiveValuesSelectSummaryAddressPushButton, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
    uiOrdering.add( &m_weightValue );
    uiOrdering.add( &m_objectiveFunction );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionWeight::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                              QString                    uiConfigName,
                                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_weightValue )
    {
        caf::PdmUiLineEditorAttribute* myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->validator = new QDoubleValidator( 0.0, 9999.0, 2 );
    }
    else if ( field == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "...";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCustomObjectiveFunctionWeight::userDescriptionField()
{
    return &m_title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimCustomObjectiveFunctionWeight::parentCurveSet() const
{
    return firstAncestorOrThisOfType<RimEnsembleCurveSet>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunction* RimCustomObjectiveFunctionWeight::parentObjectiveFunction() const
{
    return firstAncestorOrThisOfType<RimCustomObjectiveFunction>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomObjectiveFunctionWeight::updateAddressesUiField()
{
    std::vector<RifEclipseSummaryAddress> addressVector;
    for ( RimSummaryAddress* address : m_objectiveValuesSummaryAddresses )
    {
        addressVector.push_back( address->address() );
    }
    m_objectiveValuesSummaryAddressesUiField = QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector ) );
}

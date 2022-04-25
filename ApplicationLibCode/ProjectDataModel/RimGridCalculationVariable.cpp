/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimGridCalculationVariable.h"

#include "RiaApplication.h"

// #include "RimGridAddress.h"
// #include "RimGridCalculation.h"
// #include "RimGridCase.h"
// #include "RimGridCurve.h"
// #include "RimProject.h"

// #include "RiuGridVectorSelectionDialog.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"

CAF_PDM_SOURCE_INIT( RimGridCalculationVariable, "RimGridCalculationVariable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCalculationVariable::RimGridCalculationVariable()
{
    CAF_PDM_InitObject( "RimGridCalculationVariable", ":/octave.png" );

    // CAF_PDM_InitFieldNoDefault( &m_button, "PushButton", "" );
    // m_button.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    // m_button.xmlCapability()->disableIO();

    // CAF_PDM_InitFieldNoDefault( &m_case, "GridCase", "Grid Case" );
    // CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "GridAddress", "Grid Address" );

    // m_summaryAddress = new RimGridAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    // if ( changedField == &m_button )
    // {
    //     bool updateContainingEditor = false;

    //     {
    //         RiuGridVectorSelectionDialog dlg( nullptr );
    //         dlg.hideEnsembles();

    //         readDataFromApplicationStore( &dlg );

    //         if ( dlg.exec() == QDialog::Accepted )
    //         {
    //             std::vector<RiaGridCurveDefinition> curveSelection = dlg.curveSelection();
    //             if ( curveSelection.size() > 0 )
    //             {
    //                 m_case = curveSelection[0].summaryCase();
    //                 m_summaryAddress->setAddress( curveSelection[0].summaryAddress() );

    //                 writeDataToApplicationStore();

    //                 updateContainingEditor = true;
    //             }
    //         }
    //     }

    //     if ( updateContainingEditor )
    //     {
    //         RimGridCalculation* rimCalculation = nullptr;
    //         this->firstAncestorOrThisOfTypeAsserted( rimCalculation );

    //         // RimCalculation is pointed to by RicGridCurveCalculator in a PtrField
    //         // Update editors connected to RicGridCurveCalculator
    //         std::vector<caf::PdmObjectHandle*> referringObjects;
    //         rimCalculation->objectsWithReferringPtrFields( referringObjects );
    //         for ( auto o : referringObjects )
    //         {
    //             o->uiCapability()->updateConnectedEditors();
    //         }
    //     }
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCalculationVariable::displayString() const
{
    QString caseName = "Fixed name string junk";
    return caseName;
    // if ( m_case() ) caseName = m_case()->displayCaseName();

    // return RiaGridCurveDefinition::curveDefinitionText( caseName, m_summaryAddress()->address() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCalculationVariable::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // uiOrdering.add( &m_name );
    // uiOrdering.add( &m_addressUi );
    // uiOrdering.add( &m_button );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimGridCalculationVariable::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute*
// attribute )
// {
//     caf::PdmUiTableViewPushButtonEditorAttribute* attr =
//         dynamic_cast<caf::PdmUiTableViewPushButtonEditorAttribute*>( attribute );
//     if ( attr )
//     {
//         attr->registerPushButtonTextForFieldKeyword( m_button.keyword(), "Edit" );
//     }
// }

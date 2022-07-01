/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicUserDefinedCalculatorUi.h"

#include "RimUserDefinedCalculation.h"
#include "RimUserDefinedCalculationCollection.h"

#include "cafAssert.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RicUserDefinedCalculatorUi, "RicUserDefinedCalculator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicUserDefinedCalculatorUi::RicUserDefinedCalculatorUi()
{
    CAF_PDM_InitObject( "RicUserDefinedCalculator" );

    CAF_PDM_InitFieldNoDefault( &m_currentCalculation, "CurrentCalculation", "" );
    m_currentCalculation.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    // m_currentCalculation.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_currentCalculation.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_newCalculation, "NewCalculation", "New Calculation" );
    RicUserDefinedCalculatorUi::assignPushButtonEditor( &m_newCalculation );

    CAF_PDM_InitFieldNoDefault( &m_deleteCalculation, "DeleteCalculation", "Delete Calculation" );
    RicUserDefinedCalculatorUi::assignPushButtonEditor( &m_deleteCalculation );

    m_calcContextMenuMgr = std::unique_ptr<RiuCalculationsContextMenuManager>( new RiuCalculationsContextMenuManager() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedCalculation* RicUserDefinedCalculatorUi::currentCalculation() const
{
    return m_currentCalculation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::setCurrentCalculation( RimUserDefinedCalculation* calculation )
{
    m_currentCalculation = calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUserDefinedCalculatorUi::parseExpression() const
{
    if ( m_currentCalculation() )
    {
        QString previousCurveName = m_currentCalculation->description();
        if ( !m_currentCalculation()->parseExpression() )
        {
            return false;
        }

        QString currentCurveName = m_currentCalculation->description();
        if ( previousCurveName != currentCurveName )
        {
            notifyCalculatedNameChanged( m_currentCalculation()->id(), currentCurveName );
        }

        m_currentCalculation()->updateDependentObjects();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    if ( changedField == &m_newCalculation )
    {
        m_newCalculation = false;

        m_currentCalculation = calculationCollection()->addCalculation();

        this->updateConnectedEditors();
    }
    else if ( changedField == &m_deleteCalculation )
    {
        m_deleteCalculation = false;

        if ( m_currentCalculation() )
        {
            calculationCollection()->deleteCalculation( m_currentCalculation() );
            m_currentCalculation = nullptr;

            this->updateConnectedEditors();
            caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !m_currentCalculation() && !calculationCollection()->calculations().empty() )
    {
        m_currentCalculation = calculationCollection()->calculations()[0];
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword( "Calculations", calculationsGroupName() );
        group->add( &m_currentCalculation );
        group->add( &m_newCalculation );
        group->add( &m_deleteCalculation );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword( "Calculation Settings", calulationGroupName() );
        if ( m_currentCalculation() )
        {
            m_currentCalculation->uiOrdering( uiConfigName, *group );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicUserDefinedCalculatorUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_currentCalculation )
    {
        for ( auto c : calculationCollection()->calculations() )
        {
            options.push_back( caf::PdmOptionItemInfo( c->description(), c ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::assignPushButtonEditor( caf::PdmFieldHandle* fieldHandle )
{
    CAF_ASSERT( fieldHandle );
    CAF_ASSERT( fieldHandle->uiCapability() );

    fieldHandle->uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    fieldHandle->uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::assignPushButtonEditorText( caf::PdmUiEditorAttribute* attribute, const QString& text )
{
    caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->m_buttonText = text;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUserDefinedCalculatorUi::calculate() const
{
    if ( m_currentCalculation() )
    {
        QString previousCurveName = m_currentCalculation->description();
        if ( !m_currentCalculation()->parseExpression() )
        {
            return false;
        }

        QString currentCurveName = m_currentCalculation->description();
        if ( previousCurveName != currentCurveName )
        {
            notifyCalculatedNameChanged( m_currentCalculation()->id(), currentCurveName );
        }

        if ( !m_currentCalculation()->calculate() )
        {
            return false;
        }

        m_currentCalculation()->updateDependentObjects();
        caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_newCalculation == field )
    {
        RicUserDefinedCalculatorUi::assignPushButtonEditorText( attribute, "New Calculation" );
    }
    else if ( &m_deleteCalculation == field )
    {
        RicUserDefinedCalculatorUi::assignPushButtonEditorText( attribute, "Delete Calculation" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUserDefinedCalculatorUi::onEditorWidgetsCreated()
{
    if ( m_currentCalculation() != nullptr )
    {
        m_currentCalculation->attachToWidget();
    }

    for ( const auto& e : m_currentCalculation.uiCapability()->connectedEditors() )
    {
        caf::PdmUiListEditor* listEditor = dynamic_cast<caf::PdmUiListEditor*>( e );
        if ( !listEditor ) continue;

        QWidget* widget = listEditor->editorWidget();
        if ( !widget ) continue;

        m_calcContextMenuMgr->attachWidget( widget, this );
    }
}

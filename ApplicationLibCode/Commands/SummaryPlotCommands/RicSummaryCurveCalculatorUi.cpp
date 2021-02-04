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

#include "RicSummaryCurveCalculatorUi.h"

#include "RiaSummaryTools.h"

#include "RimProject.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RicSummaryCurveCalculatorUi, "RicSummaryCurveCalculator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculatorUi::RicSummaryCurveCalculatorUi()
{
    CAF_PDM_InitObject( "RicSummaryCurveCalculator", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_currentCalculation, "CurrentCalculation", "", "", "", "" );
    m_currentCalculation.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    // m_currentCalculation.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_currentCalculation.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_newCalculation, "NewCalculation", "New Calculation", "", "", "" );
    RicSummaryCurveCalculatorUi::assignPushButtonEditor( &m_newCalculation );

    CAF_PDM_InitFieldNoDefault( &m_deleteCalculation, "DeleteCalculation", "Delete Calculation", "", "", "" );
    RicSummaryCurveCalculatorUi::assignPushButtonEditor( &m_deleteCalculation );

    m_calcContextMenuMgr = std::make_unique<RiuCalculationsContextMenuManager>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCalculatorUi::calculatedSummariesGroupName()
{
    return "CalculatedSummariesGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCalculatorUi::calulationGroupName()
{
    return "CalulationGroupName";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RicSummaryCurveCalculatorUi::currentCalculation() const
{
    return m_currentCalculation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::setCurrentCalculation( RimSummaryCalculation* calculation )
{
    m_currentCalculation = calculation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCalculatorUi::parseExpression() const
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
            RiaSummaryTools::notifyCalculatedCurveNameHasChanged( m_currentCalculation()->id(), currentCurveName );
        }

        m_currentCalculation()->updateDependentCurvesAndPlots();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
void RicSummaryCurveCalculatorUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !m_currentCalculation() )
    {
        if ( !calculationCollection()->textExpressionCalculations().empty() )
        {
            m_currentCalculation = calculationCollection()->textExpressionCalculations()[0];
        }
    }

    {
        caf::PdmUiGroup* group =
            uiOrdering.addNewGroupWithKeyword( "Calculated Summaries",
                                               RicSummaryCurveCalculatorUi::calculatedSummariesGroupName() );
        group->add( &m_currentCalculation );
        group->add( &m_newCalculation );
        group->add( &m_deleteCalculation );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword( "Calculation Settings",
                                                                    RicSummaryCurveCalculatorUi::calulationGroupName() );
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
    RicSummaryCurveCalculatorUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_currentCalculation )
    {
        for ( auto c : calculationCollection()->textExpressionCalculations() )
        {
            options.push_back( caf::PdmOptionItemInfo( c->description(), c ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationCollection* RicSummaryCurveCalculatorUi::calculationCollection()
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        return proj->calculationCollection();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::assignPushButtonEditor( caf::PdmFieldHandle* fieldHandle )
{
    CVF_ASSERT( fieldHandle );
    CVF_ASSERT( fieldHandle->uiCapability() );

    fieldHandle->uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    fieldHandle->uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::assignPushButtonEditorText( caf::PdmUiEditorAttribute* attribute, const QString& text )
{
    auto* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->m_buttonText = text;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCalculatorUi::calculate() const
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
            RiaSummaryTools::notifyCalculatedCurveNameHasChanged( m_currentCalculation()->id(), currentCurveName );
        }

        if ( !m_currentCalculation()->calculate() )
        {
            return false;
        }

        m_currentCalculation()->updateDependentCurvesAndPlots();
        caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_newCalculation == field )
    {
        RicSummaryCurveCalculatorUi::assignPushButtonEditorText( attribute, "New Calculation" );
    }
    else if ( &m_deleteCalculation == field )
    {
        RicSummaryCurveCalculatorUi::assignPushButtonEditorText( attribute, "Delete Calculation" );
    }
}

//--------------------------------------------------------------------------------------------------
/// f
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculatorUi::onEditorWidgetsCreated()
{
    if ( m_currentCalculation() != nullptr )
    {
        m_currentCalculation->attachToWidget();
    }

    for ( const auto& e : m_currentCalculation.uiCapability()->connectedEditors() )
    {
        auto* listEditor = dynamic_cast<caf::PdmUiListEditor*>( e );
        if ( !listEditor ) continue;

        QWidget* widget = listEditor->editorWidget();
        if ( !widget ) continue;

        m_calcContextMenuMgr->attachWidget( widget, this );
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSummaryCurveManager.h"

#include "RiaSummaryTools.h"
#include "RifSummaryReaderInterface.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaStdStringTools.h"
#include "RifReaderEclipseSummary.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafSelectionManager.h"

#include <QDebug>
#include <QKeyEvent>

CAF_PDM_SOURCE_INIT( RimSummaryCurveManager, "RimSummaryCurveManager" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveManager::RimSummaryCurveManager()
{
    CAF_PDM_InitObject( "Summary Curve Manager", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryPlot, "SummaryPlot", "Summary Plot", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_curveFilterText, "CurveFilterText", "Curve Filter Text", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_curveCandidates, "CurveCandidates", "Candidates", "", "", "" );
    CAF_PDM_InitField( &m_includeDiffCurves,
                       "IncludeDiffCurves",
                       true,
                       "Include Difference Curves",
                       "",
                       "Difference between simulated and observed(history) curve",
                       "" );

    CAF_PDM_InitField( &m_includeHistoryCurves, "IncludeHistoryCurves", true, "Include History Curves", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonReplace, "PushButtonB", "Replace (CTRL + Enter)", "", "", "" );
    m_pushButtonReplace.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonReplace.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonClear, "PushButtonC", "Clear (Alt + Enter)", "", "", "" );
    m_pushButtonClear.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonClear.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonAppend, "PushButtonD", "Append (Shift + Enter)", "", "", "" );
    m_pushButtonAppend.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonAppend.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    updateFromSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue )
{
    if ( changedField == &m_curveFilterText )
    {
        auto mods = QGuiApplication::keyboardModifiers();

        // Use global keyboard modifiers to trigger different events when content is changed in editor changes

        // This is not working, as the fieldChanged concept returns early if the field content is unchanged
        // Must use eventFilter on editor field

        if ( mods & Qt::ShiftModifier )
            appendText();
        else if ( mods & Qt::ControlModifier )
            replaceText();
        else if ( mods & Qt::AltModifier )
            clearText();
    }

    if ( changedField == &m_pushButtonReplace )
    {
        replaceText();
    }
    if ( changedField == &m_pushButtonClear )
    {
        clearText();
    }
    if ( changedField == &m_pushButtonAppend )
    {
        appendText();
    }

    m_pushButtonReplace = false;
    m_pushButtonClear   = false;
    m_pushButtonAppend  = false;

    updateCurveCandidates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryCurveManager::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_summaryPlot )
    {
        auto coll = RiaSummaryTools::summaryPlotCollection();
        coll->summaryPlotItemInfos( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::updateCurveCandidates()
{
    m_curveCandidates.value().clear();

    if ( !m_summaryPlot ) return;

    auto curves = m_summaryPlot->summaryAndEnsembleCurves();
    if ( curves.empty() ) return;

    auto firstSource = curves.front()->summaryCaseY();

    std::set<RifEclipseSummaryAddress> allAddressesFromSource = addressesForSource( firstSource );

    QStringList allCurveAddressFilters = m_curveFilterText().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    std::vector<bool>                  usedFilters;
    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource;
    insertFilteredAddressesInSet( allCurveAddressFilters, allAddressesFromSource, &filteredAddressesFromSource, &usedFilters );

    std::vector<QString> curveCandidates;

    for ( const auto& adr : filteredAddressesFromSource )
    {
        curveCandidates.push_back( QString::fromStdString( adr.uiText() ) );
    }

    m_curveCandidates = curveCandidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::insertFilteredAddressesInSet( const QStringList&                        curveFilters,
                                                           const std::set<RifEclipseSummaryAddress>& allAddressesInCase,
                                                           std::set<RifEclipseSummaryAddress>* setToInsertFilteredAddressesIn,
                                                           std::vector<bool>*                  usedFilters )
{
    std::set<RifEclipseSummaryAddress> candidateAddresses;

    // TODO: Rename to insertFilteredAddressesInSet
    RicSummaryPlotFeatureImpl::filteredSummaryAdressesFromCase( curveFilters,
                                                                allAddressesInCase,
                                                                &candidateAddresses,
                                                                usedFilters );

    if ( !m_includeDiffCurves || !m_includeHistoryCurves )
    {
        std::set<RifEclipseSummaryAddress> tmp;

        const auto diffText = RifReaderEclipseSummary::differenceIdentifier();

        for ( const auto& adr : candidateAddresses )
        {
            if ( !m_includeDiffCurves && RiaStdStringTools::endsWith( adr.quantityName(), diffText ) ) continue;
            if ( !m_includeHistoryCurves && adr.isHistoryQuantity() ) continue;

            tmp.insert( adr );
        }

        std::swap( tmp, candidateAddresses );
    }

    setToInsertFilteredAddressesIn->insert( candidateAddresses.begin(), candidateAddresses.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                    QString                    uiConfigName,
                                                    caf::PdmUiEditorAttribute* attribute )
{
    {
        auto myAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( myAttr )
        {
            if ( field == &m_pushButtonReplace )
            {
                myAttr->m_buttonText = "Replace (Ctrl + Enter)";
            }
            if ( field == &m_pushButtonClear )
            {
                myAttr->m_buttonText = "Clear (Alt + Enter)";
            }
            if ( field == &m_pushButtonAppend )
            {
                myAttr->m_buttonText = "Append (Shift + Enter)";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::appendText()
{
    qDebug() << "append";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::replaceText()
{
    qDebug() << "replaceText";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::clearText()
{
    qDebug() << "clearText";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurveManager::eventFilter( QObject* obj, QEvent* event )
{
    auto keyEvent = dynamic_cast<QKeyEvent*>( event );
    if ( keyEvent )
    {
        auto mods = QGuiApplication::keyboardModifiers();
        // auto mods = keyEvent->modifiers();

        if ( mods & Qt::ShiftModifier )
            appendText();
        else if ( mods & Qt::ControlModifier )
            replaceText();
        else if ( mods & Qt::AltModifier )
            clearText();
    }

    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveManager::updateFromSelection()
{
    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

    RimSummaryPlot* summaryPlot = nullptr;
    if ( destinationObject ) destinationObject->firstAncestorOrThisOfType( summaryPlot );

    if ( m_summaryPlot != summaryPlot )
    {
        m_summaryPlot = summaryPlot;
        updateConnectedEditors();
        updateCurveCandidates();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryCurveManager::addressesForSource( caf::PdmObject* summarySource )
{
    auto* ensemble = dynamic_cast<RimSummaryCaseCollection*>( summarySource );
    if ( ensemble )
    {
        return ensemble->ensembleSummaryAddresses();
    }

    auto* sumCase = dynamic_cast<RimSummaryCase*>( summarySource );

    if ( sumCase )
    {
        RifSummaryReaderInterface* reader = sumCase ? sumCase->summaryReader() : nullptr;
        if ( reader )
        {
            return reader->allResultAddresses();
        }
    }

    return {};
}

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

#include "RimSummaryPlotManager.h"

#include "RiaStdStringTools.h"
#include "RiaSummaryTools.h"

#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafSelectionManager.h"

#include <QKeyEvent>

CAF_PDM_SOURCE_INIT( RimSummaryPlotManager, "RimSummaryPlotManager" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotManager::RimSummaryPlotManager()
{
    CAF_PDM_InitObject( "Summary Plot Manager" );

    CAF_PDM_InitFieldNoDefault( &m_summaryPlot, "SummaryPlot", "Summary Plot" );
    CAF_PDM_InitFieldNoDefault( &m_curveFilterText, "CurveFilterText", "Curve Filter Text" );
    CAF_PDM_InitFieldNoDefault( &m_curveCandidates, "CurveCandidates", "Candidates" );

    CAF_PDM_InitField( &m_includeDiffCurves,
                       "IncludeDiffCurves",
                       true,
                       "Include Difference Curves",
                       "",
                       "Difference between simulated and observed(history) curve",
                       "" );
    m_includeDiffCurves.uiCapability()->setUiEditorTypeName( caf::PdmUiNativeCheckBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonReplace, "PushButtonReplace", "Replace (CTRL + Enter)" );
    m_pushButtonReplace.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonReplace.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonNewPlot, "PushButtonNewPlot", "New (Alt + Enter)" );
    m_pushButtonNewPlot.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonNewPlot.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonAppend, "PushButtonAppend", "Append (Shift + Enter)" );
    m_pushButtonAppend.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_pushButtonAppend.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_labelA, "LabelA", "" );
    m_labelA.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelA.xmlCapability()->disableIO();
    m_labelA.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_labelB, "LabelB", "" );
    m_labelB.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_labelB.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    updateUiFromSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    if ( changedField == &m_curveFilterText || changedField == &m_includeDiffCurves )
    {
        updateCurveCandidates();
    }
    else if ( changedField == &m_pushButtonReplace )
    {
        replaceCurves();
        m_pushButtonReplace = false;
    }
    else if ( changedField == &m_pushButtonNewPlot )
    {
        createNewPlot();
        m_pushButtonNewPlot = false;
    }
    else if ( changedField == &m_pushButtonAppend )
    {
        appendCurves();
        m_pushButtonAppend = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryPlotManager::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
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
void RimSummaryPlotManager::updateCurveCandidates()
{
    m_curveCandidates.value().clear();

    if ( !m_summaryPlot ) return;

    auto curves = m_summaryPlot->summaryAndEnsembleCurves();
    if ( curves.empty() ) return;

    auto firstSource = curves.front()->summaryCaseY();
    if ( !firstSource ) return;

    std::set<RifEclipseSummaryAddress> addressesFromSources = addressesForSource( firstSource );
    QStringList addressFilters = m_curveFilterText().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    auto addresses = computeFilteredAddresses( addressFilters, addressesFromSources );

    std::vector<QString> curveCandidates;
    for ( const auto& adr : addresses )
    {
        curveCandidates.push_back( QString::fromStdString( adr.uiText() ) );
    }

    m_curveCandidates = curveCandidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress>
    RimSummaryPlotManager::computeFilteredAddresses( const QStringList&                        textFilters,
                                                     const std::set<RifEclipseSummaryAddress>& sourceAddresses )
{
    std::set<RifEclipseSummaryAddress> addresses;

    std::vector<bool> usedFilters;
    RicSummaryPlotFeatureImpl::insertFilteredAddressesInSet( textFilters, sourceAddresses, &addresses, &usedFilters );

    if ( m_includeDiffCurves ) return addresses;

    const auto diffText = RifReaderEclipseSummary::differenceIdentifier();

    std::set<RifEclipseSummaryAddress> addressesWithoutDiffVectors;
    for ( const auto& adr : addresses )
    {
        if ( RiaStdStringTools::endsWith( adr.quantityName(), diffText ) ) continue;

        addressesWithoutDiffVectors.insert( adr );
    }

    return addressesWithoutDiffVectors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute )
{
    {
        auto myAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( myAttr )
        {
            if ( field == &m_pushButtonReplace )
            {
                myAttr->m_buttonText = "Replace Curves \n(Ctrl + Enter)";
            }
            if ( field == &m_pushButtonNewPlot )
            {
                myAttr->m_buttonText = "Create New Plot \n(Alt + Enter)";
            }
            if ( field == &m_pushButtonAppend )
            {
                myAttr->m_buttonText = "Append Curves \n(Shift + Enter)";
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_summaryPlot );

    // uiOrdering.add( &m_labelA );
    uiOrdering.add( &m_includeDiffCurves );

    uiOrdering.add( &m_curveFilterText );
    uiOrdering.add( &m_curveCandidates );

    uiOrdering.add( &m_labelB );
    uiOrdering.add( &m_pushButtonAppend, { false } );
    uiOrdering.add( &m_pushButtonReplace, { false } );
    uiOrdering.add( &m_pushButtonNewPlot, { false } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::appendCurves()
{
    RimSummaryPlot* destinationPlot = m_summaryPlot;

    appendCurvesToPlot( destinationPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::replaceCurves()
{
    RimSummaryPlot* destinationPlot = m_summaryPlot;
    destinationPlot->deleteAllSummaryCurves();
    destinationPlot->ensembleCurveSetCollection()->deleteAllCurveSets();

    appendCurvesToPlot( destinationPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::createNewPlot()
{
    RimSummaryPlot* destinationPlot = RiaSummaryTools::summaryPlotCollection()->createSummaryPlotWithAutoTitle();

    appendCurvesToPlot( destinationPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotManager::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );

        if ( keyEvent && ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return ) )
        {
            auto mods = keyEvent->modifiers();

            if ( mods & Qt::ShiftModifier )
                appendCurves();
            else if ( mods & Qt::ControlModifier )
                replaceCurves();
            else if ( mods & Qt::AltModifier )
                createNewPlot();
        }
    }

    return QObject::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryCaseCollection*>> RimSummaryPlotManager::dataSources() const
{
    auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection();

    auto summaryCases = sumCaseMainColl->topLevelSummaryCases();
    auto ensembles    = sumCaseMainColl->summaryCaseCollections();

    return { summaryCases, ensembles };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::updateUiFromSelection()
{
    auto destinationObject = dynamic_cast<caf::PdmObjectHandle*>( caf::SelectionManager::instance()->selectedItem() );

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
std::set<RifEclipseSummaryAddress> RimSummaryPlotManager::filteredAddresses()
{
    auto [summaryCases, ensembles] = dataSources();

    std::set<RifEclipseSummaryAddress> allAddressesFromSource;

    if ( !summaryCases.empty() )
    {
        allAddressesFromSource = addressesForSource( summaryCases.front() );
    }
    else if ( !ensembles.empty() )
    {
        allAddressesFromSource = addressesForSource( ensembles.front() );
    }

    if ( allAddressesFromSource.empty() ) return {};

    QStringList allCurveAddressFilters = m_curveFilterText().split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

    return computeFilteredAddresses( allCurveAddressFilters, allAddressesFromSource );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotManager::addressesForSource( caf::PdmObject* summarySource )
{
    auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( summarySource );
    if ( ensemble )
    {
        return ensemble->ensembleSummaryAddresses();
    }

    auto sumCase = dynamic_cast<RimSummaryCase*>( summarySource );
    if ( sumCase )
    {
        auto reader = sumCase ? sumCase->summaryReader() : nullptr;
        if ( reader )
        {
            return reader->allResultAddresses();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimSummaryPlotManager::createCurveSet( RimSummaryCaseCollection*       ensemble,
                                                            const RifEclipseSummaryAddress& addr )
{
    auto curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryCaseCollection( ensemble );
    curveSet->setSummaryAddress( addr );

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryPlotManager::createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr )
{
    auto curve = new RimSummaryCurve();

    curve->setSummaryCaseY( summaryCase );
    curve->setSummaryAddressY( addr );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::appendCurvesToPlot( RimSummaryPlot*                               summaryPlot,
                                                const std::set<RifEclipseSummaryAddress>&     addresses,
                                                const std::vector<RimSummaryCase*>&           summaryCases,
                                                const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    for ( const auto& addr : addresses )
    {
        for ( const auto ensemble : ensembles )
        {
            auto curveSet = createCurveSet( ensemble, addr );
            summaryPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );
        }

        for ( const auto summaryCase : summaryCases )
        {
            auto curve = createCurve( summaryCase, addr );

            summaryPlot->addCurveNoUpdate( curve );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::appendCurvesToPlot( RimSummaryPlot* destinationPlot )
{
    CAF_ASSERT( destinationPlot );

    auto [summaryCases, ensembles] = dataSources();

    std::set<RifEclipseSummaryAddress> filteredAddressesFromSource = filteredAddresses();
    appendCurvesToPlot( destinationPlot, filteredAddressesFromSource, summaryCases, ensembles );

    destinationPlot->applyDefaultCurveAppearances();
    destinationPlot->loadDataAndUpdate();

    RiaSummaryTools::summaryPlotCollection()->updateConnectedEditors();

    setFocusToEditorWidget( m_curveFilterText.uiCapability() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotManager::setFocusToEditorWidget( caf::PdmUiFieldHandle* uiFieldHandle )
{
    CAF_ASSERT( uiFieldHandle );

    auto editors = uiFieldHandle->connectedEditors();
    if ( !editors.empty() )
    {
        auto fieldEditorHandle = dynamic_cast<caf::PdmUiFieldEditorHandle*>( editors.front() );
        if ( fieldEditorHandle && fieldEditorHandle->editorWidget() )
        {
            auto widget = fieldEditorHandle->editorWidget();

            // If the dock widget is floating, activateWindow() must be called to make sure the top level widget has
            // focus before the editor widget is given focus
            widget->activateWindow();
            widget->setFocus();
        }
    }
}

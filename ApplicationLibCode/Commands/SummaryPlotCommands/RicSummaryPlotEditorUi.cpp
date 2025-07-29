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

#include "RicSummaryPlotEditorUi.h"

#include "RiaColorTables.h"
#include "RiaCurveSetDefinition.h"
#include "RiaGuiApplication.h"
#include "Summary/RiaSummaryCurveDefinition.h"
#include "Summary/RiaSummaryPlotTools.h"

#include "RiuSummaryCurveDefinitionKeywords.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimMainPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionUi.h"
#include "RiuTools.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>
#include <sstream>

CAF_PDM_SOURCE_INIT( RicSummaryPlotEditorUi, "RicSummaryCurveCreator" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const size_t ENSEMBLE_CURVE_COUNT_THRESHOLD = 600;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RicSummaryPlotEditorUi::CONFIGURATION_NAME = "CurveCreatorCfg";

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
int ensembleCurveCount( const std::set<RiaSummaryCurveDefinition>& allCurveDefs );
template <typename T>
std::vector<T> toVector( const std::set<T>& set );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorUi::RicSummaryPlotEditorUi()
    : m_plotContainer( nullptr )
{
    CAF_PDM_InitFieldNoDefault( &m_targetPlot, "TargetPlot", "Target Plot" );

    m_previewPlot = std::make_unique<RimSummaryPlot>();
    m_previewPlot->setLegendPosition( RiuPlotWidget::Legend::TOP );

    CAF_PDM_InitFieldNoDefault( &m_applyButtonField, "ApplySelection", "" );
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_applyButtonField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_closeButtonField, "Close", "" );
    m_closeButtonField = false;
    m_closeButtonField.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_closeButtonField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_okButtonField, "OK", "" );
    m_okButtonField = false;
    m_okButtonField.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_okButtonField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_summaryCurveSelectionEditor = std::make_unique<RiuSummaryVectorSelectionWidgetCreator>();

    m_summaryCurveSelectionEditor->summaryAddressSelection()->setFieldChangedHandler( [this]() { selectionEditorFieldChanged(); } );
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setMultiSelectionMode( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotEditorUi::~RicSummaryPlotEditorUi()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotEditorUi::previewPlot() const
{
    return m_previewPlot.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::updateFromSummaryPlot( RimSummaryPlot* targetPlot, const std::vector<SummarySource*>& defaultSources )
{
    if ( targetPlot == nullptr || m_targetPlot != targetPlot )
    {
        resetAllFields();
    }

    m_targetPlot = targetPlot;

    if ( m_targetPlot )
    {
        m_plotContainer = m_targetPlot->firstAncestorOfType<RimSummaryMultiPlot>();

        populateCurveCreator( *m_targetPlot );
        syncPreviewCurvesFromUiSelection();
        setInitialCurveVisibility( targetPlot );
        m_previewPlot->loadDataAndUpdate();
    }
    else
    {
        setDefaultCurveSelection( defaultSources );
        m_previewPlot->enableAutoPlotTitle( true );
        m_previewPlot->setPlotTitleVisible( false );
        syncPreviewCurvesFromUiSelection();
        m_plotContainer = nullptr;
    }

    caf::PdmUiItem::updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::updateFromSummaryMultiPlot( RimSummaryMultiPlot*               summaryMultiPlot,
                                                         const std::vector<SummarySource*>& defaultSources )
{
    resetAllFields();

    m_plotContainer = summaryMultiPlot;

    setDefaultCurveSelection( defaultSources );

    caf::PdmUiItem::updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RicSummaryPlotEditorUi::addressSelectionWidget( QWidget* parent )
{
    return m_summaryCurveSelectionEditor->getOrCreateWidget( parent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSummaryPlotEditorUi::isCloseButtonPressed() const
{
    return m_closeButtonField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::clearCloseButton()
{
    m_closeButtonField = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_applyButtonField || changedField == &m_okButtonField )
    {
        if ( m_targetPlot == nullptr )
        {
            createNewPlot();
        }

        updateTargetPlot();

        if ( changedField == &m_okButtonField )
        {
            m_closeButtonField = true;

            RiuPlotMainWindowTools::showPlotMainWindow();
            RiuPlotMainWindowTools::selectAsCurrentItem( m_targetPlot );
            RiuPlotMainWindowTools::setExpanded( m_targetPlot );
        }

        m_applyButtonField = false;
        m_okButtonField    = false;

        caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>*>( m_targetPlot->uiCapability()->objectToggleField() );
        field->setValueWithFieldChanged( true );

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSummaryPlotEditorUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_targetPlot )
    {
        // Create New Plot item
        QString displayName = "( New Plot )";
        options.push_back( caf::PdmOptionItemInfo( displayName, nullptr ) );

        if ( m_plotContainer )
        {
            m_plotContainer->summaryPlotItemInfos( &options );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_targetPlot );
    uiOrdering.add( &m_okButtonField );
    uiOrdering.add( &m_applyButtonField );
    uiOrdering.add( &m_closeButtonField );

    uiOrdering.skipRemainingFields( true );

    syncPreviewCurvesFromUiSelection();

    m_summaryCurveSelectionEditor->updateUi( uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::syncPreviewCurvesFromUiSelection()
{
    std::vector<RiaSummaryCurveDefinition> allCurveDefinitionsVector =
        m_summaryCurveSelectionEditor->summaryAddressSelection()->allCurveDefinitionsFromSelection();

    auto curveSetDefs = m_summaryCurveSelectionEditor->summaryAddressSelection()->allCurveSetDefinitionsFromSelections();
    for ( const auto& curveSet : curveSetDefs )
    {
        allCurveDefinitionsVector.emplace_back( curveSet.ensemble(), curveSet.summaryAddress() );
    }

    std::set<RiaSummaryCurveDefinition> allCurveDefinitions =
        std::set<RiaSummaryCurveDefinition>( allCurveDefinitionsVector.begin(), allCurveDefinitionsVector.end() );

    std::vector<RimSummaryCurve*> currentCurvesInPreviewPlot = m_previewPlot->summaryAndEnsembleCurves();

    {
        std::set<RiaSummaryCurveDefinition> currentCurveDefs;
        std::set<RiaSummaryCurveDefinition> newCurveDefs;
        std::set<RimSummaryCurve*>          curvesToDelete;

        for ( const auto& curve : currentCurvesInPreviewPlot )
        {
            currentCurveDefs.insert( curve->curveDefinition() );
        }

        {
            // Determine which curves to delete from plot
            std::set<RiaSummaryCurveDefinition> deleteCurveDefs;
            std::set_difference( currentCurveDefs.begin(),
                                 currentCurveDefs.end(),
                                 allCurveDefinitions.begin(),
                                 allCurveDefinitions.end(),
                                 std::inserter( deleteCurveDefs, deleteCurveDefs.end() ) );

            for ( const auto& curve : currentCurvesInPreviewPlot )
            {
                RiaSummaryCurveDefinition curveDef = curve->curveDefinition();
                if ( deleteCurveDefs.count( curveDef ) > 0 ) curvesToDelete.insert( curve );
            }
        }

        {
            // Determine which curves are new since last time
            std::set_difference( allCurveDefinitions.begin(),
                                 allCurveDefinitions.end(),
                                 currentCurveDefs.begin(),
                                 currentCurveDefs.end(),
                                 std::inserter( newCurveDefs, newCurveDefs.end() ) );
        }

        // Curve sets to delete
        std::set<RimEnsembleCurveSet*> curveSetsToDelete;
        {
            std::vector<RiaCurveSetDefinition> allCurveSetDefinitionsVector =
                m_summaryCurveSelectionEditor->summaryAddressSelection()->allCurveSetDefinitionsFromSelections();
            std::set<RiaCurveSetDefinition> allCurveSetDefinitions =
                std::set<RiaCurveSetDefinition>( allCurveSetDefinitionsVector.begin(), allCurveSetDefinitionsVector.end() );
            std::vector<RimEnsembleCurveSet*> currentCurveSetsInPreviewPlot = m_previewPlot->curveSets();
            std::set<RiaCurveSetDefinition>   currentCurveSetDefs;

            for ( const auto& curveSet : currentCurveSetsInPreviewPlot )
            {
                RimSummaryEnsemble* ensemble = curveSet->summaryEnsemble();
                currentCurveSetDefs.insert( RiaCurveSetDefinition( ensemble, curveSet->summaryAddressY() ) );
            }

            if ( allCurveSetDefinitions.size() < currentCurveSetsInPreviewPlot.size() )
            {
                // Determine which curves to delete from plot
                std::set<RiaCurveSetDefinition> deleteCurveSetDefs;
                std::set_difference( currentCurveSetDefs.begin(),
                                     currentCurveSetDefs.end(),
                                     allCurveSetDefinitions.begin(),
                                     allCurveSetDefinitions.end(),
                                     std::inserter( deleteCurveSetDefs, deleteCurveSetDefs.end() ) );

                for ( const auto& curveSet : currentCurveSetsInPreviewPlot )
                {
                    RimSummaryEnsemble*   ensemble    = curveSet->summaryEnsemble();
                    RiaCurveSetDefinition curveSetDef = RiaCurveSetDefinition( ensemble, curveSet->summaryAddressY() );
                    if ( deleteCurveSetDefs.count( curveSetDef ) > 0 ) curveSetsToDelete.insert( curveSet );
                }
            }
        }

        updatePreviewCurvesFromCurveDefinitions( allCurveDefinitions, newCurveDefs, curvesToDelete, curveSetsToDelete );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::updatePreviewCurvesFromCurveDefinitions( const std::set<RiaSummaryCurveDefinition>& allCurveDefsToDisplay,
                                                                      const std::set<RiaSummaryCurveDefinition>& curveDefsToAdd,
                                                                      const std::set<RimSummaryCurve*>&          curvesToDelete,
                                                                      const std::set<RimEnsembleCurveSet*>&      curveSetsToDelete )
{
    static bool warningDisplayed = false;

    std::set<RiaSummaryCurveDefinition> summaryCurveDefsToDisplay;

    // Ignore curve sets when assigning colors to singe summary curves
    for ( const auto& def : allCurveDefsToDisplay )
    {
        if ( !def.isEnsembleCurve() ) summaryCurveDefsToDisplay.insert( def );
    }

    RimSummaryCurveAppearanceCalculator curveLookCalc( summaryCurveDefsToDisplay );

    // Delete curves
    if ( !curveSetsToDelete.empty() )
    {
        m_previewPlot->ensembleCurveSetCollection()->deleteCurveSets( toVector( curveSetsToDelete ) );
    }
    if ( !curvesToDelete.empty() )
    {
        m_previewPlot->deleteCurves( toVector( curvesToDelete ) );
    }

    size_t ensembleCurveCnt = ensembleCurveCount( allCurveDefsToDisplay );

    bool speedCheatsRequired = ensembleCurveCnt > ENSEMBLE_CURVE_COUNT_THRESHOLD;
    bool legendsVisible      = m_previewPlot->legendsVisible();

    // Disable legends when adding curves
    if ( speedCheatsRequired ) m_previewPlot->setLegendsVisible( false );

    // Add new curves
    std::map<RimSummaryCurve*, std::pair<bool, bool>> stashedErrorBarsAndLegendVisibility;
    for ( const auto& curveDef : curveDefsToAdd )
    {
        RimSummaryCase* currentCase = curveDef.summaryCaseY();

        if ( curveDef.isEnsembleCurve() )
        {
            // Find curveSet
            RimEnsembleCurveSet* curveSet = nullptr;
            for ( const auto& cs : m_previewPlot->ensembleCurveSetCollection()->curveSets() )
            {
                if ( cs->summaryEnsemble() == curveDef.ensemble() && cs->summaryAddressY() == curveDef.summaryAddressY() )
                {
                    curveSet = cs;
                    break;
                }
            }
            if ( !curveSet )
            {
                curveSet = new RimEnsembleCurveSet();
                curveSet->disableStatisticCurves();
                curveSet->setSummaryEnsemble( curveDef.ensemble() );

                // Do not call setSummaryAddressAndStatisticsFlag() here, as the call to m_statistics->updateAllRequiredEditors(); causes a
                // crash in updateUiOrdering. The statistics curves will be created when the curve set is added to the plot.
                curveSet->setSummaryAddressY( curveDef.summaryAddressY() );

                // Set single curve set color
                auto   allCurveSets = m_previewPlot->ensembleCurveSetCollection()->curveSets();
                size_t colorIndex =
                    std::count_if( allCurveSets.begin(),
                                   allCurveSets.end(),
                                   []( RimEnsembleCurveSet* curveSet )
                                   { return RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ); } );
                curveSet->setColor( RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( colorIndex ) );

                // Add curve to plot
                m_previewPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );

                if ( m_previewPlot->ensembleCurveSetCollection()->curveSets().size() > 1 && ensembleCurveCnt > ENSEMBLE_CURVE_COUNT_THRESHOLD )
                {
                    // Toggle off new curve set and display warning
                    curveSet->showCurves( false );

                    if ( !warningDisplayed )
                    {
                        QMessageBox mbox;
                        mbox.setIcon( QMessageBox::Icon::Warning );
                        mbox.setInformativeText( "The new curve set is hidden. Too many visible curve sets may lead to poor performance" );
                        mbox.exec();
                        warningDisplayed = true;
                    }
                }
            }
        }
        else
        {
            auto curve = RiaSummaryPlotTools::createCurve( currentCase, curveDef.summaryAddressY() );

            if ( speedCheatsRequired )
            {
                stashedErrorBarsAndLegendVisibility[curve] = std::make_pair( curve->errorBarsVisible(), curve->showInLegend() );
                curve->setErrorBarsVisible( false );
                curve->setShowInLegend( false );
            }
            if ( currentCase && currentCase->isObservedData() ) curve->setSymbolSkipDistance( 0 );

            m_previewPlot->addCurveNoUpdate( curve );
            curveLookCalc.setupCurveLook( curve );
        }
    }

    // Enable legends if there is not too many curves
    if ( speedCheatsRequired && !warningDisplayed )
    {
        m_previewPlot->setLegendsVisible( legendsVisible );

        for ( const auto& curveAndVisibilityPair : stashedErrorBarsAndLegendVisibility )
        {
            auto curve                        = curveAndVisibilityPair.first;
            auto errorBarsAndLegendVisibility = curveAndVisibilityPair.second;
            curve->setErrorBarsVisible( errorBarsAndLegendVisibility.first );
            curve->setShowInLegend( errorBarsAndLegendVisibility.second );
        }
    }
    m_previewPlot->loadDataAndUpdate();
    m_previewPlot->zoomAll();
    m_previewPlot->updateConnectedEditors();
    m_previewPlot->summaryCurveCollection()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_applyButtonField == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Apply";
        }
    }
    else if ( &m_closeButtonField == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Cancel";
        }
    }
    else if ( &m_okButtonField == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "OK";
        }
    }
    else if ( &m_targetPlot == field )
    {
        caf::PdmUiComboBoxEditorAttribute* attrib = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->adjustWidthToContents = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Populate curve creator from the given curve collection
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::populateCurveCreator( const RimSummaryPlot& sourceSummaryPlot )
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;

    m_previewPlot->deleteAllSummaryCurves();
    m_previewPlot->ensembleCurveSetCollection()->deleteAllCurveSets();

    for ( const auto& curve : sourceSummaryPlot.summaryCurves() )
    {
        curveDefs.push_back( curve->curveDefinition() );

        // Copy curve object to the preview plot
        copyCurveAndAddToPlot( curve, m_previewPlot.get(), true );
    }

    RimEnsembleCurveSetCollection* previewCurveSetColl = m_previewPlot->ensembleCurveSetCollection();
    for ( const auto& curveSet : sourceSummaryPlot.ensembleCurveSetCollection()->curveSets() )
    {
        RimEnsembleCurveSet* newCurveSet = curveSet->clone();
        newCurveSet->disableStatisticCurves();
        previewCurveSetColl->addCurveSet( newCurveSet );

        RimSummaryEnsemble* ensemble = curveSet->summaryEnsemble();
        curveDefs.emplace_back( ensemble, curveSet->summaryAddressY() );
    }

    m_previewPlot->copyAxisPropertiesFromOther( sourceSummaryPlot );
    m_previewPlot->enableAutoPlotTitle( sourceSummaryPlot.autoPlotTitle() );
    m_previewPlot->updatePlotTitle();
    m_previewPlot->updateAxes();

    if ( curveDefs.empty() )
    {
        auto sumCases = RimProject::current()->allSummaryCases();
        if ( !sumCases.empty() )
        {
            RifEclipseSummaryAddress  defaultAdr;
            RiaSummaryCurveDefinition curveDef( sumCases.front(), defaultAdr, false );
            curveDefs.push_back( curveDef );
        }
    }

    m_summaryCurveSelectionEditor->summaryAddressSelection()->setSelectedCurveDefinitions( curveDefs );
}

//--------------------------------------------------------------------------------------------------
/// Copy curves from preview plot to target plot
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::updateTargetPlot()
{
    if ( !m_targetPlot ) return;

    m_targetPlot->deleteAllSummaryCurves();
    m_targetPlot->ensembleCurveSetCollection()->deleteAllCurveSets();

    // Add edited curves to target plot
    for ( const auto& editedCurve : m_previewPlot->summaryCurves() )
    {
        if ( !editedCurve->isChecked() )
        {
            continue;
        }
        copyCurveAndAddToPlot( editedCurve, m_targetPlot );
    }

    for ( const auto& editedCurveSet : m_previewPlot->ensembleCurveSetCollection()->curveSets() )
    {
        if ( !editedCurveSet->isCurvesVisible() )
        {
            continue;
        }

        RimEnsembleCurveSet* newCurveSet = editedCurveSet->clone();
        m_targetPlot->ensembleCurveSetCollection()->addCurveSet( newCurveSet );
        newCurveSet->setParentPlotNoReplot( m_targetPlot->plotWidget() );
    }

    m_targetPlot->loadDataAndUpdate();

    m_targetPlot->updateConnectedEditors();
    m_targetPlot->curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::copyCurveAndAddToPlot( const RimSummaryCurve* curve, RimSummaryPlot* plot, bool forceVisible )
{
    auto curveCopy = curve->copyObject<RimSummaryCurve>();
    CVF_ASSERT( curveCopy );

    if ( forceVisible )
    {
        curveCopy->setCheckState( true );
    }

    curveCopy->setSummaryCaseY( curve->summaryCaseY() );

    bool autoAssignAxis = true;
    plot->addCurveNoUpdate( curveCopy, autoAssignAxis );

    curveCopy->initAfterReadRecursively();
    curveCopy->loadDataAndUpdate( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::setDefaultCurveSelection( const std::vector<SummarySource*>& defaultSources )
{
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setDefaultSelection( defaultSources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::resetAllFields()
{
    std::vector<RiaSummaryCurveDefinition> curveDefinitions;
    m_summaryCurveSelectionEditor->summaryAddressSelection()->setSelectedCurveDefinitions( curveDefinitions );

    m_previewPlot->deleteAllSummaryCurves();
    m_targetPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::applyAppearanceToAllPreviewCurves()
{
    std::set<RiaSummaryCurveDefinition> allCurveDefs = m_previewPlot->summaryAndEnsembleCurveDefinitions();

    RimSummaryCurveAppearanceCalculator curveLookCalc( allCurveDefs );

    // Summary curves
    for ( auto& curve : m_previewPlot->summaryCurves() )
    {
        curve->resetAppearance();
        curveLookCalc.setupCurveLook( curve );
    }

    // Ensemble curve sets
    int colorIndex = 0;
    for ( auto& curveSet : m_previewPlot->ensembleCurveSetCollection()->curveSets() )
    {
        if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ) )
        {
            curveSet->setColor( RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( colorIndex++ ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::createNewPlot()
{
    RimSummaryPlot* newSummaryPlot = nullptr;

    if ( !m_plotContainer )
    {
        std::vector<RimSummaryPlot*> plots;
        m_plotContainer = RiaSummaryPlotTools::createAndAppendSummaryMultiPlot( plots );
    }

    if ( m_plotContainer )
    {
        newSummaryPlot = new RimSummaryPlot();
        newSummaryPlot->setAsPlotMdiWindow();
        newSummaryPlot->enableAutoPlotTitle( true );
        m_plotContainer->addPlot( newSummaryPlot );
    }

    if ( newSummaryPlot )
    {
        newSummaryPlot->loadDataAndUpdate();

        if ( m_plotContainer )
        {
            m_plotContainer->updateConnectedEditors();
        }

        m_targetPlot = newSummaryPlot;

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::updateCurveNames()
{
    for ( RimSummaryCurve* curve : m_previewPlot->summaryCurves() )
    {
        curve->updateCurveNameNoLegendUpdate();
    }

    if ( m_previewPlot && m_previewPlot->plotWidget() ) m_previewPlot->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSummaryPlotEditorUi::isObservedData( RimSummaryCase* sumCase ) const
{
    return dynamic_cast<RimObservedSummaryData*>( sumCase ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::selectionEditorFieldChanged()
{
    syncPreviewCurvesFromUiSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotEditorUi::setInitialCurveVisibility( const RimSummaryPlot* targetPlot )
{
    // Set visibility for imported curves which were not checked in source plot
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress>> sourceCurveDefs;
    for ( const auto& curve : targetPlot->summaryCurves() )
    {
        sourceCurveDefs.insert( std::make_pair( curve->summaryCaseY(), curve->summaryAddressY() ) );
    }

    for ( const auto& curve : m_previewPlot->summaryCurves() )
    {
        auto curveDef = std::make_pair( curve->summaryCaseY(), curve->summaryAddressY() );
        if ( sourceCurveDefs.count( curveDef ) == 0 )
        {
            curve->setCheckState( false );
        }
    }

    std::set<std::pair<RimSummaryEnsemble*, RifEclipseSummaryAddress>> sourceCurveSetDefs;
    for ( const auto& curveSet : targetPlot->ensembleCurveSetCollection()->curveSets() )
    {
        sourceCurveSetDefs.insert( std::make_pair( curveSet->summaryEnsemble(), curveSet->summaryAddressY() ) );
    }

    for ( const auto& curveSet : m_previewPlot->ensembleCurveSetCollection()->curveSets() )
    {
        auto curveDef = std::make_pair( curveSet->summaryEnsemble(), curveSet->summaryAddressY() );
        if ( sourceCurveSetDefs.count( curveDef ) == 0 )
        {
            curveSet->showCurves( false );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int ensembleCurveCount( const std::set<RiaSummaryCurveDefinition>& allCurveDefs )
{
    return std::count_if( allCurveDefs.begin(),
                          allCurveDefs.end(),
                          []( const RiaSummaryCurveDefinition& def ) { return def.isEnsembleCurve(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> toVector( const std::set<T>& set )
{
    return std::vector<T>( set.begin(), set.end() );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimSummaryPlotSourceStepping.h"

#include "RiaGuiApplication.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryCurveAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifSummaryReaderInterface.h"

#include "RimDataSourceSteppingTools.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiToolBarEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping()
    : m_sourceSteppingType( Y_AXIS )
{
    CAF_PDM_InitObject( "Summary Curves Modifier", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveCase", "Case", "", "", "" );

    CAF_PDM_InitField( &m_includeEnsembleCasesForCaseStepping,
                       "IncludeEnsembleCasesForCaseStepping",
                       false,
                       "Allow Stepping on Ensemble cases",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellGroupName, "GroupName", "Group Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_region, "Region", "Region", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_quantity, "Quantities", "Quantity", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_cellBlock, "CellBlock", "Block", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_segment, "Segment", "Segment", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_completion, "Completion", "Completion", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_aquifer, "Aquifer", "Aquifer", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_placeholderForLabel, "Placeholder", "", "", "", "" );
    m_placeholderForLabel = "No common identifiers detected";
    m_placeholderForLabel.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_placeholderForLabel.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setSourceSteppingType( SourceSteppingType sourceSteppingType )
{
    m_sourceSteppingType = sourceSteppingType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextCase()
{
    modifyCurrentIndex( &m_summaryCase, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevCase()
{
    modifyCurrentIndex( &m_summaryCase, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextQuantity()
{
    if ( !m_quantity.uiCapability()->isUiHidden() )
    {
        modifyCurrentIndex( &m_quantity, 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevQuantity()
{
    if ( !m_quantity.uiCapability()->isUiHidden() )
    {
        modifyCurrentIndex( &m_quantity, -1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextOtherIdentifier()
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    modifyCurrentIndex( valueField, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevOtherIdentifier()
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    modifyCurrentIndex( valueField, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::fieldsToShowInToolbar()
{
    return computeVisibleFieldsAndSetFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto visible = computeVisibleFieldsAndSetFieldVisibility();
    if ( visible.empty() )
    {
        m_placeholderForLabel.uiCapability()->setUiHidden( false );
    }
    else
    {
        m_placeholderForLabel.uiCapability()->setUiHidden( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryPlotSourceStepping::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_includeEnsembleCasesForCaseStepping )
    {
        return caf::PdmObject::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    }
    else if ( fieldNeedingOptions == &m_placeholderForLabel )
    {
        return options;
    }
    else if ( fieldNeedingOptions == &m_summaryCase )
    {
        auto summaryCases = RimSummaryPlotSourceStepping::summaryCasesForSourceStepping();
        for ( auto sumCase : summaryCases )
        {
            options.append( caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase ) );
        }

        return options;
    }
    else if ( fieldNeedingOptions == &m_ensemble )
    {
        RimProject* proj = RimProject::current();
        for ( auto ensemble : proj->summaryGroups() )
        {
            if ( ensemble->isEnsemble() )
            {
                options.append( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
            }
        }

        return options;
    }

    auto addresses = adressesForSourceStepping();
    if ( !addresses.empty() )
    {
        if ( fieldNeedingOptions == &m_quantity )
        {
            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_FIELD;

            auto visibleCurveAddresses = addressesForCurvesInPlot();
            if ( !visibleCurveAddresses.empty() )
            {
                category = visibleCurveAddresses.begin()->category();
            }

            std::map<QString, QString> displayAndValueStrings;

            {
                RiaSummaryCurveAnalyzer quantityAnalyzer;

                auto subset = RiaSummaryCurveAnalyzer::addressesForCategory( addresses, category );
                quantityAnalyzer.appendAddresses( subset );

                RiaSummaryCurveAnalyzer analyzerForVisibleCurves;
                analyzerForVisibleCurves.appendAddresses( visibleCurveAddresses );

                if ( analyzerForVisibleCurves.quantityNamesWithHistory().empty() )
                {
                    auto quantities = quantityAnalyzer.quantities();
                    for ( const auto& s : quantities )
                    {
                        QString valueString = QString::fromStdString( s );

                        displayAndValueStrings[valueString] = valueString;
                    }
                }
                else
                {
                    // The plot displays a mix of simulated and observed vectors
                    // Create a combined item for source stepping

                    auto quantitiesWithHistory = quantityAnalyzer.quantityNamesWithHistory();
                    for ( const auto& s : quantitiesWithHistory )
                    {
                        QString valueString   = QString::fromStdString( s );
                        QString displayString = valueString + " (H)";

                        displayAndValueStrings[displayString] = valueString;
                    }

                    auto quantitiesNoHistory = quantityAnalyzer.quantityNamesNoHistory();
                    for ( const auto& s : quantitiesNoHistory )
                    {
                        QString valueString = QString::fromStdString( s );

                        displayAndValueStrings[valueString] = valueString;
                    }
                }
            }

            for ( const auto& displayAndValue : displayAndValueStrings )
            {
                options.append( caf::PdmOptionItemInfo( displayAndValue.first, displayAndValue.second ) );
            }

            if ( options.isEmpty() )
            {
                options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
            }
        }
        else
        {
            RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_INVALID;
            std::string                                  secondaryIdentifier;

            if ( fieldNeedingOptions == &m_wellName )
            {
                category = RifEclipseSummaryAddress::SUMMARY_WELL;
            }
            else if ( fieldNeedingOptions == &m_region )
            {
                category = RifEclipseSummaryAddress::SUMMARY_REGION;
            }
            else if ( fieldNeedingOptions == &m_wellGroupName )
            {
                category = RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
            }
            else if ( fieldNeedingOptions == &m_cellBlock )
            {
                category = RifEclipseSummaryAddress::SUMMARY_BLOCK;
            }
            else if ( fieldNeedingOptions == &m_segment )
            {
                secondaryIdentifier = m_wellName().toStdString();
                category            = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
            }
            else if ( fieldNeedingOptions == &m_completion )
            {
                secondaryIdentifier = m_wellName().toStdString();
                category            = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
            }
            else if ( fieldNeedingOptions == &m_aquifer )
            {
                category = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
            }

            std::vector<QString> identifierTexts;

            if ( category != RifEclipseSummaryAddress::SUMMARY_INVALID )
            {
                RiaSummaryCurveAnalyzer analyzer;
                analyzer.appendAddresses( addresses );

                identifierTexts = analyzer.identifierTexts( category, secondaryIdentifier );
            }

            if ( !identifierTexts.empty() )
            {
                for ( const auto& text : identifierTexts )
                {
                    options.append( caf::PdmOptionItemInfo( text, text ) );
                }
            }
            else
            {
                options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                     const QVariant&            oldValue,
                                                     const QVariant&            newValue )
{
    std::vector<RimSummaryCurve*> curves;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType( curveCollection );
    if ( curveCollection )
    {
        curves = curveCollection->curves();
    }

    RimEnsembleCurveSetCollection* ensembleCurveColl = nullptr;
    this->firstAncestorOrThisOfType( ensembleCurveColl );

    if ( changedField == &m_includeEnsembleCasesForCaseStepping )
    {
        if ( curveCollection )
        {
            curveCollection->updateConnectedEditors();
        }

        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RiuPlotMainWindow* mainPlotWindow               = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        bool               forceUpdateOfFieldsInToolbar = true;
        mainPlotWindow->updateSummaryPlotToolBar( forceUpdateOfFieldsInToolbar );

        return;
    }

    bool triggerLoadDataAndUpdate = false;

    if ( changedField == &m_summaryCase )
    {
        if ( m_summaryCase() )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCase*                       previousCase = dynamic_cast<RimSummaryCase*>( variantHandle.p() );

            for ( auto curve : curves )
            {
                if ( isYAxisStepping() )
                {
                    if ( previousCase == curve->summaryCaseY() )
                    {
                        curve->setSummaryCaseY( m_summaryCase );
                        curve->setCurveAppearanceFromCaseType();
                    }
                }

                if ( isXAxisStepping() )
                {
                    if ( previousCase == curve->summaryCaseX() )
                    {
                        curve->setSummaryCaseX( m_summaryCase );
                    }
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_wellGroupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_quantity.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_ensemble )
    {
        if ( m_ensemble() && ensembleCurveColl )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCaseCollection* previousCollection = dynamic_cast<RimSummaryCaseCollection*>( variantHandle.p() );

            for ( auto curveSet : ensembleCurveColl->curveSets() )
            {
                if ( curveSet->summaryCaseCollection() == previousCollection )
                {
                    curveSet->setSummaryCaseCollection( m_ensemble );
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_wellGroupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_quantity.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_quantity )
    {
        for ( auto curve : curves )
        {
            if ( isYAxisStepping() )
            {
                auto adr = curve->summaryAddressY();
                updateHistoryAndSummaryQuantityIfMatching( oldValue, newValue, &adr );
                curve->setSummaryAddressY( adr );
            }

            if ( isXAxisStepping() )
            {
                auto adr = curve->summaryAddressX();
                updateHistoryAndSummaryQuantityIfMatching( oldValue, newValue, &adr );
                curve->setSummaryAddressX( adr );
            }
        }

        if ( ensembleCurveColl )
        {
            for ( auto curveSet : ensembleCurveColl->curveSets() )
            {
                auto adr = curveSet->summaryAddress();
                updateHistoryAndSummaryQuantityIfMatching( oldValue, newValue, &adr );
                curveSet->setSummaryAddress( adr );
            }
        }

        triggerLoadDataAndUpdate = true;
    }

    {
        RifEclipseSummaryAddress::SummaryVarCategory summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_INVALID;
        if ( changedField == &m_wellName )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL;
        }
        else if ( changedField == &m_region )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_REGION;
        }
        else if ( changedField == &m_wellGroupName )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL_GROUP;
        }
        else if ( changedField == &m_cellBlock )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_BLOCK;
        }
        else if ( changedField == &m_segment )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT;
        }
        else if ( changedField == &m_completion )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION;
        }
        else if ( changedField == &m_aquifer )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_AQUIFER;
        }

        if ( summaryCategoryToModify != RifEclipseSummaryAddress::SUMMARY_INVALID )
        {
            for ( auto curve : curves )
            {
                if ( isYAxisStepping() )
                {
                    RifEclipseSummaryAddress adr = curve->summaryAddressY();
                    updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curve->setSummaryAddressY( adr );
                }

                if ( isXAxisStepping() )
                {
                    RifEclipseSummaryAddress adr = curve->summaryAddressX();
                    updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curve->setSummaryAddressX( adr );
                }
            }

            if ( ensembleCurveColl )
            {
                for ( auto curveSet : ensembleCurveColl->curveSets() )
                {
                    auto adr = curveSet->summaryAddress();
                    updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curveSet->setSummaryAddress( adr );
                }
            }

            triggerLoadDataAndUpdate = true;
        }
    }

    if ( triggerLoadDataAndUpdate )
    {
        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( summaryPlot );

        summaryPlot->updatePlotTitle();
        summaryPlot->loadDataAndUpdate();
        summaryPlot->updateConnectedEditors();

        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>( summaryPlot );
        if ( summaryCrossPlot )
        {
            // Trigger update of curve collection (and summary toolbar in main window), as the visibility of combo
            // boxes might have been changed due to the updates in this function
            if ( curveCollection )
            {
                curveCollection->updateConnectedEditors();
            }

            RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateSummaryPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmValueField* RimSummaryPlotSourceStepping::fieldToModify()
{
    RiaSummaryCurveAnalyzer analyzer;
    analyzer.appendAddresses( addressesForCurvesInPlot() );

    if ( analyzer.wellNames().size() == 1 )
    {
        return &m_wellName;
    }

    if ( analyzer.wellGroupNames().size() == 1 )
    {
        return &m_wellGroupName;
    }

    if ( analyzer.regionNumbers().size() == 1 )
    {
        return &m_region;
    }

    if ( analyzer.blocks().size() == 1 )
    {
        return &m_cellBlock;
    }

    if ( analyzer.wellNames().size() == 1 )
    {
        auto wellName = *( analyzer.wellNames().begin() );

        if ( analyzer.wellSegmentNumbers( wellName ).size() == 1 )
        {
            return &m_segment;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::adressesForSourceStepping() const
{
    std::set<RifEclipseSummaryAddress> addressSet;

    {
        RimEnsembleCurveSetCollection* ensembleCollection = nullptr;
        this->firstAncestorOrThisOfType( ensembleCollection );
        if ( ensembleCollection )
        {
            auto curveSets = ensembleCollection->curveSetsForSourceStepping();
            for ( const RimEnsembleCurveSet* curveSet : curveSets )
            {
                if ( curveSet && curveSet->summaryCaseCollection() )
                {
                    auto addresses = curveSet->summaryCaseCollection()->ensembleSummaryAddresses();
                    addressSet.insert( addresses.begin(), addresses.end() );
                }
            }
        }
    }

    {
        RimSummaryCurveCollection* curveCollection = nullptr;
        this->firstAncestorOrThisOfType( curveCollection );
        if ( curveCollection )
        {
            for ( auto curve : curveCollection->curvesForSourceStepping( m_sourceSteppingType ) )
            {
                if ( !curve ) continue;

                if ( isYAxisStepping() && curve->summaryCaseY() && curve->summaryCaseY()->summaryReader() )
                {
                    auto addresses = curve->summaryCaseY()->summaryReader()->allResultAddresses();
                    addressSet.insert( addresses.begin(), addresses.end() );
                }

                if ( isXAxisStepping() && curve->summaryCaseX() && curve->summaryCaseX()->summaryReader() )
                {
                    auto addresses = curve->summaryCaseX()->summaryReader()->allResultAddresses();
                    addressSet.insert( addresses.begin(), addresses.end() );
                }
            }
        }
    }

    return addressSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::addressesForCurvesInPlot() const
{
    std::set<RifEclipseSummaryAddress> addresses;

    RimEnsembleCurveSetCollection* ensembleCollection = nullptr;
    this->firstAncestorOrThisOfType( ensembleCollection );
    if ( ensembleCollection )
    {
        auto curveSets = ensembleCollection->curveSetsForSourceStepping();
        for ( const RimEnsembleCurveSet* curveSet : curveSets )
        {
            addresses.insert( curveSet->summaryAddress() );
        }
    }

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType( curveCollection );
    if ( curveCollection )
    {
        auto curves = curveCollection->curvesForSourceStepping( m_sourceSteppingType );
        for ( auto c : curves )
        {
            if ( isYAxisStepping() )
            {
                addresses.insert( c->summaryAddressY() );
            }

            if ( isXAxisStepping() )
            {
                addresses.insert( c->summaryAddressX() );
            }
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimSummaryPlotSourceStepping::summaryCasesCurveCollection() const
{
    std::set<RimSummaryCase*> sumCases;

    RimSummaryCurveCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType( curveCollection );

    if ( !curveCollection ) return sumCases;

    auto curves = curveCollection->curvesForSourceStepping( m_sourceSteppingType );
    for ( auto c : curves )
    {
        if ( isYAxisStepping() )
        {
            sumCases.insert( c->summaryCaseY() );
        }

        if ( isXAxisStepping() )
        {
            sumCases.insert( c->summaryCaseX() );
        }
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::computeVisibleFieldsAndSetFieldVisibility()
{
    // Required to set all stepping controls hidden, as they show up in the property editor when selecting a plot
    m_summaryCase.uiCapability()->setUiHidden( true );
    m_includeEnsembleCasesForCaseStepping.uiCapability()->setUiHidden( true );
    m_wellName.uiCapability()->setUiHidden( true );
    m_wellGroupName.uiCapability()->setUiHidden( true );
    m_region.uiCapability()->setUiHidden( true );
    m_quantity.uiCapability()->setUiHidden( true );
    m_ensemble.uiCapability()->setUiHidden( true );
    m_cellBlock.uiCapability()->setUiHidden( true );
    m_segment.uiCapability()->setUiHidden( true );
    m_completion.uiCapability()->setUiHidden( true );
    m_aquifer.uiCapability()->setUiHidden( true );

    std::vector<caf::PdmFieldHandle*> fields;

    auto sumCases = summaryCasesCurveCollection();
    if ( sumCases.size() == 1 )
    {
        RimProject* proj = RimProject::current();
        if ( proj->allSummaryCases().size() > 1 )
        {
            m_summaryCase = *( sumCases.begin() );

            m_summaryCase.uiCapability()->setUiHidden( false );

            fields.push_back( &m_summaryCase );

            m_includeEnsembleCasesForCaseStepping.uiCapability()->setUiHidden( false );
        }
    }

    auto ensembleColl = ensembleCollection();
    if ( ensembleColl.size() == 1 )
    {
        RimProject* proj = RimProject::current();

        if ( proj->summaryGroups().size() > 1 )
        {
            m_ensemble = *( ensembleColl.begin() );

            m_ensemble.uiCapability()->setUiHidden( false );

            fields.push_back( &m_ensemble );
        }
    }

    std::vector<caf::PdmFieldHandle*> fieldsCommonForAllCurves;

    {
        RiaSummaryCurveAnalyzer analyzer;
        analyzer.appendAddresses( addressesForCurvesInPlot() );

        RifEclipseSummaryAddress::SummaryVarCategory category = RifEclipseSummaryAddress::SUMMARY_INVALID;

        if ( !analyzer.categories().empty() )
        {
            if ( analyzer.categories().size() == 1 )
            {
                category = *( analyzer.categories().begin() );
            }
            else
            {
                bool allCategoriesAreDependingOnWellName = true;
                for ( auto c : analyzer.categories() )
                {
                    if ( !RifEclipseSummaryAddress::isDependentOnWellName( c ) )
                    {
                        allCategoriesAreDependingOnWellName = false;
                    }
                }

                if ( allCategoriesAreDependingOnWellName )
                {
                    category = RifEclipseSummaryAddress::SUMMARY_WELL;
                }
            }
        }

        if ( category != RifEclipseSummaryAddress::SUMMARY_INVALID )
        {
            if ( analyzer.wellNames().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.wellNames().begin() ) );
                m_wellName  = txt;
                m_wellName.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_wellName );
            }

            if ( analyzer.wellGroupNames().size() == 1 )
            {
                QString txt     = QString::fromStdString( *( analyzer.wellGroupNames().begin() ) );
                m_wellGroupName = txt;
                m_wellGroupName.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_wellGroupName );
            }

            if ( analyzer.regionNumbers().size() == 1 )
            {
                m_region = *( analyzer.regionNumbers().begin() );
                m_region.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_region );
            }

            if ( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt = QString::number( *( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).begin() ) );
                m_segment   = txt;
                m_segment.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_segment );
            }

            if ( analyzer.blocks().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.blocks().begin() ) );
                m_cellBlock = txt;
                m_cellBlock.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_cellBlock );
            }

            if ( analyzer.wellCompletions( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.wellCompletions( m_wellName().toStdString() ).begin() ) );
                m_completion = txt;
                m_completion.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_completion );
            }

            if ( analyzer.aquifers().size() == 1 )
            {
                m_aquifer = *( analyzer.aquifers().begin() );
                m_aquifer.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_aquifer );
            }

            if ( !analyzer.quantityNameForTitle().empty() )
            {
                QString txt = QString::fromStdString( analyzer.quantityNameForTitle() );
                m_quantity  = txt;
                m_quantity.uiCapability()->setUiHidden( false );

                fieldsCommonForAllCurves.push_back( &m_quantity );
            }
        }
    }

    for ( const auto& f : fieldsCommonForAllCurves )
    {
        fields.push_back( f );
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCaseCollection*> RimSummaryPlotSourceStepping::ensembleCollection() const
{
    std::set<RimSummaryCaseCollection*> sumCases;

    RimEnsembleCurveSetCollection* curveCollection = nullptr;
    this->firstAncestorOrThisOfType( curveCollection );

    if ( !curveCollection ) return sumCases;

    auto curves = curveCollection->curveSets();
    for ( auto c : curves )
    {
        sumCases.insert( c->summaryCaseCollection() );
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isXAxisStepping() const
{
    if ( m_sourceSteppingType == UNION_X_Y_AXIS ) return true;

    if ( m_sourceSteppingType == X_AXIS ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isYAxisStepping() const
{
    if ( m_sourceSteppingType == UNION_X_Y_AXIS ) return true;

    if ( m_sourceSteppingType == Y_AXIS ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAnalyzer* RimSummaryPlotSourceStepping::analyzerForReader( RifSummaryReaderInterface* reader )
{
    if ( !reader ) return nullptr;

    if ( m_curveAnalyzerForReader.first != reader )
    {
        RiaSummaryCurveAnalyzer analyzer;
        m_curveAnalyzerForReader = std::make_pair( reader, analyzer );
    }

    m_curveAnalyzerForReader.second.appendAddresses( reader->allResultAddresses() );

    return &m_curveAnalyzerForReader.second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::modifyCurrentIndex( caf::PdmValueField* valueField, int indexOffset )
{
    bool                          useOptionsOnly;
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions( valueField, &useOptionsOnly );
    RimDataSourceSteppingTools::modifyCurrentIndex( valueField, options, indexOffset );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::updateAddressIfMatching( const QVariant&                              oldValue,
                                                            const QVariant&                              newValue,
                                                            RifEclipseSummaryAddress::SummaryVarCategory category,
                                                            RifEclipseSummaryAddress*                    adr )
{
    if ( !adr ) return false;

    if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();

        if ( adr->regionNumber() == oldInt )
        {
            adr->setRegion( newInt );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();

        if ( adr->aquiferNumber() == oldInt )
        {
            adr->setAquiferNumber( newInt );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_GROUP )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();

        if ( adr->wellGroupName() == oldString )
        {
            adr->setWellGroupName( newString );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();

        if ( adr->wellName() == oldString )
        {
            adr->setWellName( newString );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_BLOCK ||
              category == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();
        if ( adr->blockAsString() == oldString )
        {
            adr->setCellIjk( newString );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT )
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();
        if ( adr->wellSegmentNumber() == oldInt )
        {
            adr->setWellSegmentNumber( newInt );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::updateHistoryAndSummaryQuantityIfMatching( const QVariant&           oldValue,
                                                                              const QVariant&           newValue,
                                                                              RifEclipseSummaryAddress* adr )
{
    if ( !adr ) return false;

    std::string oldString = oldValue.toString().toStdString();
    std::string newString = newValue.toString().toStdString();

    if ( adr->quantityName() == oldString )
    {
        adr->setQuantityName( newString );

        return true;
    }

    std::string correspondingOldString = RiaSummaryCurveAnalyzer::correspondingHistorySummaryCurveName( oldString );
    std::string correspondingNewString = RiaSummaryCurveAnalyzer::correspondingHistorySummaryCurveName( newString );

    if ( adr->quantityName() == correspondingOldString )
    {
        adr->setQuantityName( correspondingNewString );

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryPlotSourceStepping::summaryCasesForSourceStepping()
{
    std::vector<RimSummaryCase*> cases;

    RimProject* proj = RimProject::current();
    for ( auto sumCase : proj->allSummaryCases() )
    {
        if ( sumCase->isObservedData() ) continue;

        RimSummaryCaseCollection* sumCaseColl = nullptr;
        sumCase->firstAncestorOrThisOfType( sumCaseColl );

        if ( sumCaseColl && sumCaseColl->isEnsemble() )
        {
            if ( m_includeEnsembleCasesForCaseStepping() )
            {
                cases.push_back( sumCase );
            }
        }
        else
        {
            cases.push_back( sumCase );
        }
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
    if ( myAttr )
    {
        myAttr->showPreviousAndNextButtons = true;

        QString modifierText;

        if ( field == &m_summaryCase )
        {
            modifierText = ( "(Shift+" );
        }
        else if ( field == &m_wellName || field == &m_wellGroupName || field == &m_region )
        {
            modifierText = ( "(Ctrl+" );
        }
        else if ( field == &m_quantity )
        {
            modifierText = ( "(" );
        }

        if ( !modifierText.isEmpty() )
        {
            myAttr->nextButtonText = "Next " + modifierText + "PgDown)";
            myAttr->prevButtonText = "Previous " + modifierText + "PgUp)";
        }
    }

    if ( myAttr && ( uiConfigName == caf::PdmUiToolBarEditor::uiEditorConfigName() ) )
    {
        myAttr->minimumWidth = 120;
    }
}

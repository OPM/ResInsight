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

#include "RiaEnsembleNameTools.h"
#include "RiaGuiApplication.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"

#include "RimDataSourceSteppingTools.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryAddressModifier.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryDataSourceStepping.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotControls.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiToolBarEditor.h"

#include <QString>

#include <algorithm>
#include <vector>

using namespace RifEclipseSummaryAddressDefines;

CAF_PDM_SOURCE_INIT( RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping()
{
    CAF_PDM_InitObject( "Summary Curves Modifier" );

    setNotifyAllFieldsInMultiFieldChangedEvents( true );

    CAF_PDM_InitFieldNoDefault( &m_stepDimension, "StepDimension", "Step Dimension" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveCase", "Case" );

    CAF_PDM_InitField( &m_includeEnsembleCasesForCaseStepping, "IncludeEnsembleCasesForCaseStepping", true, "Include Ensemble Cases in Case List" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );
    CAF_PDM_InitFieldNoDefault( &m_groupName, "GroupName", "Group Name" );
    CAF_PDM_InitFieldNoDefault( &m_networkName, "NetworkName", "Network Name" );
    CAF_PDM_InitFieldNoDefault( &m_region, "Region", "Region" );
    CAF_PDM_InitFieldNoDefault( &m_vectorName, "VectorName", "Vector" );

    CAF_PDM_InitFieldNoDefault( &m_cellBlock, "CellBlock", "Block" );
    CAF_PDM_InitFieldNoDefault( &m_wellSegment, "Segment", "Segment" );
    CAF_PDM_InitFieldNoDefault( &m_completion, "Completion", "Completion" );
    CAF_PDM_InitFieldNoDefault( &m_aquifer, "Aquifer", "Aquifer" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );

    CAF_PDM_InitFieldNoDefault( &m_placeholderForLabel, "Placeholder", "" );
    m_placeholderForLabel = "No common identifiers detected";
    m_placeholderForLabel.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_placeholderForLabel.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_indexLabel, "IndexLabel", QString( "Step By" ), "Step By" );
    m_indexLabel.uiCapability()->setUiEditorTypeName( caf::PdmUiLabelEditor::uiEditorTypeName() );
    m_indexLabel.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_autoUpdateAppearance, "AutoUpdateAppearance", false, "Update Appearance" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setSourceSteppingObject( caf::PdmObject* sourceObject )
{
    m_objectForSourceStepping = sourceObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyNextStep()
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    modifyCurrentIndex( valueField, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::applyPrevStep()
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
    return toolbarFieldsForDataSourceStepping();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto visible = activeFieldsForDataSourceStepping();
    if ( visible.empty() )
    {
        uiOrdering.add( &m_placeholderForLabel );
    }

    for ( auto f : visible )
    {
        uiOrdering.add( f );
    }

    uiOrdering.add( &m_autoUpdateAppearance );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryPlotSourceStepping::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( ( fieldNeedingOptions == &m_placeholderForLabel ) || ( fieldNeedingOptions == &m_indexLabel ) ||
         ( fieldNeedingOptions == &m_autoUpdateAppearance ) || ( fieldNeedingOptions == &m_includeEnsembleCasesForCaseStepping ) ||
         ( fieldNeedingOptions == &m_stepDimension ) )
    {
        return {};
    }

    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_summaryCase )
    {
        auto summaryCases = RimSummaryPlotSourceStepping::summaryCasesForSourceStepping();
        for ( auto sumCase : summaryCases )
        {
            if ( sumCase->ensemble() )
            {
                if ( m_includeEnsembleCasesForCaseStepping() )
                {
                    auto name = sumCase->ensemble()->name() + " : " + sumCase->displayCaseName();
                    options.append( caf::PdmOptionItemInfo( name, sumCase ) );
                }
            }
            else
            {
                options.append( caf::PdmOptionItemInfo( sumCase->displayCaseName(), sumCase ) );
            }
        }

        return options;
    }

    if ( fieldNeedingOptions == &m_ensemble )
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

    RiaSummaryAddressAnalyzer  fallbackAnalyzer;
    RiaSummaryAddressAnalyzer* analyzer = nullptr;
    if ( !dataSourceSteppingObject()->curveSets().empty() )
    {
        auto first = dataSourceSteppingObject()->curveSets().front();
        if ( first->summaryCaseCollection() )
        {
            analyzer = first->summaryCaseCollection()->addressAnalyzer();
        }
    }

    if ( !analyzer )
    {
        // No cached analyzer found. Fallback to population of a local analyzer. Try to avoid this, as the analysis
        // operation is quite expensive.

        auto addresses = adressesForSourceStepping();
        fallbackAnalyzer.appendAddresses( addresses );
        analyzer = &fallbackAnalyzer;
    }

    if ( analyzer )
    {
        if ( fieldNeedingOptions == &m_vectorName )
        {
            m_cachedIdentifiers.clear();

            auto displayAndValueStrings = optionsForQuantity( analyzer );

            for ( const auto& displayAndValue : displayAndValueStrings )
            {
                options.append( caf::PdmOptionItemInfo( displayAndValue.first, displayAndValue.second ) );
                m_cachedIdentifiers.push_back( displayAndValue.first );
            }

            if ( options.isEmpty() )
            {
                options.push_back( caf::PdmOptionItemInfo( "None", "None" ) );
            }
        }
        else
        {
            SummaryCategory category = SummaryCategory::SUMMARY_INVALID;
            std::string     secondaryIdentifier;

            if ( fieldNeedingOptions == &m_wellName )
            {
                category = SummaryCategory::SUMMARY_WELL;
            }
            else if ( fieldNeedingOptions == &m_region )
            {
                category = SummaryCategory::SUMMARY_REGION;
            }
            else if ( fieldNeedingOptions == &m_groupName )
            {
                category = SummaryCategory::SUMMARY_GROUP;
            }
            else if ( fieldNeedingOptions == &m_networkName )
            {
                category = SummaryCategory::SUMMARY_NETWORK;
            }
            else if ( fieldNeedingOptions == &m_cellBlock )
            {
                category = SummaryCategory::SUMMARY_BLOCK;
            }
            else if ( fieldNeedingOptions == &m_wellSegment )
            {
                secondaryIdentifier = m_wellName().toStdString();
                category            = SummaryCategory::SUMMARY_WELL_SEGMENT;
            }
            else if ( fieldNeedingOptions == &m_completion )
            {
                secondaryIdentifier = m_wellName().toStdString();
                category            = SummaryCategory::SUMMARY_WELL_COMPLETION;
            }
            else if ( fieldNeedingOptions == &m_aquifer )
            {
                category = SummaryCategory::SUMMARY_AQUIFER;
            }

            std::vector<QString> identifierTexts;

            if ( category != SummaryCategory::SUMMARY_INVALID )
            {
                identifierTexts = analyzer->identifierTexts( category, secondaryIdentifier );
            }

            m_cachedIdentifiers.clear();

            if ( !identifierTexts.empty() )
            {
                for ( const auto& text : identifierTexts )
                {
                    options.append( caf::PdmOptionItemInfo( text, text ) );
                    m_cachedIdentifiers.push_back( text );
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
void RimSummaryPlotSourceStepping::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    std::vector<RimSummaryCurve*> curves;
    if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->allCurves();

    bool isAutoZoomAllowed = false;
    bool doZoomAll         = false;

    if ( changedField == &m_stepDimension )
    {
        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();
        RimSummaryMultiPlot* plot = dynamic_cast<RimSummaryMultiPlot*>( m_objectForSourceStepping.p() );
        if ( plot ) plot->storeStepDimensionFromToolbar();
        return;
    }

    if ( changedField == &m_includeEnsembleCasesForCaseStepping )
    {
        auto curveCollection = firstAncestorOrThisOfType<RimSummaryCurveCollection>();
        if ( curveCollection )
        {
            curveCollection->updateConnectedEditors();
        }

        auto ensembleCurveColl = firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();
        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();

        return;
    }

    bool triggerLoadDataAndUpdate = false;

    auto updateEnsembleAddresses =
        [&doZoomAll, &oldValue, &newValue]( const std::vector<RimEnsembleCurveSet*>& curveSets, SummaryCategory categoryToUpdate )
    {
        for ( auto curveSet : curveSets )
        {
            auto curveAdr = curveSet->curveAddress();

            auto yAddressToModify = curveAdr.summaryAddressY();
            auto xAddressToModify = curveAdr.summaryAddressX();

            if ( categoryToUpdate != SummaryCategory::SUMMARY_INVALID )
            {
                RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, categoryToUpdate, yAddressToModify );
                RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, categoryToUpdate, xAddressToModify );
            }
            else
            {
                RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, yAddressToModify );
                RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, xAddressToModify );
            }

            curveSet->setCurveAddress( RiaSummaryCurveAddress( xAddressToModify, yAddressToModify ) );
            curveSet->updateConnectedEditors();

            doZoomAll = true;
        }
    };

    if ( changedField == &m_summaryCase )
    {
        if ( m_summaryCase() )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCase*                       previousCase  = dynamic_cast<RimSummaryCase*>( variantHandle.p() );

            for ( auto curve : curves )
            {
                if ( previousCase == curve->summaryCaseY() )
                {
                    curve->setSummaryCaseY( m_summaryCase );

                    if ( m_autoUpdateAppearance )
                    {
                        curve->setCurveAppearanceFromCaseType();
                    }
                }

                if ( previousCase == curve->summaryCaseX() )
                {
                    curve->setSummaryCaseX( m_summaryCase );
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_groupName.uiCapability()->updateConnectedEditors();
        m_networkName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_vectorName.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_ensemble )
    {
        if ( m_ensemble() && dataSourceSteppingObject() )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle      = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCaseCollection*             previousCollection = dynamic_cast<RimSummaryCaseCollection*>( variantHandle.p() );

            for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
            {
                if ( curveSet->summaryCaseCollection() == previousCollection )
                {
                    curveSet->setSummaryCaseCollection( m_ensemble );
                }
            }

            triggerLoadDataAndUpdate = true;
        }

        m_wellName.uiCapability()->updateConnectedEditors();
        m_groupName.uiCapability()->updateConnectedEditors();
        m_networkName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_vectorName.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_vectorName )
    {
        updateVectorNameInCurves( curves, oldValue, newValue );

        if ( dataSourceSteppingObject() )
        {
            updateEnsembleAddresses( dataSourceSteppingObject()->curveSets(), SummaryCategory::SUMMARY_INVALID );
        }

        m_vectorName.uiCapability()->updateConnectedEditors();
        triggerLoadDataAndUpdate = true;
        isAutoZoomAllowed        = true;
    }

    if ( changedField != &m_vectorName )
    {
        SummaryCategory summaryCategoryToModify = SummaryCategory::SUMMARY_INVALID;
        if ( changedField == &m_wellName )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_WELL;
        }
        else if ( changedField == &m_region )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_REGION;
        }
        else if ( changedField == &m_groupName )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_GROUP;
        }
        else if ( changedField == &m_networkName )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_NETWORK;
        }
        else if ( changedField == &m_cellBlock )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_BLOCK;
        }
        else if ( changedField == &m_wellSegment )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_WELL_SEGMENT;
        }
        else if ( changedField == &m_completion )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_WELL_COMPLETION;
        }
        else if ( changedField == &m_aquifer )
        {
            summaryCategoryToModify = SummaryCategory::SUMMARY_AQUIFER;
        }

        if ( summaryCategoryToModify != SummaryCategory::SUMMARY_INVALID )
        {
            for ( auto curve : curves )
            {
                RifEclipseSummaryAddress adrY = curve->summaryAddressY();
                RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, adrY );
                curve->setSummaryAddressY( adrY );

                RifEclipseSummaryAddress adrX = curve->summaryAddressX();
                RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, adrX );
                curve->setSummaryAddressX( adrX );
            }

            if ( dataSourceSteppingObject() )
            {
                updateEnsembleAddresses( dataSourceSteppingObject()->curveSets(), summaryCategoryToModify );
            }

            triggerLoadDataAndUpdate = true;
        }
    }

    if ( triggerLoadDataAndUpdate )
    {
        auto summaryPlot = firstAncestorOrThisOfType<RimSummaryPlot>();

        RimSummaryMultiPlot* summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( m_objectForSourceStepping.p() );
        if ( summaryMultiPlot )
        {
            summaryMultiPlot->updatePlots();
            summaryMultiPlot->updatePlotTitles();

            if ( doZoomAll )
            {
                summaryMultiPlot->zoomAll();
            }
            else if ( isAutoZoomAllowed )
            {
                // The time axis can be zoomed and will be used for all plots. Do not zoom time axis in this case.
                summaryMultiPlot->zoomAllYAxes();
            }
        }
        else
        {
            summaryPlot->updatePlotTitle();
            summaryPlot->loadDataAndUpdate();
            summaryPlot->updateConnectedEditors();
            summaryPlot->curvesChanged.send();
        }

        updateAllRequiredEditors();

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();

        auto ensembleCurveColl = firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();
        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmValueField* RimSummaryPlotSourceStepping::fieldToModify()
{
    switch ( m_stepDimension() )
    {
        case RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE:
            return &m_summaryCase;
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE:
            return &m_ensemble;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL:
            return &m_wellName;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP:
            return &m_groupName;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK:
            return &m_networkName;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::REGION:
            return &m_region;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR:
            return &m_vectorName;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK:
            return &m_cellBlock;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER:
            return &m_aquifer;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL_SEGMENT:
            return &m_wellSegment;

        default:
            break;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryPlotSourceStepping::adressesForSourceStepping() const
{
    std::set<RifEclipseSummaryAddress> addressSet;

    if ( dataSourceSteppingObject() )
    {
        for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
        {
            if ( curveSet && curveSet->summaryCaseCollection() )
            {
                auto addresses = curveSet->summaryCaseCollection()->ensembleSummaryAddresses();
                addressSet.insert( addresses.begin(), addresses.end() );
            }
        }

        std::vector<RimSummaryCurve*> curves;
        if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->curvesForStepping();

        size_t maxAddrCount = 0;
        int    maxAddrIndex = -1;

        for ( size_t i = 0; i < curves.size(); i++ )
        {
            auto curve = curves[i];
            if ( !curve ) continue;

            if ( curve->summaryCaseY() && curve->summaryCaseY()->summaryReader() )
            {
                auto addresses = curve->summaryCaseY()->summaryReader()->allResultAddresses();

                size_t addrCount = addresses.size();
                if ( addrCount > maxAddrCount )
                {
                    maxAddrCount = addrCount;
                    maxAddrIndex = (int)i;
                }
            }

            if ( curve->summaryCaseX() && curve->summaryCaseX()->summaryReader() )
            {
                auto   addresses = curve->summaryCaseX()->summaryReader()->allResultAddresses();
                size_t addrCount = addresses.size();
                if ( addrCount > maxAddrCount )
                {
                    maxAddrCount = addrCount;
                    maxAddrIndex = (int)i;
                }
            }
        }

        if ( maxAddrIndex >= 0 )
        {
            auto curve = curves[maxAddrIndex];

            if ( curve->summaryCaseX() && curve->summaryCaseX()->summaryReader() )
            {
                auto addresses = curve->summaryCaseX()->summaryReader()->allResultAddresses();
                addressSet.insert( addresses.begin(), addresses.end() );
            }

            if ( curve->summaryCaseY() && curve->summaryCaseY()->summaryReader() )
            {
                auto addresses = curve->summaryCaseY()->summaryReader()->allResultAddresses();
                addressSet.insert( addresses.begin(), addresses.end() );
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

    if ( dataSourceSteppingObject() )
    {
        for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
        {
            addresses.insert( curveSet->summaryAddressY() );
        }

        std::vector<RimSummaryCurve*> curves;
        if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->curvesForStepping();

        for ( auto curve : curves )
        {
            addresses.insert( curve->summaryAddressY() );

            if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
            {
                addresses.insert( curve->summaryAddressX() );
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

    std::vector<RimSummaryCurve*> curves;
    if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->curvesForStepping();
    for ( auto c : curves )
    {
        sumCases.insert( c->summaryCaseY() );

        if ( c->axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
        {
            sumCases.insert( c->summaryCaseX() );
        }
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::activeFieldsForDataSourceStepping()
{
    RimProject* proj = RimProject::current();
    if ( !proj ) return {};

    std::vector<caf::PdmFieldHandle*> fields;

    fields.push_back( &m_stepDimension );

    auto sumCases = summaryCasesCurveCollection();
    if ( sumCases.size() == 1 )
    {
        if ( proj->allSummaryCases().size() > 1 )
        {
            m_summaryCase = *( sumCases.begin() );

            fields.push_back( &m_summaryCase );
            fields.push_back( &m_includeEnsembleCasesForCaseStepping );
        }
    }

    auto ensembleColl = ensembleCollection();
    if ( ensembleColl.size() == 1 )
    {
        if ( proj->summaryGroups().size() > 1 )
        {
            m_ensemble = *( ensembleColl.begin() );

            fields.push_back( &m_ensemble );
        }
    }

    std::vector<caf::PdmFieldHandle*> fieldsCommonForAllCurves;

    {
        RiaSummaryAddressAnalyzer analyzer;
        analyzer.appendAddresses( addressesForCurvesInPlot() );

        SummaryCategory category = SummaryCategory::SUMMARY_INVALID;

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
                    category = SummaryCategory::SUMMARY_WELL;
                }
            }
        }

        if ( category != SummaryCategory::SUMMARY_INVALID )
        {
            if ( analyzer.wellNames().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.wellNames().begin() ) );
                m_wellName  = txt;

                fieldsCommonForAllCurves.push_back( &m_wellName );
            }

            if ( analyzer.groupNames().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.groupNames().begin() ) );
                m_groupName = txt;

                fieldsCommonForAllCurves.push_back( &m_groupName );
            }

            if ( analyzer.networkNames().size() == 1 )
            {
                QString txt   = QString::fromStdString( *( analyzer.networkNames().begin() ) );
                m_networkName = txt;

                fieldsCommonForAllCurves.push_back( &m_networkName );
            }

            if ( analyzer.regionNumbers().size() == 1 )
            {
                m_region = *( analyzer.regionNumbers().begin() );

                fieldsCommonForAllCurves.push_back( &m_region );
            }

            if ( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt   = QString::number( *( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).begin() ) );
                m_wellSegment = txt;

                fieldsCommonForAllCurves.push_back( &m_wellSegment );
            }

            if ( analyzer.blocks().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.blocks().begin() ) );
                m_cellBlock = txt;

                fieldsCommonForAllCurves.push_back( &m_cellBlock );
            }

            if ( analyzer.wellCompletions( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt  = QString::fromStdString( *( analyzer.wellCompletions( m_wellName().toStdString() ).begin() ) );
                m_completion = txt;

                fieldsCommonForAllCurves.push_back( &m_completion );
            }

            if ( analyzer.aquifers().size() == 1 )
            {
                m_aquifer = *( analyzer.aquifers().begin() );

                fieldsCommonForAllCurves.push_back( &m_aquifer );
            }

            if ( !analyzer.quantityNameForTitle().empty() )
            {
                QString txt  = QString::fromStdString( analyzer.quantityNameForTitle() );
                m_vectorName = txt;

                fieldsCommonForAllCurves.push_back( &m_vectorName );
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
    std::set<RimSummaryCaseCollection*> summaryCaseCollections;

    if ( dataSourceSteppingObject() )
    {
        for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
        {
            if ( curveSet && curveSet->summaryCaseCollection() )
            {
                summaryCaseCollections.insert( curveSet->summaryCaseCollection() );
            }
        }
    }

    return summaryCaseCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::modifyCurrentIndex( caf::PdmValueField* valueField, int indexOffset, bool notifyChange )
{
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions( valueField );
    RimDataSourceSteppingTools::modifyCurrentIndex( valueField, options, indexOffset, notifyChange );
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

        auto sumCaseColl = sumCase->firstAncestorOrThisOfType<RimSummaryCaseCollection>();
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
RimSummaryDataSourceStepping* RimSummaryPlotSourceStepping::dataSourceSteppingObject() const
{
    return dynamic_cast<RimSummaryDataSourceStepping*>( m_objectForSourceStepping.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    auto* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
    if ( myAttr )
    {
        if ( field == &m_stepDimension )
        {
            myAttr->showPreviousAndNextButtons = false;
        }
        else
        {
            QString nextText       = RimSummaryPlotControls::nextStepKeyText();
            QString prevText       = RimSummaryPlotControls::prevStepKeyText();
            myAttr->nextButtonText = "Next (" + nextText + ")";
            myAttr->prevButtonText = "Previous (" + prevText + ")";

            myAttr->nextIcon     = QIcon( ":/ComboBoxDown.svg" );
            myAttr->previousIcon = QIcon( ":/ComboBoxUp.svg" );

            myAttr->showPreviousAndNextButtons = true;
        }
    }

    if ( myAttr && ( uiConfigName == caf::PdmUiToolBarEditor::uiEditorConfigName() ) )
    {
        myAttr->minimumWidth = 120;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryPlotSourceStepping::toolbarFieldsForDataSourceStepping()
{
    std::vector<caf::PdmFieldHandle*> fields;
    fields.push_back( &m_indexLabel );
    fields.push_back( &m_stepDimension );

    caf::PdmFieldHandle* field = fieldToModify();
    if ( field != nullptr ) fields.push_back( field );

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryPlotSourceStepping::stepAddress( RifEclipseSummaryAddress addr, int direction )
{
    auto                      addresses = adressesForSourceStepping();
    RiaSummaryAddressAnalyzer analyzer;
    analyzer.appendAddresses( addresses );

    // Find the iterator to the string in the list of strings
    auto getIdIterator = [direction]( const std::vector<QString>& ids, const QString& searchString ) -> decltype( ids.begin() )
    {
        auto found = std::find( ids.begin(), ids.end(), searchString );
        if ( found != ids.end() )
        {
            if ( direction > 0 )
                ++found;
            else if ( found != ids.begin() )
                --found;
        }
        return found;
    };

    switch ( m_stepDimension() )
    {
        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL:
        {
            auto ids          = analyzer.identifierTexts( SummaryCategory::SUMMARY_WELL, "" );
            auto searchString = QString::fromStdString( addr.wellName() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setWellName( ( *found ).toStdString() );
        }
        break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP:
        {
            auto ids          = analyzer.identifierTexts( SummaryCategory::SUMMARY_GROUP, "" );
            auto searchString = QString::fromStdString( addr.groupName() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setGroupName( ( *found ).toStdString() );
        }
        break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK:
        {
            auto ids          = analyzer.identifierTexts( SummaryCategory::SUMMARY_NETWORK, "" );
            auto searchString = QString::fromStdString( addr.networkName() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setNetworkName( ( *found ).toStdString() );
        }
        break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::REGION:
        {
            auto ids          = analyzer.identifierTexts( SummaryCategory::SUMMARY_REGION, "" );
            auto searchString = QString::number( addr.regionNumber() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setRegion( ( *found ).toInt() );
        }
        break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR:
        {
            auto options = optionsForQuantity( addresses );

            std::vector<QString> ids;
            for ( auto it = options.begin(); it != options.end(); it++ )
            {
                ids.push_back( it->second );
            }

            auto searchString = QString::fromStdString( addr.vectorName() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setVectorName( ( *found ).toStdString() );
        }
        break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK:
        {
            auto ids          = analyzer.identifierTexts( SummaryCategory::SUMMARY_BLOCK, "" );
            auto searchString = QString::fromStdString( addr.blockAsString() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setCellIjk( ( *found ).toStdString() );
        }
        break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER:
        {
            auto ids          = analyzer.identifierTexts( SummaryCategory::SUMMARY_AQUIFER, "" );
            auto searchString = QString::number( addr.aquiferNumber() );
            auto found        = getIdIterator( ids, searchString );
            if ( found != ids.end() ) addr.setAquiferNumber( ( *found ).toInt() );
        }
        break;

        default:
            break;
    }
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::syncWithStepper( RimSummaryPlotSourceStepping* other )
{
    switch ( m_stepDimension() )
    {
        case RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE:
            m_summaryCase = other->m_summaryCase();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE:
            m_ensemble = other->m_ensemble();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL:
            m_wellName = other->m_wellName();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP:
            m_groupName = other->m_groupName();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK:
            m_networkName = other->m_networkName();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::REGION:
            m_region = other->m_region();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR:
            m_vectorName = other->m_vectorName();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK:
            m_cellBlock = other->m_cellBlock();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER:
            m_aquifer = other->m_aquifer();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL_SEGMENT:
            m_wellSegment = other->m_wellSegment();
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setStep( QString stepIdentifier )
{
    if ( std::count( m_cachedIdentifiers.begin(), m_cachedIdentifiers.end(), stepIdentifier ) == 0 ) return;

    switch ( m_stepDimension() )
    {
        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL:
            m_wellName.setValueWithFieldChanged( stepIdentifier );
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP:
            m_groupName.setValueWithFieldChanged( stepIdentifier );
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK:
            m_networkName.setValueWithFieldChanged( stepIdentifier );
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR:
            m_vectorName.setValueWithFieldChanged( stepIdentifier );
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK:
            m_cellBlock.setValueWithFieldChanged( stepIdentifier );
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER:
        case RimSummaryDataSourceStepping::SourceSteppingDimension::REGION:
        case RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE:
        case RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE:
        default:
            CAF_ASSERT( false ); // not supported for these dimensions, yet
            return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimSummaryPlotSourceStepping::optionsForQuantity( std::set<RifEclipseSummaryAddress> addresses )
{
    SummaryCategory category = SummaryCategory::SUMMARY_FIELD;

    auto visibleCurveAddresses = addressesForCurvesInPlot();
    if ( !visibleCurveAddresses.empty() )
    {
        category = visibleCurveAddresses.begin()->category();
    }

    std::map<QString, QString> displayAndValueStrings;

    {
        RiaSummaryAddressAnalyzer quantityAnalyzer;

        auto subset = RiaSummaryAddressAnalyzer::addressesForCategory( addresses, category );
        quantityAnalyzer.appendAddresses( subset );

        auto quantities = quantityAnalyzer.quantities();
        for ( const auto& s : quantities )
        {
            QString valueString = QString::fromStdString( s );

            displayAndValueStrings[valueString] = valueString;
        }
    }

    return displayAndValueStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimSummaryPlotSourceStepping::optionsForQuantity( RiaSummaryAddressAnalyzer* analyzser )
{
    SummaryCategory category = SummaryCategory::SUMMARY_FIELD;

    auto visibleCurveAddresses = addressesForCurvesInPlot();
    if ( !visibleCurveAddresses.empty() )
    {
        category = visibleCurveAddresses.begin()->category();
    }

    std::map<QString, QString> displayAndValueStrings;

    if ( analyzser )
    {
        auto vectorNames = analyzser->vectorNamesForCategory( category );

        for ( const auto& s : vectorNames )
        {
            QString valueString = QString::fromStdString( s );

            displayAndValueStrings[valueString] = valueString;
        }
    }

    return displayAndValueStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::updateVectorNameInCurves( std::vector<RimSummaryCurve*>& curves,
                                                             const QVariant&                oldValue,
                                                             const QVariant&                newValue )
{
    std::map<RimSummaryPlot*, std::vector<RimSummaryCurve*>> curvesInPlot;
    for ( auto curve : curves )
    {
        {
            auto adr = curve->summaryAddressY();
            if ( RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, adr ) ) curve->setSummaryAddressY( adr );
        }

        {
            auto adr = curve->summaryAddressX();
            if ( RimDataSourceSteppingTools::updateQuantityIfMatching( oldValue, newValue, adr ) ) curve->setSummaryAddressX( adr );
        }

        if ( m_autoUpdateAppearance )
        {
            auto summaryPlot = curve->firstAncestorOfType<RimSummaryPlot>();
            if ( summaryPlot )
            {
                if ( curvesInPlot.count( summaryPlot ) )
                {
                    curvesInPlot[summaryPlot].push_back( curve );
                }
                else
                {
                    curvesInPlot[summaryPlot] = { curve };
                }
            }
        }
    }

    if ( m_autoUpdateAppearance )
    {
        // Apply the curve appearance for all curves in one go. If appearance of each curve was updated as part of the
        // loop, the appearance of curves was based on a mix of old and new curves causing a mix of different curve
        // styles

        for ( const auto& [plot, curves] : curvesInPlot )
        {
            plot->applyDefaultCurveAppearances( curves );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDataSourceStepping::SourceSteppingDimension RimSummaryPlotSourceStepping::stepDimension() const
{
    return m_stepDimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setStepDimension( RimSummaryDataSourceStepping::SourceSteppingDimension dimension )
{
    m_stepDimension = dimension;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryPlotSourceStepping::stepCase( int direction )
{
    std::vector<RimSummaryCase*> cases;

    auto summaryCases = RimSummaryPlotSourceStepping::summaryCasesForSourceStepping();
    for ( auto sumCase : summaryCases )
    {
        if ( sumCase->ensemble() )
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

    auto found = std::find( cases.begin(), cases.end(), m_summaryCase() );
    if ( found != cases.end() )
    {
        if ( direction > 0 )
        {
            found++;
        }
        else
        {
            if ( found != cases.begin() ) found--;
        }
        if ( found != cases.end() ) return *found;
    }

    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimSummaryPlotSourceStepping::stepEnsemble( int direction )
{
    std::vector<RimSummaryCaseCollection*> ensembles;

    RimProject* proj = RimProject::current();
    for ( auto ensemble : proj->summaryGroups() )
    {
        if ( ensemble->isEnsemble() )
        {
            ensembles.push_back( ensemble );
        }
    }

    auto found = std::find( ensembles.begin(), ensembles.end(), m_ensemble() );
    if ( found != ensembles.end() )
    {
        if ( direction > 0 )
        {
            found++;
        }
        else
        {
            if ( found != ensembles.begin() ) found--;
        }
        if ( found != ensembles.end() ) return *found;
    }

    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::updateStepIndex( int direction )
{
    caf::PdmValueField* valueField = fieldToModify();
    if ( !valueField ) return;

    bool notifyChange = false;
    modifyCurrentIndex( valueField, direction, notifyChange );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RimSummaryPlotSourceStepping::plotsMatchingStepSettings( std::vector<RimSummaryPlot*> plots )
{
    std::vector<RimPlot*> matchingPlots;

    int         caseIdToMatch     = -1;
    int         ensembleIdToMatch = -1;
    std::string wellNameToMatch;
    std::string groupNameToMatch;
    std::string networkToMatch;
    int         regionToMatch = -1;
    std::string vectorToMatch;
    std::string blockToMatch;
    int         aquiferToMatch = -1;

    switch ( m_stepDimension() )
    {
        case RimSummaryDataSourceStepping::SourceSteppingDimension::SUMMARY_CASE:
            if ( m_summaryCase() ) caseIdToMatch = m_summaryCase()->caseId();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::ENSEMBLE:
            if ( m_ensemble() ) ensembleIdToMatch = m_ensemble()->ensembleId();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::WELL:
            wellNameToMatch = m_wellName().toStdString();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::GROUP:
            groupNameToMatch = m_groupName().toStdString();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::NETWORK:
            networkToMatch = m_networkName().toStdString();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::REGION:
            regionToMatch = m_region();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::VECTOR:
            vectorToMatch = m_vectorName().toStdString();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::BLOCK:
            blockToMatch = m_cellBlock().toStdString();
            break;

        case RimSummaryDataSourceStepping::SourceSteppingDimension::AQUIFER:
            aquiferToMatch = m_aquifer();
            break;

        default:
            break;
    }

    for ( auto plot : plots )
    {
        bool isMatching = false;

        if ( caseIdToMatch != -1 )
        {
            auto curves = plot->summaryCurves();
            for ( auto c : curves )
            {
                if ( c->summaryCaseY()->caseId() == caseIdToMatch ) isMatching = true;
            }
        }
        else if ( ensembleIdToMatch != -1 )
        {
            auto curves = plot->curveSets();
            for ( auto c : curves )
            {
                if ( c->ensembleId() == ensembleIdToMatch ) isMatching = true;
            }
        }
        else
        {
            auto addresses = RimSummaryAddressModifier::allSummaryAddressesY( plot );

            for ( const auto& a : addresses )
            {
                if ( !wellNameToMatch.empty() && a.wellName() == wellNameToMatch )
                {
                    isMatching = true;
                }
                else if ( !vectorToMatch.empty() && a.vectorName() == vectorToMatch )
                {
                    isMatching = true;
                }
                else if ( !groupNameToMatch.empty() && a.groupName() == groupNameToMatch )
                {
                    isMatching = true;
                }
                else if ( !networkToMatch.empty() && a.networkName() == networkToMatch )
                {
                    isMatching = true;
                }
                else if ( regionToMatch != -1 && a.regionNumber() == regionToMatch )
                {
                    isMatching = true;
                }
                else if ( !blockToMatch.empty() && a.blockAsString() == blockToMatch )
                {
                    isMatching = true;
                }
                else if ( aquiferToMatch != -1 && a.aquiferNumber() == aquiferToMatch )
                {
                    isMatching = true;
                }
            }
        }

        if ( isMatching ) matchingPlots.push_back( plot );
    }

    return matchingPlots;
}

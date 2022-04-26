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
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlot.h"
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
#include "cafPdmUiListEditor.h"
#include "cafPdmUiToolBarEditor.h"

namespace caf
{
template <>
void AppEnum<RimSummaryPlotSourceStepping::SourceSteppingDimension>::setUp()
{
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::QUANTITY, "QUANTITY", "Quantity" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::WELL, "WELL", "Well" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::GROUP, "GROUP", "Group" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::REGION, "REGION", "Region" );
    addItem( RimSummaryPlotSourceStepping::SourceSteppingDimension::BLOCK, "BLOCK", "Block" );
    setDefault( RimSummaryPlotSourceStepping::SourceSteppingDimension::QUANTITY );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimSummaryPlotSourceStepping, "RimSummaryCurveCollectionModifier" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping::RimSummaryPlotSourceStepping()
    : m_sourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS )
{
    CAF_PDM_InitObject( "Summary Curves Modifier" );

    CAF_PDM_InitFieldNoDefault( &m_stepDimension, "StepDimension", "Step Dimension" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveCase", "Case" );

    CAF_PDM_InitField( &m_includeEnsembleCasesForCaseStepping,
                       "IncludeEnsembleCasesForCaseStepping",
                       true,
                       "Include Ensemble Cases in Case List" );

    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );
    CAF_PDM_InitFieldNoDefault( &m_groupName, "GroupName", "Group Name" );
    CAF_PDM_InitFieldNoDefault( &m_region, "Region", "Region" );
    CAF_PDM_InitFieldNoDefault( &m_quantity, "Quantities", "Quantity" );

    CAF_PDM_InitFieldNoDefault( &m_cellBlock, "CellBlock", "Block" );
    CAF_PDM_InitFieldNoDefault( &m_segment, "Segment", "Segment" );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotSourceStepping::setSourceSteppingType( RimSummaryDataSourceStepping::Axis sourceSteppingType )
{
    m_sourceSteppingType = sourceSteppingType;
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

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimSummaryPlotSourceStepping::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( ( fieldNeedingOptions == &m_includeEnsembleCasesForCaseStepping ) || ( fieldNeedingOptions == &m_stepDimension ) )
    {
        return caf::PdmObject::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    }

    if ( ( fieldNeedingOptions == &m_placeholderForLabel ) || ( fieldNeedingOptions == &m_indexLabel ) )
    {
        return options;
    }

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
                RiaSummaryAddressAnalyzer quantityAnalyzer;

                auto subset = RiaSummaryAddressAnalyzer::addressesForCategory( addresses, category );
                quantityAnalyzer.appendAddresses( subset );

                RiaSummaryAddressAnalyzer analyzerForVisibleCurves;
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
            else if ( fieldNeedingOptions == &m_groupName )
            {
                category = RifEclipseSummaryAddress::SUMMARY_GROUP;
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
                RiaSummaryAddressAnalyzer analyzer;
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
    if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->allCurves( m_sourceSteppingType );

    if ( changedField == &m_stepDimension )
    {
        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();
        return;
    }

    if ( changedField == &m_includeEnsembleCasesForCaseStepping )
    {
        RimSummaryCurveCollection* curveCollection = nullptr;
        this->firstAncestorOrThisOfType( curveCollection );
        if ( curveCollection )
        {
            curveCollection->updateConnectedEditors();
        }

        RimEnsembleCurveSetCollection* ensembleCurveColl = nullptr;
        this->firstAncestorOrThisOfType( ensembleCurveColl );
        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();

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
        m_groupName.uiCapability()->updateConnectedEditors();
        m_region.uiCapability()->updateConnectedEditors();
        m_quantity.uiCapability()->updateConnectedEditors();
    }
    else if ( changedField == &m_ensemble )
    {
        if ( m_ensemble() && dataSourceSteppingObject() )
        {
            caf::PdmPointer<caf::PdmObjectHandle> variantHandle = oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            RimSummaryCaseCollection* previousCollection = dynamic_cast<RimSummaryCaseCollection*>( variantHandle.p() );

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
                RimDataSourceSteppingTools::updateHistoryAndSummaryQuantityIfMatching( oldValue, newValue, &adr );
                curve->setSummaryAddressY( adr );
            }

            if ( isXAxisStepping() )
            {
                auto adr = curve->summaryAddressX();
                RimDataSourceSteppingTools::updateHistoryAndSummaryQuantityIfMatching( oldValue, newValue, &adr );
                curve->setSummaryAddressX( adr );
            }

            curve->setDefaultCurveAppearance();
        }

        if ( dataSourceSteppingObject() )
        {
            for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
            {
                auto adr = curveSet->summaryAddress();
                RimDataSourceSteppingTools::updateHistoryAndSummaryQuantityIfMatching( oldValue, newValue, &adr );
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
        else if ( changedField == &m_groupName )
        {
            summaryCategoryToModify = RifEclipseSummaryAddress::SUMMARY_GROUP;
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
                    RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curve->setSummaryAddressY( adr );
                }

                if ( isXAxisStepping() )
                {
                    RifEclipseSummaryAddress adr = curve->summaryAddressX();
                    RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curve->setSummaryAddressX( adr );
                }
            }

            if ( dataSourceSteppingObject() )
            {
                for ( auto curveSet : dataSourceSteppingObject()->curveSets() )
                {
                    auto adr = curveSet->summaryAddress();
                    RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, summaryCategoryToModify, &adr );
                    curveSet->setSummaryAddress( adr );
                }
            }

            triggerLoadDataAndUpdate = true;
        }
    }

    if ( triggerLoadDataAndUpdate )
    {
        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfType( summaryPlot );
        if ( summaryPlot )
        {
            summaryPlot->updatePlotTitle();
            summaryPlot->loadDataAndUpdate();
            summaryPlot->updateConnectedEditors();
        }

        RimSummaryMultiPlot* summaryMultiPlot = dynamic_cast<RimSummaryMultiPlot*>( m_objectForSourceStepping.p() );
        if ( summaryMultiPlot )
        {
            summaryMultiPlot->loadDataAndUpdate();
        }

        RimEnsembleCurveSetCollection* ensembleCurveColl = nullptr;
        this->firstAncestorOrThisOfType( ensembleCurveColl );
        if ( ensembleCurveColl )
        {
            ensembleCurveColl->updateConnectedEditors();
        }

        RimSummaryCrossPlot* summaryCrossPlot = dynamic_cast<RimSummaryCrossPlot*>( summaryPlot );
        if ( summaryCrossPlot )
        {
            // Trigger update of curve collection (and summary toolbar in main window), as the visibility of combo
            // boxes might have been changed due to the updates in this function
            RimSummaryCurveCollection* curveCollection = nullptr;
            this->firstAncestorOrThisOfType( curveCollection );
            if ( curveCollection )
            {
                curveCollection->updateConnectedEditors();
            }

            RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
            mainPlotWindow->updateMultiPlotToolBar();
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
        case SourceSteppingDimension::SUMMARY_CASE:
            return &m_summaryCase;
            break;

        case SourceSteppingDimension::ENSEMBLE:
            return &m_ensemble;

        case SourceSteppingDimension::WELL:
            return &m_wellName;

        case SourceSteppingDimension::GROUP:
            return &m_groupName;

        case SourceSteppingDimension::REGION:
            return &m_region;

        case SourceSteppingDimension::QUANTITY:
            return &m_quantity;

        case SourceSteppingDimension::BLOCK:
            return &m_cellBlock;

        case SourceSteppingDimension::SEGMENT:
            return &m_segment;

        case SourceSteppingDimension::COMPLETION:
            return &m_completion;

        case SourceSteppingDimension::AQUIFER:
            return &m_aquifer;

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
        if ( dataSourceSteppingObject() )
            curves = dataSourceSteppingObject()->curvesForStepping( m_sourceSteppingType );

        for ( auto curve : curves )
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
            addresses.insert( curveSet->summaryAddress() );
        }

        std::vector<RimSummaryCurve*> curves;
        if ( dataSourceSteppingObject() )
            curves = dataSourceSteppingObject()->curvesForStepping( m_sourceSteppingType );

        for ( auto curve : curves )
        {
            if ( isYAxisStepping() )
            {
                addresses.insert( curve->summaryAddressY() );
            }

            if ( isXAxisStepping() )
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
    if ( dataSourceSteppingObject() ) curves = dataSourceSteppingObject()->curvesForStepping( m_sourceSteppingType );
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

                fieldsCommonForAllCurves.push_back( &m_wellName );
            }

            if ( analyzer.groupNames().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.groupNames().begin() ) );
                m_groupName = txt;

                fieldsCommonForAllCurves.push_back( &m_groupName );
            }

            if ( analyzer.regionNumbers().size() == 1 )
            {
                m_region = *( analyzer.regionNumbers().begin() );

                fieldsCommonForAllCurves.push_back( &m_region );
            }

            if ( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt = QString::number( *( analyzer.wellSegmentNumbers( m_wellName().toStdString() ).begin() ) );
                m_segment   = txt;

                fieldsCommonForAllCurves.push_back( &m_segment );
            }

            if ( analyzer.blocks().size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.blocks().begin() ) );
                m_cellBlock = txt;

                fieldsCommonForAllCurves.push_back( &m_cellBlock );
            }

            if ( analyzer.wellCompletions( m_wellName().toStdString() ).size() == 1 )
            {
                QString txt = QString::fromStdString( *( analyzer.wellCompletions( m_wellName().toStdString() ).begin() ) );
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
                QString txt = QString::fromStdString( analyzer.quantityNameForTitle() );
                m_quantity  = txt;

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
bool RimSummaryPlotSourceStepping::isXAxisStepping() const
{
    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS ) return true;

    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::X_AXIS ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotSourceStepping::isYAxisStepping() const
{
    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS ) return true;

    if ( m_sourceSteppingType == RimSummaryDataSourceStepping::Axis::Y_AXIS ) return true;

    return false;
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
    caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
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

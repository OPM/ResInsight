////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicSummaryPlotTemplateTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryTools.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "RicSelectPlotTemplateUi.h"

#include "RifSummaryReaderInterface.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimDialogData.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QFile>
#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotTemplateTools::createMultiPlotFromTemplateFile( const QString& fileName )
{
    QFile importFile( fileName );
    if ( !importFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        RiaLogging::error( QString( "Create Plot from Template : Could not open the file: %1" ).arg( fileName ) );
        return nullptr;
    }

    QTextStream stream( &importFile );

    QString objectAsText = stream.readAll();

    caf::PdmObjectHandle* obj =
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( objectAsText,
                                                                 caf::PdmDefaultObjectFactory::instance(),
                                                                 true );

    return dynamic_cast<RimSummaryMultiPlot*>( obj );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotTemplateTools::create( const QString& fileName )
{
    auto sumCases           = RicSummaryPlotTemplateTools::selectedSummaryCases();
    auto sumCaseCollections = RicSummaryPlotTemplateTools::selectedSummaryCaseCollections();

    auto summaryAddressCollections = RicSummaryPlotTemplateTools::selectedSummaryAddressCollections();

    std::vector<QString>                wellNames;
    std::vector<QString>                groupNames;
    std::vector<QString>                regions;
    std::set<RimSummaryCase*>           caseSet;
    std::set<RimSummaryCaseCollection*> caseCollectionSet;

    if ( summaryAddressCollections.empty() )
    {
        RiaSummaryAddressAnalyzer analyzer;

        if ( !sumCases.empty() )
        {
            auto firstCase = sumCases.front();

            analyzer.appendAddresses( firstCase->summaryReader()->allResultAddresses() );
        }
        else if ( !sumCaseCollections.empty() )
        {
            auto caseCollection = sumCaseCollections.front();

            auto firstCase = caseCollection->firstSummaryCase();
            if ( firstCase != nullptr )
            {
                analyzer.appendAddresses( firstCase->summaryReader()->allResultAddresses() );
            }
        }

        if ( !analyzer.wellNames().empty() )
            wellNames.push_back( QString::fromStdString( *( analyzer.wellNames().begin() ) ) );
        if ( !analyzer.groupNames().empty() )
            groupNames.push_back( QString::fromStdString( *( analyzer.groupNames().begin() ) ) );
        if ( !analyzer.regionNumbers().empty() )
            regions.push_back( QString::number( *( analyzer.regionNumbers().begin() ) ) );
    }
    else
    {
        for ( auto a : summaryAddressCollections )
        {
            if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL )
            {
                wellNames.push_back( a->name() );
            }
            else if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::GROUP )
            {
                groupNames.push_back( a->name() );
            }
            else if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION )
            {
                regions.push_back( a->name() );
            }

            auto sumCase = RiaSummaryTools::summaryCaseById( a->caseId() );
            if ( sumCase ) caseSet.insert( sumCase );

            auto ensemble = RiaSummaryTools::ensembleById( a->ensembleId() );
            if ( ensemble ) caseCollectionSet.insert( ensemble );
        }
    }

    for ( auto sumCase : caseSet )
    {
        sumCases.push_back( sumCase );
    }

    for ( auto sumCaseCollection : caseCollectionSet )
    {
        sumCaseCollections.push_back( sumCaseCollection );
    }

    auto proj        = RimProject::current();
    auto collections = proj->mainPlotCollection()->summaryMultiPlotCollection();

    auto newSummaryPlot = RicSummaryPlotTemplateTools::createMultiPlotFromTemplateFile( fileName );
    if ( !newSummaryPlot ) return nullptr;

    collections->addSummaryMultiPlot( newSummaryPlot );
    newSummaryPlot->resolveReferencesRecursively();

    RicSummaryPlotTemplateTools::setValuesForPlaceholders( newSummaryPlot,
                                                           sumCases,
                                                           sumCaseCollections,
                                                           wellNames,
                                                           groupNames,
                                                           regions );
    newSummaryPlot->initAfterReadRecursively();
    newSummaryPlot->loadDataAndUpdate();
    collections->updateConnectedEditors();

    return newSummaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotTemplateTools::create( const QString&                                fileName,
                                                          const std::vector<RimSummaryCase*>&           sumCases,
                                                          const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    std::vector<QString> wellNames;
    std::vector<QString> groupNames;
    std::vector<QString> regions;

    RiaSummaryAddressAnalyzer analyzer;

    if ( !sumCases.empty() )
    {
        auto firstCase = sumCases.front();

        analyzer.appendAddresses( firstCase->summaryReader()->allResultAddresses() );
    }
    else if ( !ensembles.empty() )
    {
        auto caseCollection = ensembles.front();

        auto firstCase = caseCollection->firstSummaryCase();
        if ( firstCase != nullptr )
        {
            analyzer.appendAddresses( firstCase->summaryReader()->allResultAddresses() );
        }
    }

    if ( !analyzer.wellNames().empty() )
        wellNames.push_back( QString::fromStdString( *( analyzer.wellNames().begin() ) ) );
    if ( !analyzer.groupNames().empty() )
        groupNames.push_back( QString::fromStdString( *( analyzer.groupNames().begin() ) ) );
    if ( !analyzer.regionNumbers().empty() )
        regions.push_back( QString::number( *( analyzer.regionNumbers().begin() ) ) );

    auto proj        = RimProject::current();
    auto collections = proj->mainPlotCollection()->summaryMultiPlotCollection();

    auto newSummaryPlot = RicSummaryPlotTemplateTools::createMultiPlotFromTemplateFile( fileName );
    if ( !newSummaryPlot ) return nullptr;

    collections->addSummaryMultiPlot( newSummaryPlot );
    newSummaryPlot->resolveReferencesRecursively();

    RicSummaryPlotTemplateTools::setValuesForPlaceholders( newSummaryPlot, sumCases, ensembles, wellNames, groupNames, regions );
    newSummaryPlot->initAfterReadRecursively();
    newSummaryPlot->loadDataAndUpdate();
    collections->updateConnectedEditors();

    return newSummaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::setValuesForPlaceholders( RimSummaryMultiPlot*                summaryMultiPlot,
                                                            const std::vector<RimSummaryCase*>& selectedSummaryCases,
                                                            const std::vector<RimSummaryCaseCollection*>& selectedEnsembles,
                                                            const std::vector<QString>&                   wellNames,
                                                            const std::vector<QString>&                   groupNames,
                                                            const std::vector<QString>&                   regions )

{
    // Assumes this plot is inserted into the project. This is required when assigning the ptrFields
    RimProject* proj = nullptr;
    summaryMultiPlot->firstAncestorOfType( proj );
    CAF_ASSERT( proj );

    auto plots = summaryMultiPlot->plots();
    for ( auto p : plots )
    {
        auto summaryPlot = dynamic_cast<RimSummaryPlot*>( p );
        if ( summaryPlot )
            setValuesForPlaceholders( summaryPlot, selectedSummaryCases, selectedEnsembles, wellNames, groupNames, regions );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::setValuesForPlaceholders( RimSummaryPlot*                     summaryPlot,
                                                            const std::vector<RimSummaryCase*>& selectedSummaryCases,
                                                            const std::vector<RimSummaryCaseCollection*>& selectedEnsembles,
                                                            const std::vector<QString>&                   wellNames,
                                                            const std::vector<QString>&                   groupNames,
                                                            const std::vector<QString>&                   regions )
{
    // Assumes this plot is inserted into the project. This is required when assigning the ptrFields
    RimProject* proj = nullptr;
    summaryPlot->firstAncestorOfType( proj );
    CAF_ASSERT( proj );

    {
        // Replace single summary curves data sources

        auto summaryCurves = summaryPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS );
        for ( const auto& curve : summaryCurves )
        {
            auto fieldHandle = curve->findField( RicSummaryPlotTemplateTools::summaryCaseFieldKeyword() );
            if ( fieldHandle )
            {
                bool          conversionOk      = false;
                const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForSummaryCase();

                auto referenceString = fieldHandle->xmlCapability()->referenceString();
                int  indexValue =
                    RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( selectedSummaryCases.size() ) )
                {
                    auto summaryCaseY = selectedSummaryCases[static_cast<int>( indexValue )];
                    curve->setSummaryCaseY( summaryCaseY );
                }
            }

            // Replace placeholders with object names from selection
            auto curveAdr = curve->summaryAddressY();
            setPlaceholderWellName( &curveAdr, wellNames );
            setPlaceholderGroupName( &curveAdr, groupNames );
            setPlaceholderRegion( &curveAdr, regions );
            curve->setSummaryAddressY( curveAdr );
        }

        for ( const auto& curveSet : summaryPlot->curveSets() )
        {
            const QString summaryGroupFieldKeyword = RicSummaryPlotTemplateTools::summaryGroupFieldKeyword();

            auto fieldHandle = curveSet->findField( summaryGroupFieldKeyword );
            if ( fieldHandle )
            {
                bool          conversionOk      = false;
                const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup();

                auto referenceString = fieldHandle->xmlCapability()->referenceString();
                int  indexValue =
                    RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( selectedEnsembles.size() ) )
                {
                    auto ensembleCase = selectedEnsembles[static_cast<int>( indexValue )];
                    curveSet->setSummaryCaseCollection( ensembleCase );
                }
            }

            // Replace placeholders with object names from selection
            auto curveAdr = curveSet->summaryAddress();
            setPlaceholderWellName( &curveAdr, wellNames );
            setPlaceholderGroupName( &curveAdr, groupNames );
            setPlaceholderRegion( &curveAdr, regions );
            curveSet->setSummaryAddress( curveAdr );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::htmlTextFromPlotAndSelection( const RimSummaryPlot* templatePlot,
                                                                   const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
                                                                   const std::vector<caf::PdmObject*>& selectedSources )
{
    QString text;

    RiaSummaryAddressAnalyzer selectionAnalyzer;

    selectionAnalyzer.appendAddresses( selectedSummaryAddresses );

    if ( templatePlot )
    {
        std::set<QString>         templateSources;
        RiaSummaryAddressAnalyzer templateAnalyzer;

        {
            std::set<RifEclipseSummaryAddress> templateAddresses;

            for ( const auto& curve : templatePlot->summaryCurves() )
            {
                auto adr = curve->summaryAddressY();
                templateAddresses.insert( adr );

                auto fieldHandle = curve->findField( "SummaryCase" );
                if ( fieldHandle )
                {
                    auto test = fieldHandle->xmlCapability()->referenceString();
                    templateSources.insert( test );
                }
            }

            templateAnalyzer.appendAddresses( templateAddresses );
        }

        text += "<b> Requirements </b><br>";

        if ( !templateSources.empty() )
        {
            QString itemText      = "Source Cases";
            size_t  requiredCount = templateSources.size();
            size_t  selectedCount = selectedSources.size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }

        if ( !templateAnalyzer.wellNames().empty() )
        {
            QString itemText      = "Wells";
            size_t  requiredCount = templateAnalyzer.wellNames().size();
            size_t  selectedCount = selectionAnalyzer.wellNames().size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }

        if ( !templateAnalyzer.groupNames().empty() )
        {
            QString itemText      = "Groups";
            size_t  requiredCount = templateAnalyzer.groupNames().size();
            size_t  selectedCount = selectionAnalyzer.groupNames().size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }

        if ( !templateAnalyzer.regionNumbers().empty() )
        {
            QString itemText      = "Regions";
            size_t  requiredCount = templateAnalyzer.regionNumbers().size();
            size_t  selectedCount = selectionAnalyzer.regionNumbers().size();
            text += htmlTextFromCount( itemText, requiredCount, selectedCount );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::htmlTextFromCount( const QString& itemText, size_t requiredItemCount, size_t selectionCount )
{
    QString text;

    QString colorString = "green";

    if ( selectionCount < requiredItemCount )
        colorString = "red";
    else if ( selectionCount > requiredItemCount )
        colorString = "orange";

    text += QString( "<b><font color='%1'>" ).arg( colorString );
    text += QString( "%1 : %2 selected (%3 required)\n" ).arg( itemText ).arg( selectionCount ).arg( requiredItemCount );
    text += "</font></b>";

    text += "<br>";

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::selectPlotTemplatePath()
{
    RiuPlotMainWindow*      plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    RicSelectPlotTemplateUi ui;

    caf::PdmUiPropertyViewDialog propertyDialog( plotwindow, &ui, "Select Plot Template", "" );
    propertyDialog.resize( QSize( 400, 600 ) );

    if ( propertyDialog.exec() == QDialog::Accepted && !ui.selectedPlotTemplates().empty() )
    {
        QString fileName = ui.selectedPlotTemplates().front()->absoluteFilePath();

        RiaPreferences::current()->setDefaultPlotTemplatePath( fileName );
        RiaPreferences::current()->writePreferencesToApplicationStore();

        return fileName;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicSummaryPlotTemplateTools::selectDefaultPlotTemplates( std::vector<QString> currentSelection )
{
    RiuPlotMainWindow*      plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    RicSelectPlotTemplateUi ui;
    ui.setMultiSelectMode( true );
    ui.setInitialSelection( currentSelection );

    caf::PdmUiPropertyViewDialog propertyDialog( plotwindow, &ui, "Select Default Plot Templates", "" );
    propertyDialog.resize( QSize( 500, 600 ) );

    std::vector<QString> selection;

    if ( propertyDialog.exec() == QDialog::Accepted && !ui.selectedPlotTemplates().empty() )
    {
        for ( auto item : ui.selectedPlotTemplates() )
        {
            selection.push_back( item->absoluteFilePath() );
        }
    }

    return selection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicSummaryPlotTemplateTools::selectedSummaryCases()
{
    std::vector<RimSummaryCase*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCaseCollection*> RicSummaryPlotTemplateTools::selectedSummaryCaseCollections()
{
    std::vector<RimSummaryCaseCollection*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressCollection*> RicSummaryPlotTemplateTools::selectedSummaryAddressCollections()
{
    std::vector<RimSummaryAddressCollection*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::summaryCaseFieldKeyword()
{
    return "SummaryCase";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::summaryGroupFieldKeyword()
{
    return "SummaryGroup";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::placeholderTextForSummaryCase()
{
    return "__CASE_NAME__";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup()
{
    return "__ENSEMBLE_NAME__";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::placeholderTextForWell()
{
    return "__WELL_NAME__";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::placeholderTextForGroup()
{
    return "__GROUP__";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress
    RicSummaryPlotTemplateTools::firstAddressByQuantity( const RifEclipseSummaryAddress&           sourceAddress,
                                                         const std::set<RifEclipseSummaryAddress>& allAddresses )
{
    for ( const auto& a : allAddresses )
    {
        if ( sourceAddress.vectorName() == a.vectorName() )
        {
            return a;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicSummaryPlotTemplateTools::findValueForKeyword( const QString& keyword, const QString& valueString, bool* ok )
{
    // Example string : "CASE_NAME 1"
    // Will match the string specified by keyword, and return the value captured by the regexp

    QString            regexpString = QString( "%1 (\\d++)" ).arg( keyword );
    QRegularExpression rx( regexpString );

    auto match = rx.match( valueString );
    if ( match.hasMatch() )
    {
        QString integerAsText = match.captured( 1 );

        if ( !integerAsText.isEmpty() )
        {
            int integerValue = integerAsText.toInt();

            if ( ok )
            {
                *ok = true;
            }
            return integerValue;
        }
    }

    if ( ok )
    {
        *ok = false;
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::setPlaceholderWellName( RifEclipseSummaryAddress*   summaryAddress,
                                                          const std::vector<QString>& wellNames )
{
    if ( wellNames.empty() ) return;

    auto          sourceWellName    = QString::fromStdString( summaryAddress->wellName() );
    bool          conversionOk      = false;
    const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForWell();

    int indexValue = RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, sourceWellName, &conversionOk );
    if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( wellNames.size() ) )
    {
        summaryAddress->setWellName( wellNames[indexValue].toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::setPlaceholderGroupName( RifEclipseSummaryAddress*   summaryAddress,
                                                           const std::vector<QString>& groupNames )
{
    if ( groupNames.empty() ) return;

    auto          sourceGroupName   = QString::fromStdString( summaryAddress->groupName() );
    bool          conversionOk      = false;
    const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForGroup();

    int indexValue = RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, sourceGroupName, &conversionOk );
    if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( groupNames.size() ) )
    {
        summaryAddress->setGroupName( groupNames[indexValue].toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::setPlaceholderRegion( RifEclipseSummaryAddress*   summaryAddress,
                                                        const std::vector<QString>& regions )
{
    CAF_ASSERT( summaryAddress );
    if ( regions.empty() ) return;

    int indexValue = summaryAddress->regionNumber();
    if ( indexValue < -1 )
    {
        indexValue = -( indexValue - 2 );

        bool conversionOk = false;
        if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( regions.size() ) )
        {
            summaryAddress->setRegion( regions[indexValue].toInt() );
        }
    }
}

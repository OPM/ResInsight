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

#include "RiaSummaryPlotTemplateTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "Summary/RiaSummaryAddressAnalyzer.h"
#include "Summary/RiaSummaryTools.h"

#include "PlotTemplateCommands/RicSelectPlotTemplateUi.h"

#include "RifSummaryReaderInterface.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimDialogData.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
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
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( objectAsText, caf::PdmDefaultObjectFactory::instance(), true );

    return dynamic_cast<RimSummaryMultiPlot*>( obj );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotTemplateTools::create( const QString& fileName )
{
    auto sumCases     = RicSummaryPlotTemplateTools::selectedSummaryCases();
    auto sumEnsembles = RicSummaryPlotTemplateTools::selectedSummaryEnsembles();

    auto summaryAddressCollections = RicSummaryPlotTemplateTools::selectedSummaryAddressCollections();

    std::vector<QString>          wellNames;
    std::vector<QString>          groupNames;
    std::vector<QString>          regions;
    std::set<RimSummaryCase*>     caseSet;
    std::set<RimSummaryEnsemble*> caseCollectionSet;

    if ( summaryAddressCollections.empty() )
    {
        RiaSummaryAddressAnalyzer analyzer;

        if ( !sumCases.empty() )
        {
            auto firstCase = sumCases.front();

            analyzer.appendAddresses( firstCase->summaryReader()->allResultAddresses() );
        }
        else if ( !sumEnsembles.empty() )
        {
            auto caseCollection = sumEnsembles.front();

            auto firstCase = caseCollection->firstSummaryCase();
            if ( firstCase != nullptr )
            {
                analyzer.appendAddresses( firstCase->summaryReader()->allResultAddresses() );
            }
        }

        if ( !analyzer.wellNames().empty() ) wellNames.push_back( QString::fromStdString( *( analyzer.wellNames().begin() ) ) );
        if ( !analyzer.groupNames().empty() ) groupNames.push_back( QString::fromStdString( *( analyzer.groupNames().begin() ) ) );
        if ( !analyzer.regionNumbers().empty() ) regions.push_back( QString::number( *( analyzer.regionNumbers().begin() ) ) );
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
        sumEnsembles.push_back( sumCaseCollection );
    }

    auto collections = RimMainPlotCollection::current()->summaryMultiPlotCollection();

    auto newSummaryPlot = RicSummaryPlotTemplateTools::createMultiPlotFromTemplateFile( fileName );
    if ( !newSummaryPlot ) return nullptr;

    collections->addSummaryMultiPlot( newSummaryPlot );
    newSummaryPlot->resolveReferencesRecursively();

    RicSummaryPlotTemplateTools::setValuesForPlaceholders( newSummaryPlot, sumCases, sumEnsembles, wellNames, groupNames, regions );
    newSummaryPlot->initAfterReadRecursively();
    newSummaryPlot->loadDataAndUpdate();

    collections->updateConnectedEditors();

    return newSummaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSummaryPlotTemplateTools::create( const QString&                          fileName,
                                                          const std::vector<RimSummaryCase*>&     sumCases,
                                                          const std::vector<RimSummaryEnsemble*>& ensembles )
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

    if ( !analyzer.wellNames().empty() ) wellNames.push_back( QString::fromStdString( *( analyzer.wellNames().begin() ) ) );
    if ( !analyzer.groupNames().empty() ) groupNames.push_back( QString::fromStdString( *( analyzer.groupNames().begin() ) ) );
    if ( !analyzer.regionNumbers().empty() ) regions.push_back( QString::number( *( analyzer.regionNumbers().begin() ) ) );

    auto collections = RimMainPlotCollection::current()->summaryMultiPlotCollection();

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
void RicSummaryPlotTemplateTools::setValuesForPlaceholders( RimSummaryMultiPlot*                    summaryMultiPlot,
                                                            const std::vector<RimSummaryCase*>&     selectedSummaryCases,
                                                            const std::vector<RimSummaryEnsemble*>& selectedEnsembles,
                                                            const std::vector<QString>&             wellNames,
                                                            const std::vector<QString>&             groupNames,
                                                            const std::vector<QString>&             regions )

{
    auto plots = summaryMultiPlot->plots();
    for ( auto p : plots )
    {
        auto summaryPlot = dynamic_cast<RimSummaryPlot*>( p );
        if ( summaryPlot ) setValuesForPlaceholders( summaryPlot, selectedSummaryCases, selectedEnsembles, wellNames, groupNames, regions );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::setValuesForPlaceholders( RimSummaryPlot*                         summaryPlot,
                                                            const std::vector<RimSummaryCase*>&     selectedSummaryCases,
                                                            const std::vector<RimSummaryEnsemble*>& selectedEnsembles,
                                                            const std::vector<QString>&             wellNames,
                                                            const std::vector<QString>&             groupNames,
                                                            const std::vector<QString>&             regions )
{
    {
        // Replace single summary curves data sources

        auto summaryCurves = summaryPlot->allCurves();
        for ( const auto& curve : summaryCurves )
        {
            auto summaryCaseHandle = curve->findField( RicSummaryPlotTemplateTools::summaryCaseFieldKeyword() );
            if ( summaryCaseHandle )
            {
                bool          conversionOk      = false;
                const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForSummaryCase();

                auto referenceString = summaryCaseHandle->xmlCapability()->referenceString();
                int  indexValue = RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( selectedSummaryCases.size() ) )
                {
                    auto summaryCaseY = selectedSummaryCases[static_cast<int>( indexValue )];
                    curve->setSummaryCaseY( summaryCaseY );
                }
            }

            auto summaryCaseXHandle = curve->findField( RicSummaryPlotTemplateTools::summaryCaseXFieldKeyword() );
            if ( summaryCaseXHandle )
            {
                bool          conversionOk      = false;
                const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForSummaryCaseX();

                auto referenceString = summaryCaseXHandle->xmlCapability()->referenceString();
                int  indexValue = RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( selectedSummaryCases.size() ) )
                {
                    auto summaryCase = selectedSummaryCases[static_cast<int>( indexValue )];
                    curve->setSummaryCaseX( summaryCase );
                }
            }

            // Replace placeholders with object names from selection
            auto curveAdr = curve->summaryAddressY();
            setPlaceholderWellName( &curveAdr, wellNames );
            setPlaceholderGroupName( &curveAdr, groupNames );
            setPlaceholderRegion( &curveAdr, regions );
            curve->setSummaryAddressY( curveAdr );

            if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
            {
                auto curveAdr = curve->summaryAddressX();
                setPlaceholderWellName( &curveAdr, wellNames );
                setPlaceholderGroupName( &curveAdr, groupNames );
                setPlaceholderRegion( &curveAdr, regions );
                curve->setSummaryAddressX( curveAdr );
            }
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
                int  indexValue = RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( selectedEnsembles.size() ) )
                {
                    auto ensembleCase = selectedEnsembles[static_cast<int>( indexValue )];
                    curveSet->setSummaryEnsemble( ensembleCase );
                }
            }

            // Replace placeholders with object names from selection
            auto adr       = curveSet->curveAddress();
            auto curveAdrY = adr.summaryAddressY();
            setPlaceholderWellName( &curveAdrY, wellNames );
            setPlaceholderGroupName( &curveAdrY, groupNames );
            setPlaceholderRegion( &curveAdrY, regions );
            auto curveAdrX = adr.summaryAddressX();
            setPlaceholderWellName( &curveAdrX, wellNames );
            setPlaceholderGroupName( &curveAdrX, groupNames );
            setPlaceholderRegion( &curveAdrX, regions );
            curveSet->setCurveAddress( RiaSummaryCurveAddress( curveAdrX, curveAdrY ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::htmlTextFromPlotAndSelection( const RimSummaryPlot*                     templatePlot,
                                                                   const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
                                                                   const std::vector<caf::PdmObject*>&       selectedSources )
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

        RiaPreferences::current()->setLastUsedPlotTemplatePath( fileName );
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
    return caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> RicSummaryPlotTemplateTools::selectedSummaryEnsembles()
{
    return caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressCollection*> RicSummaryPlotTemplateTools::selectedSummaryAddressCollections()
{
    return caf::SelectionManager::instance()->objectsByType<RimSummaryAddressCollection>();
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
QString RicSummaryPlotTemplateTools::summaryCaseXFieldKeyword()
{
    return "SummaryCaseX";
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
QString RicSummaryPlotTemplateTools::placeholderTextForSummaryCaseX()
{
    return "__CASE_NAME_X__";
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
RifEclipseSummaryAddress RicSummaryPlotTemplateTools::firstAddressByQuantity( const RifEclipseSummaryAddress&           sourceAddress,
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
void RicSummaryPlotTemplateTools::setPlaceholderWellName( RifEclipseSummaryAddress* summaryAddress, const std::vector<QString>& wellNames )
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
void RicSummaryPlotTemplateTools::setPlaceholderGroupName( RifEclipseSummaryAddress* summaryAddress, const std::vector<QString>& groupNames )
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
void RicSummaryPlotTemplateTools::setPlaceholderRegion( RifEclipseSummaryAddress* summaryAddress, const std::vector<QString>& regions )
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

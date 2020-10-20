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

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaSummaryCurveAnalyzer.h"

#include "RicSelectPlotTemplateUi.h"

#include "RifSummaryReaderInterface.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimDialogData.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QFile>
#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotTemplateTools::createPlotFromTemplateFile( const QString& fileName )
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

    RimSummaryPlot* newSummaryPlot = dynamic_cast<RimSummaryPlot*>( obj );
    if ( newSummaryPlot )
    {
        return newSummaryPlot;
    }

    delete obj;

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotTemplateTools::appendSummaryPlotToPlotCollection(
    RimSummaryPlot*                               summaryPlot,
    const std::vector<RimSummaryCase*>&           selectedSummaryCases,
    const std::vector<RimSummaryCaseCollection*>& selectedEnsembles )
{
    if ( !summaryPlot ) return;

    if ( selectedSummaryCases.empty() && selectedEnsembles.empty() ) return;

    RimSummaryPlotCollection* plotColl = RimProject::current()->mainPlotCollection()->summaryPlotCollection();

    plotColl->addPlot( summaryPlot );
    summaryPlot->resolveReferencesRecursively();
    summaryPlot->initAfterReadRecursively();

    {
        // Replace single summary curves data sources

        auto summaryCurves = summaryPlot->summaryCurves();

        const QString summaryFieldKeyword = RicSummaryPlotTemplateTools::summaryCaseFieldKeyword();

        int maximumIndexValue = -1;
        for ( const auto& curve : summaryCurves )
        {
            auto fieldHandle = curve->findField( summaryFieldKeyword );
            if ( fieldHandle )
            {
                bool          conversionOk      = false;
                const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForSummaryCase();

                auto referenceString = fieldHandle->xmlCapability()->referenceString();
                int  indexValue =
                    RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                maximumIndexValue = std::max( maximumIndexValue, indexValue );

                if ( conversionOk && indexValue >= 0 && indexValue < static_cast<int>( selectedSummaryCases.size() ) )
                {
                    auto summaryCaseY = selectedSummaryCases[static_cast<int>( indexValue )];
                    curve->setSummaryCaseY( summaryCaseY );

                    auto currentAddressY = curve->summaryAddressY();
                    if ( summaryCaseY->summaryReader() && !summaryCaseY->summaryReader()->hasAddress( currentAddressY ) )
                    {
                        auto allAddresses = summaryCaseY->summaryReader()->allResultAddresses();

                        auto candidate =
                            RicSummaryPlotTemplateTools::firstAddressByQuantity( currentAddressY, allAddresses );
                        if ( candidate.category() != RifEclipseSummaryAddress::SUMMARY_INVALID )
                        {
                            curve->setSummaryAddressY( candidate );
                        }
                    }
                }
            }
        }

        if ( selectedSummaryCases.size() > static_cast<size_t>( maximumIndexValue + 1 ) )
        {
            // Use the curve style of the last curve in template, and duplicate this for remaining data sources

            if ( !summaryCurves.empty() )
            {
                auto lastSummaryCurve = summaryCurves.back();

                for ( size_t i = maximumIndexValue; i < selectedSummaryCases.size(); i++ )
                {
                    auto newCurve =
                        dynamic_cast<RimSummaryCurve*>( lastSummaryCurve->xmlCapability()->copyByXmlSerialization(
                            caf::PdmDefaultObjectFactory::instance() ) );

                    auto summaryCaseY = selectedSummaryCases[i];
                    newCurve->setSummaryCaseY( summaryCaseY );
                    summaryPlot->addCurveAndUpdate( newCurve );
                }
            }
        }
    }

    {
        // Replace ensemble data sources

        auto summaryCurveSets = summaryPlot->ensembleCurveSetCollection()->curveSets();

        const QString summaryGroupFieldKeyword = RicSummaryPlotTemplateTools::summaryGroupFieldKeyword();

        int maximumIndexValue = -1;

        for ( const auto& curveSet : summaryCurveSets )
        {
            auto fieldHandle = curveSet->findField( summaryGroupFieldKeyword );
            if ( fieldHandle )
            {
                bool          conversionOk      = false;
                const QString placeholderString = RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup();

                auto referenceString = fieldHandle->xmlCapability()->referenceString();
                int  indexValue =
                    RicSummaryPlotTemplateTools::findValueForKeyword( placeholderString, referenceString, &conversionOk );

                maximumIndexValue = std::max( maximumIndexValue, indexValue );

                if ( conversionOk && indexValue < static_cast<int>( selectedEnsembles.size() ) )
                {
                    auto summaryCaseY = selectedEnsembles[indexValue];
                    curveSet->setSummaryCaseCollection( summaryCaseY );
                }
            }
        }

        if ( selectedEnsembles.size() > static_cast<size_t>( maximumIndexValue + 1 ) )
        {
            // Use the curve style of the last curve in template, and duplicate this for remaining data sources

            if ( !summaryCurveSets.empty() )
            {
                auto lastSummaryCurveSet = summaryCurveSets.back();

                for ( size_t i = maximumIndexValue; i < selectedEnsembles.size(); i++ )
                {
                    auto newCurveSet =
                        dynamic_cast<RimEnsembleCurveSet*>( lastSummaryCurveSet->xmlCapability()->copyByXmlSerialization(
                            caf::PdmDefaultObjectFactory::instance() ) );

                    auto ensembleDataSource = selectedEnsembles[i];
                    newCurveSet->setSummaryCaseCollection( ensembleDataSource );

                    summaryPlot->ensembleCurveSetCollection()->addCurveSet( newCurveSet );
                }
            }
        }
    }

    plotColl->updateConnectedEditors();

    summaryPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::htmlTextFromPlotAndSelection( const RimSummaryPlot* templatePlot,
                                                                   const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
                                                                   const std::vector<caf::PdmObject*>& selectedSources )
{
    QString text;

    RiaSummaryCurveAnalyzer selectionAnalyzer;

    selectionAnalyzer.appendAddresses( selectedSummaryAddresses );

    if ( templatePlot )
    {
        std::set<QString>       templateSources;
        RiaSummaryCurveAnalyzer templateAnalyzer;

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

        if ( !templateAnalyzer.wellGroupNames().empty() )
        {
            QString itemText      = "Well Groups";
            size_t  requiredCount = templateAnalyzer.wellGroupNames().size();
            size_t  selectedCount = selectionAnalyzer.wellGroupNames().size();
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

        RiaApplication::instance()->preferences()->setDefaultPlotTemplatePath( fileName );
        RiaApplication::instance()->preferences()->writePreferencesToApplicationStore();

        return fileName;
    }

    return QString();
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
    return "CASE_NAME";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup()
{
    return "ENSEMBLE_NAME";
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
        if ( sourceAddress.quantityName() == a.quantityName() )
        {
            return a;
        }
    }

    return RifEclipseSummaryAddress();
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

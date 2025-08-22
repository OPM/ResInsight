////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicSaveMultiPlotTemplateFeature.h"

#include "RicReloadPlotTemplatesFeature.h"
#include "RicSaveMultiPlotTemplateFeatureSettings.h"
#include "Summary/RiaSummaryPlotTemplateTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "Summary/RiaSummaryAddressAnalyzer.h"
#include "Summary/RiaSummaryTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuFileDialogTools.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicSaveMultiPlotTemplateFeature, "RicSaveMultiPlotTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveMultiPlotTemplateFeature::isCommandEnabled() const
{
    return selectedSummaryPlot() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeature::onActionTriggered( bool isChecked )
{
    if ( !selectedSummaryPlot() ) return;

    RiaGuiApplication* app = RiaGuiApplication::instance();

    QString fallbackPath;
    auto    folders = app->preferences()->plotTemplateFolders();
    if ( !folders.empty() )
    {
        // Use the last folder from preferences as the default fall back folder
        fallbackPath = folders.back();
    }

    QString startPath = app->lastUsedDialogDirectoryWithFallback( "PLOT_TEMPLATE", fallbackPath );

    QString templateCandidateName = caf::Utils::makeValidFileBasename( selectedSummaryPlot()->description() );

    RicSaveMultiPlotTemplateFeatureSettings settings;
    settings.setFilePath( startPath );
    settings.setName( templateCandidateName );

    caf::PdmUiPropertyViewDialog propertyDialog( RiuPlotMainWindow::instance(), &settings, "Export Plot Template", "" );
    propertyDialog.resize( QSize( 600, 400 ) );

    if ( propertyDialog.exec() != QDialog::Accepted ) return;

    auto plot = selectedSummaryPlot();
    if ( !plot ) return;

    plot->storeStepDimensionFromToolbar();

    QString ext = ".rpt";
    if ( !selectedSummaryPlot()->curveSets().empty() ) ext = ".erpt";

    QString fileName = settings.filePath() + "/" + settings.name() + ext;
    if ( !fileName.isEmpty() )
    {
        QFile exportFile( fileName );
        if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            RiaLogging::errorInMessageBox( nullptr, "Save Plot Template", QString( "Could not save to the file: %1" ).arg( fileName ) );
            return;
        }

        QString objectAsText = createTextFromObject( selectedSummaryPlot(), settings );

        QTextStream stream( &exportFile );
        stream << objectAsText;

        QString absPath                = QFileInfo( fileName ).absolutePath();
        bool    foundPathInPreferences = false;
        for ( const auto& f : folders )
        {
            if ( absPath.indexOf( f ) != -1 )
            {
                foundPathInPreferences = true;
            }
        }

        if ( !foundPathInPreferences )
        {
            QMessageBox msgBox;
            msgBox.setIcon( QMessageBox::Question );

            QString questionText;
            questionText = QString( "The path is not part of the search path for templates.\n\nDo you want to append "
                                    "the destination path to the search path?" );

            msgBox.setText( questionText );
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );

            int ret = msgBox.exec();
            if ( ret == QMessageBox::Yes )
            {
                app->preferences()->appendPlotTemplateFolders( absPath );
                app->preferences()->writePreferencesToApplicationStore();
            }
        }

        app->setLastUsedDialogDirectory( "PLOT_TEMPLATE", absPath );

        RicReloadPlotTemplatesFeature::rebuildFromDisc();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSaveMultiPlotTemplateFeature::createTextFromObject( RimSummaryMultiPlot*                           summaryPlot,
                                                               const RicSaveMultiPlotTemplateFeatureSettings& settings )
{
    if ( !summaryPlot ) return {};

    QString objectAsText = summaryPlot->writeObjectToXmlString();

    RiaSummaryAddressAnalyzer analyzer;

    {
        std::vector<RifEclipseSummaryAddress> addresses;
        {
            std::set<QString> sourceStrings;

            const QString summaryFieldKeyword = RicSummaryPlotTemplateTools::summaryCaseFieldKeyword();
            for ( const auto& curve : summaryPlot->allCurves() )
            {
                auto fieldHandle = curve->findField( summaryFieldKeyword );
                if ( fieldHandle )
                {
                    auto reference = caf::PdmReferenceHelper::referenceFromFieldToObject( fieldHandle, curve->summaryCaseY() );

                    sourceStrings.insert( reference );
                }

                addresses.push_back( curve->summaryAddressY() );
            }

            replaceStrings( sourceStrings, summaryFieldKeyword, RicSummaryPlotTemplateTools::placeholderTextForSummaryCase(), objectAsText );
        }

        {
            std::set<QString> sourceStrings;

            const QString summaryFieldKeyword = RicSummaryPlotTemplateTools::summaryCaseXFieldKeyword();
            for ( const auto& curve : summaryPlot->allCurves() )
            {
                if ( curve->axisTypeX() != RiaDefines::HorizontalAxisType::SUMMARY_VECTOR ) continue;

                auto fieldHandle = curve->findField( summaryFieldKeyword );
                if ( fieldHandle )
                {
                    auto reference = caf::PdmReferenceHelper::referenceFromFieldToObject( fieldHandle, curve->summaryCaseX() );

                    sourceStrings.insert( reference );
                }

                addresses.push_back( curve->summaryAddressX() );
            }

            replaceStrings( sourceStrings, summaryFieldKeyword, RicSummaryPlotTemplateTools::placeholderTextForSummaryCaseX(), objectAsText );
        }

        {
            std::set<QString> ensembleReferenceStrings;

            const QString summaryGroupFieldKeyword = RicSummaryPlotTemplateTools::summaryGroupFieldKeyword();

            for ( const auto& curveSet : summaryPlot->curveSets() )
            {
                auto fieldHandle = curveSet->findField( summaryGroupFieldKeyword );
                if ( fieldHandle )
                {
                    auto reference = caf::PdmReferenceHelper::referenceFromFieldToObject( fieldHandle, curveSet->summaryEnsemble() );
                    ensembleReferenceStrings.insert( reference );
                }

                addresses.push_back( curveSet->summaryAddressY() );
            }

            replaceStrings( ensembleReferenceStrings,
                            summaryGroupFieldKeyword,
                            RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup(),
                            objectAsText );
        }

        analyzer.appendAddresses( addresses );
    }

    RimSummaryAddress dummy;

    if ( settings.usePlacholderForWells() )
    {
        std::set<QString> sourceStrings;
        for ( const auto& wellName : analyzer.wellNames() )
        {
            sourceStrings.insert( QString::fromStdString( wellName ) );
        }

        replaceStrings( sourceStrings,
                        dummy.keywordForCategory( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL ),
                        RicSummaryPlotTemplateTools::placeholderTextForWell(),
                        objectAsText );
    }

    if ( settings.usePlacholderForGroups() )
    {
        std::set<QString> sourceStrings;
        for ( const auto& groupName : analyzer.groupNames() )
        {
            sourceStrings.insert( QString::fromStdString( groupName ) );
        }

        replaceStrings( sourceStrings,
                        dummy.keywordForCategory( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP ),
                        RicSummaryPlotTemplateTools::placeholderTextForGroup(),
                        objectAsText );
    }

    if ( settings.usePlacholderForRegions() )
    {
        std::vector<int> regionNumbers;
        for ( auto regNumber : analyzer.regionNumbers() )
        {
            regionNumbers.push_back( regNumber );
        }

        for ( int i = 0; i < static_cast<int>( regionNumbers.size() ); i++ )
        {
            // Encode placeholder index. Use negative values below -1 to represent a placeholder index
            int index = -( i + 2 );

            QString fieldKeyword             = dummy.keywordForCategory( RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION );
            QString sourceString             = QString( "<%1>%2</%1>" ).arg( fieldKeyword ).arg( regionNumbers[i] );
            QString replacementTextWithIndex = QString( "<%1>%2</%1>" ).arg( fieldKeyword ).arg( index );

            objectAsText.replace( sourceString, replacementTextWithIndex );
        }
    }

    return objectAsText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeature::replaceStrings( const std::set<QString>& sourceStrings,
                                                      const QString&           fieldKeyword,
                                                      const QString&           placeholderText,
                                                      QString&                 objectAsText )
{
    size_t index = 0;
    for ( const auto& sourceString : sourceStrings )
    {
        QString sourceTextWithTags = QString( "<%1>%2</%1>" ).arg( fieldKeyword ).arg( sourceString );

        QString replacementTextWithIndex = QString( "<%1>%2 %3</%1>" ).arg( fieldKeyword ).arg( placeholderText ).arg( index++ );

        objectAsText.replace( sourceTextWithTags, replacementTextWithIndex );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save As Plot Template" );
    actionToSetup->setIcon( QIcon( ":/plot-template-standard.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSaveMultiPlotTemplateFeature::selectedSummaryPlot() const
{
    return dynamic_cast<RimSummaryMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );
}

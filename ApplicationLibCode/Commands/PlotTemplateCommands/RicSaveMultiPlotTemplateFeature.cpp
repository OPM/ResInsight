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

#include "RicSaveMultiPlotTemplateFeature.h"

#include "RicReloadPlotTemplatesFeature.h"
#include "RicSummaryPlotTemplateTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuFileDialogTools.h"

#include "cafPdmObject.h"
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
bool RicSaveMultiPlotTemplateFeature::isCommandEnabled()
{
    if ( selectedSummaryPlot() ) return true;

    return false;
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

    startPath = startPath + "/" + templateCandidateName + ".rpt";

    QString fileName = RiuFileDialogTools::getSaveFileName( nullptr,
                                                            tr( "Save Plot Template To File" ),
                                                            startPath,
                                                            tr( "Plot Template Files (*.rpt);;All files(*.*)" ) );
    if ( !fileName.isEmpty() )
    {
        QFile exportFile( fileName );
        if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            RiaLogging::error( QString( "Save Plot Template : Could not open the file: %1" ).arg( fileName ) );
            return;
        }

        QString objectAsText = createTextFromObject( selectedSummaryPlot() );

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
QString RicSaveMultiPlotTemplateFeature::createTextFromObject( RimSummaryMultiPlot* summaryPlot )
{
    if ( !summaryPlot ) return {};

    QString objectAsText = summaryPlot->writeObjectToXmlString();

    {
        RiaSummaryAddressAnalyzer analyzer;

        {
            std::vector<RifEclipseSummaryAddress> addresses;
            std::set<QString>                     sourceStrings;

            const QString summaryFieldKeyword = RicSummaryPlotTemplateTools::summaryCaseFieldKeyword();
            for ( const auto& curve : summaryPlot->allCurves( RimSummaryDataSourceStepping::Axis::Y_AXIS ) )
            {
                auto fieldHandle = curve->findField( summaryFieldKeyword );
                if ( fieldHandle )
                {
                    auto reference = fieldHandle->xmlCapability()->referenceString();
                    sourceStrings.insert( reference );
                }

                addresses.push_back( curve->summaryAddressY() );
            }

            replaceStrings( sourceStrings,
                            "SummaryCase",
                            RicSummaryPlotTemplateTools::placeholderTextForSummaryCase(),
                            objectAsText );

            {
                std::set<QString> ensembleReferenceStrings;

                const QString summaryGroupFieldKeyword = RicSummaryPlotTemplateTools::summaryGroupFieldKeyword();

                for ( const auto& curveSet : summaryPlot->curveSets() )
                {
                    auto fieldHandle = curveSet->findField( summaryGroupFieldKeyword );
                    if ( fieldHandle )
                    {
                        auto reference = fieldHandle->xmlCapability()->referenceString();
                        ensembleReferenceStrings.insert( reference );
                    }

                    addresses.push_back( curveSet->summaryAddress() );
                }

                replaceStrings( ensembleReferenceStrings,
                                "SummaryGroupCase",
                                RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup(),
                                objectAsText );
            }

            analyzer.appendAddresses( addresses );
        }

        {
            std::set<QString> sourceStrings;

            for ( const auto& wellName : analyzer.wellNames() )
            {
                sourceStrings.insert( QString::fromStdString( wellName ) );
            }

            replaceStrings( sourceStrings, "SummaryWell", RicSummaryPlotTemplateTools::placeholderTextForWell(), objectAsText );
        }

        {
            std::set<QString> sourceStrings;

            for ( const auto& wellGroupName : analyzer.wellGroupNames() )
            {
                sourceStrings.insert( QString::fromStdString( wellGroupName ) );
            }

            replaceStrings( sourceStrings,
                            "SummaryWellGroup",
                            RicSummaryPlotTemplateTools::placeholderTextForWellGroup(),
                            objectAsText );
        }

        {
            std::vector<int> regionNumbers;
            for ( auto regNumber : analyzer.regionNumbers() )
            {
                regionNumbers.push_back( regNumber );
            }

            for ( auto i = 0; i < regionNumbers.size(); i++ )
            {
                // Encode placeholder index. Use negative values below -1 to represent a placeholder index
                int index = -( i + 2 );

                QString fieldKeyword             = "SummaryRegion";
                QString replacementTextWithIndex = QString( "<%1>%2</%1>" ).arg( fieldKeyword ).arg( index );
                QString sourceReplacementString  = QString( "<%1>%2</%1>" ).arg( fieldKeyword ).arg( regionNumbers[i] );

                objectAsText.replace( sourceReplacementString, replacementTextWithIndex );
            }
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
        QString replacementTextWithIndex =
            QString( "<%1>%2 %3</%1>" ).arg( fieldKeyword ).arg( placeholderText ).arg( index++ );

        QString sourceReplacementString = QString( "<%1>%2</%1>" ).arg( fieldKeyword ).arg( sourceString );

        objectAsText.replace( sourceReplacementString, replacementTextWithIndex );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save As Plot Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* RicSaveMultiPlotTemplateFeature::selectedSummaryPlot() const
{
    return dynamic_cast<RimSummaryMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );
}

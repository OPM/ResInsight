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

#include "RicSavePlotTemplateFeature.h"

#include "RicReloadPlotTemplatesFeature.h"
#include "RicSummaryPlotTemplateTools.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaSummaryTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "RiuFileDialogTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicSavePlotTemplateFeature, "RicSavePlotTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSavePlotTemplateFeature::isCommandEnabled()
{
    if ( selectedSummaryPlot() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSavePlotTemplateFeature::onActionTriggered( bool isChecked )
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
QString RicSavePlotTemplateFeature::createTextFromObject( RimSummaryPlot* summaryPlot )
{
    if ( !summaryPlot ) return QString();

    QString objectAsText = summaryPlot->writeObjectToXmlString();

    caf::PdmObjectHandle* obj =
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( objectAsText,
                                                                 caf::PdmDefaultObjectFactory::instance(),
                                                                 true );

    RimSummaryPlot* newSummaryPlot = dynamic_cast<RimSummaryPlot*>( obj );
    if ( newSummaryPlot )
    {
        {
            std::set<QString> caseReferenceStrings;

            const QString summaryFieldKeyword = RicSummaryPlotTemplateTools::summaryCaseFieldKeyword();
            for ( const auto& curve : newSummaryPlot->summaryCurves() )
            {
                auto fieldHandle = curve->findField( summaryFieldKeyword );
                if ( fieldHandle )
                {
                    auto reference = fieldHandle->xmlCapability()->referenceString();
                    caseReferenceStrings.insert( reference );
                }
            }

            size_t index = 0;
            for ( const auto& s : caseReferenceStrings )
            {
                QString placeholderText = RicSummaryPlotTemplateTools::placeholderTextForSummaryCase();
                QString caseName        = QString( "%1 %2" ).arg( placeholderText ).arg( index++ );

                objectAsText.replace( s, caseName );
            }
        }

        {
            std::set<QString> ensembleReferenceStrings;

            const QString summaryGroupFieldKeyword = RicSummaryPlotTemplateTools::summaryGroupFieldKeyword();

            for ( const auto& curveSet : newSummaryPlot->ensembleCurveSetCollection()->curveSets() )
            {
                auto fieldHandle = curveSet->findField( summaryGroupFieldKeyword );
                if ( fieldHandle )
                {
                    auto reference = fieldHandle->xmlCapability()->referenceString();
                    ensembleReferenceStrings.insert( reference );
                }
            }

            size_t index = 0;
            for ( const auto& s : ensembleReferenceStrings )
            {
                QString placeholderText = RicSummaryPlotTemplateTools::placeholderTextForSummaryGroup();
                QString ensembleName    = QString( "%1 %2" ).arg( placeholderText ).arg( index++ );

                objectAsText.replace( s, ensembleName );
            }
        }
    }

    delete obj;

    return objectAsText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSavePlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save As Plot Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSavePlotTemplateFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot( selObj );
    }

    return sumPlot;
}

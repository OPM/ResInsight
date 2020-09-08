/////////////////////////////////////////////////////////////////////////////////
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
#include "RicImportGeneralDataFeature.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"

#include "RicImportSummaryCasesFeature.h"

#include "RimSummaryCase.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>
#include <QString>
#include <QStringList>

using namespace RiaDefines;

CAF_CMD_SOURCE_INIT( RicImportGeneralDataFeature, "RicImportGeneralDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportGeneralDataFeature::OpenCaseResults
    RicImportGeneralDataFeature::openEclipseFilesFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot )
{
    CVF_ASSERT( !fileNames.empty() );

    QString defaultDir = QFileInfo( fileNames.last() ).absolutePath();

    QStringList eclipseCaseFiles;
    QStringList eclipseInputFiles;
    QStringList eclipseSummaryFiles;

    for ( const QString& fileName : fileNames )
    {
        int fileTypeAsInt = int( obtainFileTypeFromFileName( fileName ) );
        if ( fileTypeAsInt & ( int( ImportFileType::ECLIPSE_GRID_FILE ) | int( ImportFileType::ECLIPSE_EGRID_FILE ) ) )
        {
            eclipseCaseFiles.push_back( fileName );
        }
        else if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_INPUT_FILE ) )
        {
            eclipseInputFiles.push_back( fileName );
        }
        else if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_SUMMARY_FILE ) )
        {
            eclipseSummaryFiles.push_back( fileName );
        }
    }

    OpenCaseResults results;
    if ( !eclipseCaseFiles.empty() )
    {
        if ( !openEclipseCaseFromFileNames( eclipseCaseFiles ) )
        {
            return OpenCaseResults();
        }
        results.eclipseCaseFiles = eclipseCaseFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_EGRID_FILE ),
                                                                defaultDir );
    }
    if ( !eclipseInputFiles.empty() )
    {
        if ( !openInputEclipseCaseFromFileNames( eclipseInputFiles ) )
        {
            return OpenCaseResults();
        }
        results.eclipseInputFiles = eclipseInputFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_INPUT_FILE ),
                                                                defaultDir );
    }
    if ( !eclipseSummaryFiles.empty() )
    {
        if ( !openSummaryCaseFromFileNames( eclipseSummaryFiles, doCreateDefaultPlot ) )
        {
            return OpenCaseResults();
        }
        results.eclipseSummaryFiles = eclipseSummaryFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_SUMMARY_FILE ),
                                                                defaultDir );
    }
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportGeneralDataFeature::fileNamesFromCaseNames( const QStringList& caseNames )
{
    QStringList fileNames;
    {
        for ( const auto& caseName : caseNames )
        {
            if ( caseName.lastIndexOf( "." ) != -1 )
            {
                QFileInfo fi( caseName );
                fileNames.push_back( fi.absoluteFilePath() );
            }
            else
            {
                RiaEclipseFileNameTools nameTool( caseName );
                QString                 filenameWithExtension = nameTool.findRelatedGridFile();
                fileNames.push_back( filenameWithExtension );
            }
        }
    }

    return fileNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGeneralDataFeature::onActionTriggered( bool isChecked )
{
    openFileDialog( ImportFileType::ANY_ECLIPSE_FILE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGeneralDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Case24x24.png" ) );
    actionToSetup->setText( "Import Eclipse Files" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ImportFileType fileType )
{
    QString eclipseGridFilePattern( "*.GRID" );
    QString eclipseEGridFilePattern( "*.EGRID" );
    QString eclipseInputFilePattern( "*.GRDECL" );
    QString eclipseSummaryFilePattern( "*.SMSPEC" );

    QStringList filePatternTexts;
    if ( fileType == ImportFileType::ANY_ECLIPSE_FILE )
    {
        filePatternTexts += QString( "Eclipse Files (%1 %2 %3 %4)" )
                                .arg( eclipseGridFilePattern )
                                .arg( eclipseEGridFilePattern )
                                .arg( eclipseInputFilePattern )
                                .arg( eclipseSummaryFilePattern );
    }

    int fileTypeAsInt = int( fileType );

    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_EGRID_FILE ) )
    {
        filePatternTexts += QString( "Eclipse EGrid Files (%1)" ).arg( eclipseEGridFilePattern );
    }
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_GRID_FILE ) )
    {
        filePatternTexts += QString( "Eclipse Grid Files (%1)" ).arg( eclipseGridFilePattern );
    }
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_INPUT_FILE ) )
    {
        filePatternTexts += QString( "Eclipse Input Files and Input Properties (%1)" ).arg( eclipseInputFilePattern );
    }
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_SUMMARY_FILE ) )
    {
        filePatternTexts += QString( "Eclipse Summary File (%1)" ).arg( eclipseSummaryFilePattern );
    }

    QString fullPattern = filePatternTexts.join( ";;" );

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectory( defaultDirectoryLabel( fileType ) );

    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Data File",
                                                                  defaultDir,
                                                                  fullPattern );
    return fileNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGeneralDataFeature::openFileDialog( ImportFileType fileTypes )
{
    QStringList fileNames = getEclipseFileNamesWithDialog( fileTypes );
    if ( fileNames.empty() ) return;

    if ( fileTypes == ImportFileType::ANY_ECLIPSE_FILE )
    {
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ANY_ECLIPSE_FILE ),
                                                                fileNames.front() );
    }

    if ( !openEclipseFilesFromFileNames( fileNames, true ) )
    {
        RiaLogging::error( QString( "Failed to open file names: %1" ).arg( fileNames.join( ", " ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openEclipseCaseFromFileNames( const QStringList& fileNames )
{
    RiaImportEclipseCaseTools::FileCaseIdMap newCaseFiles;
    if ( RiaImportEclipseCaseTools::openEclipseCasesFromFile( fileNames, &newCaseFiles ) )
    {
        for ( const auto newCaseFileAndId : newCaseFiles )
        {
            RiaApplication::instance()->addToRecentFiles( newCaseFileAndId.first );
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openInputEclipseCaseFromFileNames( const QStringList& fileNames )
{
    QString fileContainingGrid;
    if ( RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames( fileNames, &fileContainingGrid ) )
    {
        RiaApplication::instance()->addToRecentFiles( fileContainingGrid );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openSummaryCaseFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot )
{
    std::vector<RimSummaryCase*> newCases;
    if ( RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles( fileNames, doCreateDefaultPlot, &newCases ) )
    {
        RicImportSummaryCasesFeature::addCasesToGroupIfRelevant( newCases );
        for ( const RimSummaryCase* newCase : newCases )
        {
            RiaApplication::instance()->addToRecentFiles( newCase->summaryHeaderFilename() );
        }
        return true;
    }
    return false;
}

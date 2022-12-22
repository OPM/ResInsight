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
    RicImportGeneralDataFeature::openEclipseFilesFromFileNames( const QStringList&                 fileNames,
                                                                bool                               doCreateDefaultPlot,
                                                                bool                               createDefaultView,
                                                                std::shared_ptr<RifReaderSettings> readerSettings )
{
    CVF_ASSERT( !fileNames.empty() );

    QString defaultDir = QFileInfo( fileNames.last() ).absolutePath();

    QStringList eclipseCaseFiles;
    QStringList eclipseInputFiles;
    QStringList eclipseSummaryFiles;
    QStringList roffFiles;

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
        else if ( fileTypeAsInt & int( ImportFileType::ROFF_FILE ) )
        {
            roffFiles.push_back( fileName );
        }
    }

    OpenCaseResults results;
    if ( !eclipseCaseFiles.empty() )
    {
        if ( !openEclipseCaseFromFileNames( eclipseCaseFiles, createDefaultView, results.createdCaseIds, readerSettings ) )
        {
            return OpenCaseResults();
        }
        results.eclipseCaseFiles = eclipseCaseFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_EGRID_FILE ),
                                                                defaultDir );
    }
    if ( !eclipseInputFiles.empty() )
    {
        if ( !openInputEclipseCaseFromFileNames( eclipseInputFiles, createDefaultView, results.createdCaseIds ) )
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
    if ( !roffFiles.empty() )
    {
        if ( !openRoffCaseFromFileNames( roffFiles, createDefaultView, results.createdCaseIds ) )
        {
            return OpenCaseResults();
        }
        results.roffFiles = roffFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ROFF_FILE ),
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
    actionToSetup->setIcon( QIcon( ":/Case.svg" ) );
    actionToSetup->setText( "Import Eclipse Files" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportGeneralDataFeature::getFilePattern( const std::vector<RiaDefines::ImportFileType>& fileTypes,
                                                     bool                                           allowWildcard )
{
    QStringList filePatternTexts;

    if ( allowWildcard )
    {
        filePatternTexts += "All Files (*.* *)";
    }

    for ( auto f : fileTypes )
    {
        filePatternTexts += getFilePattern( f );
    }

    return filePatternTexts.join( ";;" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportGeneralDataFeature::getFilePattern( RiaDefines::ImportFileType fileType )
{
    QString eclipseGridFilePattern( "*.GRID" );
    QString eclipseEGridFilePattern( "*.EGRID" );
    QString eclipseInputFilePattern( "*.GRDECL" );
    QString eclipseSummaryFilePattern( "*.SMSPEC" );
    QString roffFilePattern( "*.ROFF *.ROFFASC" );

    if ( fileType == ImportFileType::ANY_ECLIPSE_FILE )
    {
        return QString( "Eclipse Files (%1 %2 %3 %4)" )
            .arg( eclipseGridFilePattern )
            .arg( eclipseEGridFilePattern )
            .arg( eclipseInputFilePattern )
            .arg( eclipseSummaryFilePattern );
    }

    if ( fileType == ImportFileType::ECLIPSE_EGRID_FILE )
    {
        return QString( "Eclipse EGrid Files (%1)" ).arg( eclipseEGridFilePattern );
    }

    if ( fileType == ImportFileType::ECLIPSE_GRID_FILE )
    {
        return QString( "Eclipse Grid Files (%1)" ).arg( eclipseGridFilePattern );
    }

    if ( fileType == ImportFileType::ECLIPSE_INPUT_FILE )
    {
        return QString( "Eclipse Input Files and Input Properties (%1)" ).arg( eclipseInputFilePattern );
    }

    if ( fileType == ImportFileType::ECLIPSE_SUMMARY_FILE )
    {
        return QString( "Eclipse Summary File (%1)" ).arg( eclipseSummaryFilePattern );
    }

    if ( fileType == ImportFileType::ROFF_FILE )
    {
        return QString( "Roff File (%1)" ).arg( roffFilePattern );
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicImportGeneralDataFeature::getEclipseFileNamesWithDialog( RiaDefines::ImportFileType fileType )
{
    QStringList filePatternTexts;
    if ( fileType == ImportFileType::ANY_ECLIPSE_FILE )
    {
        filePatternTexts += getFilePattern( ImportFileType::ANY_ECLIPSE_FILE );
    }

    int fileTypeAsInt = int( fileType );
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_EGRID_FILE ) )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_EGRID_FILE );
    }
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_GRID_FILE ) )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_GRID_FILE );
    }
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_INPUT_FILE ) )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_INPUT_FILE );
    }
    if ( fileTypeAsInt & int( ImportFileType::ECLIPSE_SUMMARY_FILE ) )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_SUMMARY_FILE );
    }
    if ( fileTypeAsInt & int( ImportFileType::ROFF_FILE ) )
    {
        filePatternTexts += getFilePattern( ImportFileType::ROFF_FILE );
    }

    QString fullPattern = filePatternTexts.join( ";;" );

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectory( defaultDirectoryLabel( fileType ) );

    // Use nullptr as parent to this dialog, as this function is called from both plot window and main window
    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( nullptr, "Import Data File", defaultDir, fullPattern );

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

    if ( !openEclipseFilesFromFileNames( fileNames, true, true, nullptr ) )
    {
        RiaLogging::error( QString( "Failed to open file names: %1" ).arg( fileNames.join( ", " ) ) );
    }
    else
    {
        if ( fileNames.size() == 1 )
        {
            RiaApplication::instance()->addToRecentFiles( fileNames.front() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openEclipseCaseFromFileNames( const QStringList&                 fileNames,
                                                                bool                               createDefaultView,
                                                                std::vector<int>&                  createdCaseIds,
                                                                std::shared_ptr<RifReaderSettings> readerSettings )
{
    bool                                     noDialog = false;
    RiaImportEclipseCaseTools::FileCaseIdMap newCaseFiles;
    if ( RiaImportEclipseCaseTools::openEclipseCasesFromFile( fileNames, createDefaultView, &newCaseFiles, noDialog, readerSettings ) )
    {
        for ( const auto& newCaseFileAndId : newCaseFiles )
        {
            createdCaseIds.push_back( newCaseFileAndId.second );
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openInputEclipseCaseFromFileNames( const QStringList& fileNames,
                                                                     bool               createDefaultView,
                                                                     std::vector<int>&  createdCaseIds )
{
    auto generatedCaseId = RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames( fileNames, createDefaultView );
    if ( generatedCaseId >= 0 )
    {
        RiaApplication::instance()->addToRecentFiles( fileNames[0] );
        createdCaseIds.push_back( generatedCaseId );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openRoffCaseFromFileNames( const QStringList& fileNames,
                                                             bool               createDefaultView,
                                                             std::vector<int>&  createdCaseIds )
{
    CAF_ASSERT( !fileNames.empty() );

    auto generatedCaseId = RiaImportEclipseCaseTools::openRoffCaseFromFileNames( fileNames, createDefaultView );
    if ( generatedCaseId >= 0 )
    {
        RiaApplication::instance()->addToRecentFiles( fileNames[0] );
        createdCaseIds.push_back( generatedCaseId );
        return true;
    }
    return false;
}

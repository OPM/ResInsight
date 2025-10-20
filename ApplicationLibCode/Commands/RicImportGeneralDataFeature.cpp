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
#include "RiaPreferencesGrid.h"
#include "RiaPreferencesSummary.h"

#include "RicImportSummaryCasesFeature.h"

#include "RifEclipseInputFileTools.h"
#include "RifRoffFileTools.h"

#include "RimRoffCase.h"
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
    RicImportGeneralDataFeature::openEclipseFilesFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot, bool createDefaultView )
{
    RifReaderSettings rs = RiaPreferencesGrid::current()->readerSettings();
    return openEclipseFilesFromFileNames( fileNames, doCreateDefaultPlot, createDefaultView, rs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportGeneralDataFeature::OpenCaseResults RicImportGeneralDataFeature::openEclipseFilesFromFileNames( const QStringList& fileNames,
                                                                                                         bool doCreateDefaultPlot,
                                                                                                         bool createDefaultView,
                                                                                                         RifReaderSettings& readerSettings )
{
    CVF_ASSERT( !fileNames.empty() );

    QString defaultDir = QFileInfo( fileNames.last() ).absolutePath();

    QStringList eclipseCaseFiles;
    QStringList eclipseInputFiles;
    QStringList eclipseSummaryFiles;
    QStringList roffFiles;
    QStringList emFiles;

    for ( const QString& fileName : fileNames )
    {
        RiaDefines::ImportFileType fileType = obtainFileTypeFromFileName( fileName );
        if ( RiaDefines::isEclipseResultFileType( fileType ) )
        {
            eclipseCaseFiles.push_back( fileName );
        }
        else if ( fileType == ImportFileType::ECLIPSE_INPUT_FILE )
        {
            eclipseInputFiles.push_back( fileName );
        }
        else if ( fileType == ImportFileType::ECLIPSE_SUMMARY_FILE )
        {
            eclipseSummaryFiles.push_back( fileName );
        }
        else if ( fileType == ImportFileType::ROFF_FILE )
        {
            roffFiles.push_back( fileName );
        }
        else if ( fileType == ImportFileType::EM_H5GRID )
        {
            emFiles.push_back( fileName );
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
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_EGRID_FILE ), defaultDir );
    }
    if ( !eclipseInputFiles.empty() )
    {
        if ( !openEclipseInputFilesFromFileNames( eclipseInputFiles, createDefaultView, results.createdCaseIds ) )
        {
            return OpenCaseResults();
        }
        results.eclipseInputFiles = eclipseInputFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_INPUT_FILE ), defaultDir );
    }
    if ( !eclipseSummaryFiles.empty() )
    {
        if ( !openSummaryCaseFromFileNames( eclipseSummaryFiles, doCreateDefaultPlot ) )
        {
            return OpenCaseResults();
        }
        results.eclipseSummaryFiles = eclipseSummaryFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ECLIPSE_SUMMARY_FILE ), defaultDir );
    }
    if ( !roffFiles.empty() )
    {
        if ( !openRoffFilesFromFileNames( roffFiles, createDefaultView, results.createdCaseIds ) )
        {
            return OpenCaseResults();
        }
        results.roffFiles = roffFiles;
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ROFF_FILE ), defaultDir );
    }

    if ( !emFiles.empty() )
    {
        if ( !RiaImportEclipseCaseTools::openEmFilesFromFileNames( emFiles, createDefaultView, results.createdCaseIds ) )
        {
            return OpenCaseResults();
        }
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
QString RicImportGeneralDataFeature::getFilePattern( const std::vector<RiaDefines::ImportFileType>& fileTypes, bool allowWildcard )
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
    QString esmrySummaryFilePattern( "*.ESMRY" );
    QString roffFilePattern( "*.ROFF *.ROFFASC" );

    if ( fileType == ImportFileType::ANY_ECLIPSE_FILE )
    {
        return QString( "Eclipse Files (%1 %2 %3 %4)" )
            .arg( eclipseGridFilePattern )
            .arg( eclipseEGridFilePattern )
            .arg( eclipseInputFilePattern )
            .arg( eclipseSummaryFilePattern );
    }

    if ( fileType == ImportFileType::ECLIPSE_RESULT_GRID )
    {
        return QString( "Eclipse Result Grid File (%1 %2)" ).arg( eclipseGridFilePattern ).arg( eclipseEGridFilePattern );
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
        QStringList filterTexts;
        if ( RiaPreferencesSummary::current()->summaryDataReader() == RiaPreferencesSummary::SummaryReaderMode::OPM_COMMON )
        {
            filterTexts += QString( "ESMRY Summary Files (%1)" ).arg( esmrySummaryFilePattern );
            filterTexts += QString( "Eclipse Summary Files (%1)" ).arg( eclipseSummaryFilePattern );
        }
        else
        {
            filterTexts += QString( "Eclipse Summary Files (%1)" ).arg( eclipseSummaryFilePattern );
            filterTexts += QString( "ESMRY Summary Files (%1)" ).arg( esmrySummaryFilePattern );
        }

        filterTexts += QString( "All Summary Files (%1 %2)" ).arg( eclipseSummaryFilePattern ).arg( esmrySummaryFilePattern );

        return filterTexts.join( ";;" );
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
    if ( fileType == ImportFileType::ECLIPSE_RESULT_GRID )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_RESULT_GRID );
    }
    if ( fileType == ImportFileType::ECLIPSE_EGRID_FILE )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_EGRID_FILE );
    }
    if ( fileType == ImportFileType::ECLIPSE_GRID_FILE )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_GRID_FILE );
    }
    if ( fileType == ImportFileType::ECLIPSE_INPUT_FILE )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_INPUT_FILE );
    }
    if ( fileType == ImportFileType::ECLIPSE_SUMMARY_FILE )
    {
        filePatternTexts += getFilePattern( ImportFileType::ECLIPSE_SUMMARY_FILE );
    }
    if ( fileType == ImportFileType::ROFF_FILE )
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
        RiaApplication::instance()->setLastUsedDialogDirectory( defaultDirectoryLabel( ImportFileType::ANY_ECLIPSE_FILE ), fileNames.front() );
    }

    if ( !openEclipseFilesFromFileNames( fileNames, true, true ) )
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
bool RicImportGeneralDataFeature::openEclipseCaseFromFileNames( const QStringList& fileNames,
                                                                bool               createDefaultView,
                                                                std::vector<int>&  createdCaseIds,
                                                                RifReaderSettings& readerSettings )
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
bool RicImportGeneralDataFeature::openSummaryCaseFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot )
{
    auto [isOk, newCases] = RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles( fileNames, doCreateDefaultPlot );
    if ( isOk )
    {
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
bool RicImportGeneralDataFeature::openEclipseInputFilesFromFileNames( const QStringList& fileNames,
                                                                      bool               createDefaultView,
                                                                      std::vector<int>&  createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    // For single file - assume grid and perform open - to prevent multiple read of grid files
    if ( fileNames.size() == 1 )
    {
        return openGrdeclCaseAndPropertiesFromFileNames( fileNames, createDefaultView, createdCaseIds );
    }

    auto numGridCases =
        static_cast<int>( std::count_if( fileNames.begin(),
                                         fileNames.end(),
                                         []( const auto& fileName ) { return RifEclipseInputFileTools::hasGridData( fileName ); } ) );

    if ( numGridCases != fileNames.size() && numGridCases != 1 )
    {
        RiaLogging::error( "Select only grid case files or 1 grid case file with N property files" );
        return false;
    }

    if ( numGridCases == 1 )
    {
        // Open single grid case and connected property files
        return openGrdeclCaseAndPropertiesFromFileNames( fileNames, createDefaultView, createdCaseIds );
    }
    else
    {
        // Open multiple grid cases
        return openGrdeclCasesFromFileNames( fileNames, createDefaultView, createdCaseIds );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Assumes N files containing grid info - each file must be grid case
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openGrdeclCasesFromFileNames( const QStringList& fileNames,
                                                                bool               createDefaultView,
                                                                std::vector<int>&  createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    const size_t initialNumCases  = createdCaseIds.size();
    const auto   generatedCaseIds = RiaImportEclipseCaseTools::openEclipseInputCasesFromFileNames( fileNames, createDefaultView );

    if ( fileNames.size() == static_cast<int>( generatedCaseIds.size() ) )
    {
        RiaLogging::error( "Expected to create one eclipse case per file provided" );
    }

    for ( int i = 0; i < fileNames.size(); ++i )
    {
        const auto caseId = generatedCaseIds[static_cast<size_t>( i )];
        if ( caseId >= 0 )
        {
            RiaApplication::instance()->addToRecentFiles( fileNames[i] );
            createdCaseIds.push_back( caseId );
        }
    }

    return initialNumCases != createdCaseIds.size();
}

//--------------------------------------------------------------------------------------------------
/// Assuming 1 file with grid data and N files with property info for respective grid file
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openGrdeclCaseAndPropertiesFromFileNames( const QStringList& fileNames,
                                                                            bool               createDefaultView,
                                                                            std::vector<int>&  createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    const auto [caseFileName, generatedCaseId] =
        RiaImportEclipseCaseTools::openEclipseInputCaseAndPropertiesFromFileNames( fileNames, createDefaultView );
    if ( generatedCaseId >= 0 )
    {
        RiaApplication::instance()->addToRecentFiles( caseFileName );
        createdCaseIds.push_back( generatedCaseId );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openRoffFilesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    auto numGridCases = static_cast<int>(
        std::count_if( fileNames.begin(), fileNames.end(), []( const auto& fileName ) { return RifRoffFileTools::hasGridData( fileName ); } ) );

    if ( numGridCases != fileNames.size() && numGridCases != 1 )
    {
        RiaLogging::error( "Select only grid case files or 1 grid case file with N property files" );
        return false;
    }

    if ( numGridCases == 1 )
    {
        // Open single grid case and connected property files
        return openRoffCaseAndPropertiesFromFileNames( fileNames, createDefaultView, createdCaseIds );
    }
    else
    {
        // Open multiple grid cases
        return openRoffCasesFromFileNames( fileNames, createDefaultView, createdCaseIds );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Assumes N files containing grid info - each file must be grid case
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openRoffCasesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    const size_t initialNumCases  = createdCaseIds.size();
    auto         generatedCaseIds = RiaImportEclipseCaseTools::openRoffCasesFromFileNames( fileNames, createDefaultView );

    if ( fileNames.size() == static_cast<int>( generatedCaseIds.size() ) )
    {
        RiaLogging::error( "Expected to create one roff case per file provided" );
    }

    for ( int i = 0; i < fileNames.size(); ++i )
    {
        const auto caseId = generatedCaseIds[static_cast<size_t>( i )];
        if ( caseId >= 0 )
        {
            RiaApplication::instance()->addToRecentFiles( fileNames[i] );
            createdCaseIds.push_back( caseId );
        }
    }
    return initialNumCases != createdCaseIds.size();
}

//--------------------------------------------------------------------------------------------------
/// Assuming 1 roff file with grid data and N roff files with property info for respective grid file
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::openRoffCaseAndPropertiesFromFileNames( const QStringList& fileNames,
                                                                          bool               createDefaultView,
                                                                          std::vector<int>&  createdCaseIds )
{
    if ( fileNames.empty() ) return false;

    auto gridCaseItr =
        std::find_if( fileNames.begin(), fileNames.end(), []( const auto& fileName ) { return RifRoffFileTools::hasGridData( fileName ); } );

    if ( gridCaseItr == fileNames.end() )
    {
        RiaLogging::error( "Provided file names must contain one grid file" );
        return false;
    }

    auto* generatedRoffCase = RiaImportEclipseCaseTools::openRoffCaseFromFileName( *gridCaseItr, createDefaultView );
    if ( !generatedRoffCase ) return false;

    createdCaseIds.push_back( generatedRoffCase->caseId() );

    QStringList propertyFileNames;
    for ( auto fileNameItr = fileNames.begin(); fileNameItr != fileNames.end(); ++fileNameItr )
    {
        if ( fileNameItr == gridCaseItr ) continue;

        propertyFileNames.push_back( *fileNameItr );
    }
    generatedRoffCase->importAsciiInputProperties( propertyFileNames );

    return true;
}

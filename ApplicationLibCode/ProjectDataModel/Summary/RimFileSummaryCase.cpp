/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimFileSummaryCase.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesSummary.h"

#include "RifEclipseSummaryTools.h"
#include "RifMultipleSummaryReaders.h"
#include "RifOpmCommonSummary.h"
#include "RifProjectSummaryDataWriter.h"
#include "RifReaderEclipseSummary.h"
#include "RifReaderOpmRft.h"
#include "RifSummaryReaderMultipleFiles.h"

#include "RimCalculatedSummaryCurveReader.h"
#include "RimProject.h"
#include "RimRftCase.h"
#include "RimSummaryCalculationCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <QDir>
#include <QFileInfo>
#include <QUuid>

//==================================================================================================
//
//
//
//==================================================================================================
CAF_PDM_SOURCE_INIT( RimFileSummaryCase, "FileSummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileSummaryCase::RimFileSummaryCase()
{
    CAF_PDM_InitScriptableObject( "File Summary Case ", ":/SummaryCases16x16.png", "", "A Summary Case based on SMSPEC files" );
    CAF_PDM_InitScriptableField( &m_includeRestartFiles, "IncludeRestartFiles", false, "Include Restart Files" );
    m_includeRestartFiles.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_additionalSummaryFilePath, "AdditionalSummaryFilePath", "Additional File Path (set invisible when ready)" );
    m_additionalSummaryFilePath.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_rftCase, "RftCase", "RFT Data" );
    m_rftCase = new RimRftCase;

    m_hasParsedForWeseglink = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileSummaryCase::~RimFileSummaryCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFileSummaryCase::summaryHeaderFilename() const
{
    return m_summaryHeaderFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFileSummaryCase::caseName() const
{
    QFileInfo caseFileName( summaryHeaderFilename() );

    return caseFileName.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createSummaryReaderInterfaceThreadSafe( RiaThreadSafeLogger* threadSafeLogger )
{
    // RimFileSummaryCase::findRelatedFilesAndCreateReader is a performance bottleneck. The function
    // RifEclipseSummaryTools::getRestartFile() should be refactored to use opm-common instead of libecl.
    // It is not possible to use restart files in ESMRY file format, see see ESmry::make_esmry_file()
    //
    // https://github.com/OPM/ResInsight/issues/11342

    m_fileSummaryReader =
        RimFileSummaryCase::findRelatedFilesAndCreateReader( summaryHeaderFilename(), m_includeRestartFiles, threadSafeLogger );

    m_multiSummaryReader = new RifMultipleSummaryReaders;
    m_multiSummaryReader->addReader( m_fileSummaryReader.p() );

    openAndAttachAdditionalReader();

    RimSummaryCalculationCollection* calcColl = RimProject::current()->calculationCollection();
    m_calculatedSummaryReader                 = new RifCalculatedSummaryCurveReader( calcColl, this );

    m_multiSummaryReader->addReader( m_calculatedSummaryReader.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createSummaryReaderInterface()
{
    RiaThreadSafeLogger threadSafeLogger;
    createSummaryReaderInterfaceThreadSafe( &threadSafeLogger );

    auto messages = threadSafeLogger.messages();
    for ( const auto& m : messages )
    {
        RiaLogging::info( m );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createRftReaderInterface()
{
    QFileInfo fileInfo( summaryHeaderFilename() );
    QString   folder = fileInfo.absolutePath();

    QString rftFileName = folder + "/" + fileInfo.completeBaseName() + ".RFT";
    {
        QFileInfo fi( rftFileName );

        if ( fi.exists() )
        {
            m_rftCase()->setRftFileName( rftFileName );
        }
    }

    // Usually, the data deck file path is empty at this point. If the user has specified the path manually, we use that.
    // The intention is to use searchForWseglinkAndRecreateRftReader() to search for the data deck file path. Here we avoid searching for
    // the data deck file by default, as this is a time consuming operation.
    m_summaryEclipseRftReader = RimFileSummaryCase::createOpmRftReader( rftFileName, m_rftCase->dataDeckFilePath() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimFileSummaryCase::findRelatedFilesAndCreateReader( const QString&       headerFileName,
                                                                                bool                 lookForRestartFiles,
                                                                                RiaThreadSafeLogger* threadSafeLogger )
{
    if ( lookForRestartFiles )
    {
        std::vector<QString>            warnings;
        std::vector<RifRestartFileInfo> restartFileInfos = RifEclipseSummaryTools::getRestartFiles( headerFileName, warnings );

        if ( !restartFileInfos.empty() )
        {
            std::vector<std::string> summaryFileNames;
            summaryFileNames.push_back( headerFileName.toStdString() );
            for ( const auto& s : restartFileInfos )
            {
                summaryFileNames.push_back( s.fileName.toStdString() );
            }

            // The ordering in intended to be start of history first, so we reverse the ordering
            std::reverse( summaryFileNames.begin(), summaryFileNames.end() );

            auto summaryReader = new RifSummaryReaderMultipleFiles( summaryFileNames );
            if ( !summaryReader->createReadersAndImportMetaData( threadSafeLogger ) )
            {
                delete summaryReader;
                return nullptr;
            }

            return summaryReader;
        }
    }

    RifReaderEclipseSummary* summaryFileReader = new RifReaderEclipseSummary;

    // All restart data is taken care of by RifSummaryReaderMultipleFiles, never read restart data from native file
    // readers
    if ( !summaryFileReader->open( headerFileName, threadSafeLogger ) )
    {
        delete summaryFileReader;
        return nullptr;
    }

    return summaryFileReader;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmRft* RimFileSummaryCase::createOpmRftReader( const QString& rftFileName, const QString& dataDeckFileName )
{
    QFileInfo fi( rftFileName );

    if ( fi.exists() )
    {
        return new RifReaderOpmRft( rftFileName, dataDeckFileName );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_additionalSummaryFilePath )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    RimSummaryCase::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    if ( rftReader() )
    {
        uiTreeOrdering.add( m_rftCase() );
    }
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::openAndAttachAdditionalReader()
{
    QString additionalSummaryFilePath = m_additionalSummaryFilePath().path();
    if ( additionalSummaryFilePath.isEmpty() ) return;

    cvf::ref<RifOpmCommonEclipseSummary> opmCommonReader = new RifOpmCommonEclipseSummary;
    opmCommonReader->useEnhancedSummaryFiles( true );

    bool includeRestartFiles = false;
    auto isValid             = opmCommonReader->open( additionalSummaryFilePath, includeRestartFiles, nullptr );
    if ( isValid )
    {
        m_multiSummaryReader->addReader( opmCommonReader.p() );
        m_additionalSummaryFileReader = opmCommonReader;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimFileSummaryCase::summaryReader()
{
    if ( m_multiSummaryReader.isNull() )
    {
        createSummaryReaderInterface();
    }
    return m_multiSummaryReader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimFileSummaryCase::rftReader()
{
    if ( m_summaryEclipseRftReader.isNull() )
    {
        createRftReaderInterface();
    }
    return m_summaryEclipseRftReader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::setIncludeRestartFiles( bool includeRestartFiles )
{
    m_includeRestartFiles = includeRestartFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::setSummaryData( const std::string& keyword, const std::string& unit, const std::vector<float>& values )
{
    size_t mainSummaryFileValueCount = m_fileSummaryReader->timeSteps( RifEclipseSummaryAddress() ).size();
    if ( values.size() != mainSummaryFileValueCount )
    {
        QString txt = QString( "Wrong size of summary data for keyword %1. Expected %2 values, received %3 values" )
                          .arg( QString::fromStdString( keyword ) )
                          .arg( mainSummaryFileValueCount )
                          .arg( values.size() );
        RiaLogging::error( txt );

        return;
    }

    // Remove existing reader to be able to write to the summary file
    m_multiSummaryReader->removeReader( m_additionalSummaryFileReader.p() );
    m_additionalSummaryFileReader = nullptr;

    RifProjectSummaryDataWriter projectSummaryDataWriter;

    QString   tmpAdditionalSummaryFilePath = m_additionalSummaryFilePath().path();
    QFileInfo fi( tmpAdditionalSummaryFilePath );
    if ( fi.exists() )
    {
        projectSummaryDataWriter.importFromProjectSummaryFile( tmpAdditionalSummaryFilePath.toStdString() );
    }
    else
    {
        projectSummaryDataWriter.importFromSourceSummaryReader( m_fileSummaryReader.p() );

        auto tempFilePath = additionalSummaryDataFilePath();

        m_additionalSummaryFilePath  = tempFilePath;
        tmpAdditionalSummaryFilePath = tempFilePath;
    }

    projectSummaryDataWriter.setData( { keyword }, { unit }, { values } );

    std::string outputFilePath = tmpAdditionalSummaryFilePath.toStdString();
    projectSummaryDataWriter.writeDataToFile( outputFilePath );

    for ( const auto& txt : projectSummaryDataWriter.errorMessages() )
    {
        RiaLogging::error( QString::fromStdString( txt ) );
    }
    projectSummaryDataWriter.clearErrorMessages();

    openAndAttachAdditionalReader();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::onProjectBeingSaved()
{
    // If additional data is stored in temp folder, copy to project folder and remove file in temp folder

    auto existingFilePath = m_additionalSummaryFilePath().path();
    if ( QFile::exists( existingFilePath ) )
    {
        auto currentFilePath = additionalSummaryDataFilePath();
        if ( existingFilePath != currentFilePath )
        {
            if ( QFile::copy( existingFilePath, currentFilePath ) )
            {
                QFile::remove( existingFilePath );
                m_additionalSummaryFilePath = currentFilePath;

                openAndAttachAdditionalReader();
            }
            else
            {
                QString txt = "Error when copying temporary file to " + currentFilePath;
                RiaLogging::error( txt );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFileSummaryCase::createAdditionalSummaryFileName()
{
    QUuid   uuid       = QUuid::createUuid();
    QString uuidString = uuid.toString();
    uuidString.remove( '{' ).remove( '}' );

    auto filePath = "RI_SUMMARY_DATA_" + uuidString + ".ESMRY";

    return filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::searchForWseglinkAndRecreateRftReader()
{
    if ( m_hasParsedForWeseglink ) return;

    if ( m_rftCase->dataDeckFilePath().isEmpty() )
    {
        QFileInfo fileInfo( summaryHeaderFilename() );
        QString   folder = fileInfo.absolutePath();

        // Search for *.DATA file in same folder as summary file. If not found, search for a schedule file.
        QString validDataDeckFileName;

        QString   dataDeckFileName = folder + "/" + fileInfo.completeBaseName() + ".DATA";
        QFileInfo fi( dataDeckFileName );

        if ( fi.exists() )
        {
            validDataDeckFileName = dataDeckFileName;
        }
        else
        {
            QString   scheduleFileName = folder + "/" + fileInfo.completeBaseName() + ".SCH";
            QFileInfo fi( scheduleFileName );

            if ( fi.exists() )
            {
                validDataDeckFileName = scheduleFileName;
            }
        }

        if ( !validDataDeckFileName.isEmpty() )
        {
            m_rftCase->setDataDeckFileName( dataDeckFileName );

            // Create a new reader including the file path to the data deck file
            // NB! This can be a time consuming operation
            m_summaryEclipseRftReader = RimFileSummaryCase::createOpmRftReader( m_rftCase->rftFilePath(), m_rftCase->dataDeckFilePath() );
        }
    }

    m_hasParsedForWeseglink = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFileSummaryCase::additionalSummaryDataFilePath() const
{
    QDir            storageDir;
    RiaApplication* app = RiaApplication::instance();
    if ( app->isProjectSavedToDisc() )
    {
        QString projectFileName = RimProject::current()->fileName();

        QFileInfo fileInfo( projectFileName );
        storageDir = fileInfo.dir();
    }
    else
    {
        // If project is not saved, use the temp folder
        storageDir = QDir::temp();
    }

    QString fileName;
    auto    cacheSummaryFilePath = m_additionalSummaryFilePath().path();
    if ( cacheSummaryFilePath.isEmpty() )
    {
        fileName = createAdditionalSummaryFileName();
    }
    else
    {
        QFileInfo fi( cacheSummaryFilePath );
        fileName = fi.fileName();
    }

    auto filePath = storageDir.absoluteFilePath( fileName );

    return filePath;
}

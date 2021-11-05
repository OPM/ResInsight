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

#include "RiaLogging.h"

#include "RicfCommandObject.h"
#include "RifEclipseSummaryTools.h"
#include "RifReaderEclipseRft.h"
#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderMultipleFiles.h"

#include "RimTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

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
    QFileInfo caseFileName( this->summaryHeaderFilename() );

    return caseFileName.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    //  m_summaryHeaderFilename =
    //      RimTools::relocateFile( m_summaryHeaderFilename().path(), newProjectPath, oldProjectPath, nullptr, nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createSummaryReaderInterfaceThreadSafe( RiaThreadSafeLogger* threadSafeLogger )
{
    m_summaryFileReader = RimFileSummaryCase::findRelatedFilesAndCreateReader( this->summaryHeaderFilename(),
                                                                               m_includeRestartFiles,
                                                                               threadSafeLogger );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createSummaryReaderInterface()
{
    RiaThreadSafeLogger threadSafeLogger;
    m_summaryFileReader = RimFileSummaryCase::findRelatedFilesAndCreateReader( this->summaryHeaderFilename(),
                                                                               m_includeRestartFiles,
                                                                               &threadSafeLogger );

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
    m_summaryEclipseRftReader = RimFileSummaryCase::findRftDataAndCreateReader( this->summaryHeaderFilename() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimFileSummaryCase::findRelatedFilesAndCreateReader( const QString& headerFileName,
                                                                                bool           includeRestartFiles,
                                                                                RiaThreadSafeLogger* threadSafeLogger )
{
    if ( includeRestartFiles )
    {
        std::vector<QString>            warnings;
        std::vector<RifRestartFileInfo> restartFileInfos =
            RifEclipseSummaryTools::getRestartFiles( headerFileName, warnings );

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
RifReaderEclipseRft* RimFileSummaryCase::findRftDataAndCreateReader( const QString& headerFileName )
{
    QFileInfo fileInfo( headerFileName );
    QString   folder = fileInfo.absolutePath();

    QString   rftFileName = folder + "/" + fileInfo.completeBaseName() + ".RFT";
    QFileInfo rftFileInfo( rftFileName );

    if ( rftFileInfo.exists() )
    {
        std::unique_ptr<RifReaderEclipseRft> rftReader( new RifReaderEclipseRft( rftFileInfo.filePath() ) );
        return rftReader.release();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimFileSummaryCase::summaryReader()
{
    if ( m_summaryFileReader.isNull() )
    {
        createSummaryReaderInterface();
    }
    return m_summaryFileReader.p();
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

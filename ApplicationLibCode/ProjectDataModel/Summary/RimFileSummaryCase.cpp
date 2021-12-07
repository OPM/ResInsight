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
#include "RifOpmCommonSummary.h"
#include "RifReaderEclipseRft.h"
#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderMultipleFiles.h"

#include "RimTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiPushButtonEditor.h"

#include "RiaPreferencesSummary.h"
#include "RifMultipleSummaryReaders.h"
#include "cafPdmUiFilePathEditor.h"
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/EclOutput.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"
#include <QDir>
#include <QFileInfo>

#pragma optimize( "", on )

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

    CAF_PDM_InitFieldNoDefault( &m_additionalSummaryFilePath, "AdditionalSummaryFilePath", "Additional File Path" );
    CAF_PDM_InitFieldNoDefault( &m_appendDataToAdditionalSummaryFile, "AppendDataToAdditionalSummaryFile", "Append Data" );
    m_appendDataToAdditionalSummaryFile.uiCapability()->setUiEditorTypeName(
        caf::PdmUiPushButtonEditor::uiEditorTypeName() );
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
    m_fileSummaryReader = RimFileSummaryCase::findRelatedFilesAndCreateReader( this->summaryHeaderFilename(),
                                                                               m_includeRestartFiles,
                                                                               threadSafeLogger );

    m_multiSummaryReader = new RifMultipleSummaryReaders;
    m_multiSummaryReader->addReader( m_fileSummaryReader.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::createSummaryReaderInterface()
{
    RiaThreadSafeLogger threadSafeLogger;
    m_fileSummaryReader  = RimFileSummaryCase::findRelatedFilesAndCreateReader( this->summaryHeaderFilename(),
                                                                               m_includeRestartFiles,
                                                                               &threadSafeLogger );
    m_multiSummaryReader = new RifMultipleSummaryReaders;
    m_multiSummaryReader->addReader( m_fileSummaryReader.p() );

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
    std::vector<RifSummaryReaderInterface*> summaryReaders;

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
void RimFileSummaryCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    if ( changedField == &m_appendDataToAdditionalSummaryFile )
    {
        m_appendDataToAdditionalSummaryFile = false;

        appendData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_additionalSummaryFilePath )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::appendData()
{
    RifEclipseSummaryAddress adr = RifEclipseSummaryAddress::miscAddress( "MSJ_TEST" );

    size_t valueCount = m_fileSummaryReader->timeSteps( RifEclipseSummaryAddress() ).size();

    std::vector<double> values;

    for ( size_t i = 0; i < valueCount; i++ )
    {
        values.push_back( i );
    }

    setSummaryData( adr, values );
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
void RimFileSummaryCase::setSummaryData( const RifEclipseSummaryAddress& address, const std::vector<double>& values )
{
    // ensure esmry file is created
    // append data to esmry
    // reload esmry file
    // update meta data

    m_multiSummaryReader->removeReader( m_additionalSummaryFileReader.p() );
    m_additionalSummaryFileReader = nullptr;

    if ( m_additionalSummaryFileReader.isNull() )
    {
        cvf::ref<RifOpmCommonEclipseSummary> opmCommonReader = new RifOpmCommonEclipseSummary;

        opmCommonReader->useEnhancedSummaryFiles( true );

        QString headerFileName = m_additionalSummaryFilePath().path();

        auto isValid = opmCommonReader->open( headerFileName, false, nullptr );

        if ( !isValid )
        {
            auto              sourceFileName = this->summaryHeaderFilename().toStdString();
            Opm::EclIO::ESmry sourceSummaryData( sourceFileName );

            {
                // Create smry file with one summary vector
                {
                    std::string outputFileName = m_additionalSummaryFilePath().path().toStdString();

                    // The ExtESmry reader supports only binary mode, set formatted to false
                    bool                  formatted = false;
                    Opm::EclIO::EclOutput outFile( outputFileName, formatted, std::ios::out );

                    Opm::TimeStampUTC ts( std::chrono::system_clock::to_time_t( sourceSummaryData.startdate() ) );

                    std::vector<int> start_date_vect =
                        { ts.day(), ts.month(), ts.year(), ts.hour(), ts.minutes(), ts.seconds(), 0 };

                    outFile.write<int>( "START", start_date_vect );

                    std::vector<std::string> keywords = sourceSummaryData.keywordList();
                    std::vector<std::string> unitTexts;
                    for ( const auto& key : keywords )
                    {
                        const auto& unit = sourceSummaryData.get_unit( key );
                        unitTexts.push_back( unit );
                    }

                    outFile.write( "KEYCHECK", keywords );
                    outFile.write( "UNITS", unitTexts );

                    size_t timeStepCount = 0;
                    {
                        auto tstepValues = sourceSummaryData.get( "TIME" );
                        timeStepCount    = tstepValues.size();
                    }

                    {
                        // Bool array 1 means RSTEP, 0 means no RSTEP
                        // Dummy values, but not relevant for our use
                        // Required by the reader
                        std::vector<int> intValues( timeStepCount, 1 );
                        outFile.write<int>( "RSTEP", intValues );
                    }

                    {
                        // TSTEP represents time steps
                        // Dummy values, but not relevant for our use
                        // Required by the reader
                        std::vector<int> intValues;
                        intValues.resize( timeStepCount );
                        std::iota( intValues.begin(), intValues.end(), 0 );
                        outFile.write<int>( "TSTEP", intValues );
                    }

                    for ( size_t n = 0; n < static_cast<size_t>( keywords.size() ); n++ )
                    {
                        std::string vect_name = "V" + std::to_string( n );
                        const auto& values    = sourceSummaryData.get( keywords[n] );
                        outFile.write<float>( vect_name, values );
                    }
                }
            }

            if ( isValid )
            {
                m_additionalSummaryFileReader = opmCommonReader;
                m_multiSummaryReader->addReader( m_additionalSummaryFileReader.p() );
            }
        }
    }
}

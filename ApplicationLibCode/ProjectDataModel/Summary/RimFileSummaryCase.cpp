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
#include "RifProjectSummaryDataWriter.h"
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

    openAndAttachAdditionalReader();
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

    openAndAttachAdditionalReader();

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

    std::vector<float> values;

    for ( size_t i = 0; i < valueCount; i++ )
    {
        values.push_back( i );
    }

    setSummaryData( adr, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileSummaryCase::openAndAttachAdditionalReader()
{
    QString additionalSummaryFilePath = m_additionalSummaryFilePath().path();

    cvf::ref<RifOpmCommonEclipseSummary> opmCommonReader = new RifOpmCommonEclipseSummary;
    opmCommonReader->useEnhancedSummaryFiles( true );

    bool includeRestartFiles = false;
    auto isValid             = opmCommonReader->open( additionalSummaryFilePath, includeRestartFiles, nullptr );
    if ( isValid )
    {
        m_multiSummaryReader->addReader( opmCommonReader.p() );
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
void RimFileSummaryCase::setSummaryData( const RifEclipseSummaryAddress& address, const std::vector<float>& values )
{
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
        projectSummaryDataWriter.importFromSourceSummaryFile( summaryHeaderFilename().toStdString() );

        QUuid   uuid       = QUuid::createUuid();
        QString uuidString = uuid.toString();
        uuidString.remove( '{' ).remove( '}' );

        QString projectSummaryDataPath = "RI_SUMMARY_DATA_" + uuidString + ".ESMRY";

        auto tempDir      = QDir::temp();
        auto tempFilePath = tempDir.absoluteFilePath( projectSummaryDataPath );

        m_additionalSummaryFilePath  = tempFilePath;
        tmpAdditionalSummaryFilePath = tempFilePath;
    }

    auto keyword = address.uiText();

    projectSummaryDataWriter.setData( { keyword }, { "" }, { values } );

    std::string outputFilePath = tmpAdditionalSummaryFilePath.toStdString();
    projectSummaryDataWriter.writeDataToFile( outputFilePath );

    openAndAttachAdditionalReader();
}

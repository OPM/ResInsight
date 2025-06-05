/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimObservedEclipseUserData.h"

#include "RiaLogging.h"

#include "RifColumnBasedUserData.h"
#include "RifColumnBasedUserDataParser.h"
#include "RifKeywordVectorParser.h"
#include "RifKeywordVectorUserData.h"
#include "RifMultipleSummaryReaders.h"
#include "RifSummaryReaderInterface.h"

#include "RimCalculatedSummaryCurveReader.h"

#include "cafUtils.h"

#include <QFile>

CAF_PDM_SOURCE_INIT( RimObservedEclipseUserData, "RimObservedEclipseUserData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedEclipseUserData::RimObservedEclipseUserData()
{
    CAF_PDM_InitObject( "Observed RSMSPEC Column Based Data File", ":/ObservedRSMDataFile16x16.png" );
    m_summaryHeaderFilename.uiCapability()->setUiName( "File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedEclipseUserData::~RimObservedEclipseUserData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedEclipseUserData::createSummaryReaderInterface()
{
    m_multiSummaryReader = nullptr;
    m_summaryReader      = nullptr;

    if ( caf::Utils::fileExists( summaryHeaderFilename() ) )
    {
        QFile file( summaryHeaderFilename() );
        if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            RiaLogging::error( QString( "Failed to open %1" ).arg( summaryHeaderFilename() ) );

            return;
        }

        QTextStream in( &file );
        QString     fileContents = in.readAll();
        fileContents.replace( "\t", " " );

        if ( RifKeywordVectorParser::canBeParsed( fileContents ) )
        {
            RifKeywordVectorUserData* keywordVectorUserData = new RifKeywordVectorUserData();
            if ( keywordVectorUserData->parse( fileContents, customWellName() ) )
            {
                m_summaryReader = keywordVectorUserData;
            }
        }
        else
        {
            RifColumnBasedUserData* columnBaseUserData = new RifColumnBasedUserData();
            if ( columnBaseUserData->parse( fileContents, &m_errorText ) )
            {
                m_summaryReader = columnBaseUserData;
            }
        }

        if ( m_summaryReader.notNull() )
        {
            m_multiSummaryReader = new RifMultipleSummaryReaders;
            m_multiSummaryReader->addReader( m_summaryReader.p() );

            m_calculatedSummaryReader = new RifCalculatedSummaryCurveReader( this );

            m_multiSummaryReader->addReader( m_calculatedSummaryReader.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimObservedEclipseUserData::summaryReader()
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
QString RimObservedEclipseUserData::errorMessagesFromReader()
{
    return m_errorText;
}

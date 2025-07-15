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

#include "RimCsvUserData.h"

#include "RiaLogging.h"

#include "RifColumnBasedUserDataParser.h"
#include "RifCsvUserData.h"
#include "RifKeywordVectorUserData.h"
#include "RifMultipleSummaryReaders.h"
#include "RifSummaryReaderInterface.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "RimCalculatedSummaryCurveReader.h"

#include "cafUtils.h"

#include <QFile>

CAF_PDM_SOURCE_INIT( RimCsvUserData, "RimCsvUserData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCsvUserData::RimCsvUserData()
{
    CAF_PDM_InitObject( "Observed CSV Data File", ":/ObservedCSVDataFile16x16.png" );
    m_summaryHeaderFilename.uiCapability()->setUiName( "File" );

    CAF_PDM_InitFieldNoDefault( &m_parseOptions, "ParseOptions", "" );
    m_parseOptions = new RicPasteAsciiDataToSummaryPlotFeatureUi();
    m_parseOptions.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCsvUserData::~RimCsvUserData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCsvUserData::createSummaryReaderInterface()
{
    m_multiSummaryReader = nullptr;

    if ( caf::Utils::fileExists( summaryHeaderFilename() ) )
    {
        RifCsvUserData* csvUserData = new RifCsvUserData();
        if ( csvUserData->parse( summaryHeaderFilename(), m_parseOptions->parseOptions(), &m_errorText ) )
        {
            m_summaryReader = csvUserData;

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
RifSummaryReaderInterface* RimCsvUserData::summaryReader()
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
QString RimCsvUserData::errorMessagesFromReader()
{
    return m_errorText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPasteAsciiDataToSummaryPlotFeatureUi* RimCsvUserData::parseOptions() const
{
    return m_parseOptions();
}

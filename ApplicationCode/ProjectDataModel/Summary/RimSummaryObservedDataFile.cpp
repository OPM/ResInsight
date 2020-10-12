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

#include "RimSummaryObservedDataFile.h"

#include "RifReaderObservedData.h"

#include "RimTools.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimSummaryObservedDataFile, "SummaryObservedDataFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryObservedDataFile::RimSummaryObservedDataFile()
{
    CAF_PDM_InitObject( "Observed data file", ":/ObservedDataFile16x16.png", "", "" );
    m_summaryHeaderFilename.uiCapability()->setUiName( "File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryObservedDataFile::~RimSummaryObservedDataFile()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryObservedDataFile::createSummaryReaderInterface()
{
    m_summaryReader = new RifReaderObservedData;

    if ( !m_summaryReader->open( this->summaryHeaderFilename(), identifierName(), summaryCategory() ) )
    {
        m_summaryReader = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryObservedDataFile::summaryReader()
{
    if ( m_summaryReader.isNull() )
    {
        createSummaryReaderInterface();
    }
    return m_summaryReader.p();
}

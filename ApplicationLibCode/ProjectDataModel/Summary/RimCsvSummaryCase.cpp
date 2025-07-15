/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimCsvSummaryCase.h"

#include "RiaLogging.h"

#include "RifRevealCsvSummaryReader.h"
#include "RifStimPlanCsvSummaryReader.h"
#include "RifSummaryReaderInterface.h"

#include "RimTools.h"

#include "cafPdmUiTextEditor.h"
#include "cafUtils.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimCsvSummaryCase, "CsvSummaryCase" );

namespace caf
{
template <>
void caf::AppEnum<RimCsvSummaryCase::FileType>::setUp()
{
    addItem( RimCsvSummaryCase::FileType::REVEAL, "REVEAL", "Reveal" );
    addItem( RimCsvSummaryCase::FileType::STIMPLAN, "STIMPLAN", "StimPlan" );

    setDefault( RimCsvSummaryCase::FileType::REVEAL );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCsvSummaryCase::RimCsvSummaryCase()
{
    CAF_PDM_InitFieldNoDefault( &m_fileType, "FileType", "File Type" );
    CAF_PDM_InitField( &m_startDate, "StartDate", QDateTime::currentDateTime(), "Start Date" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCsvSummaryCase::caseName() const
{
    QFileInfo caseFileName( summaryHeaderFilename() );
    return caseFileName.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCsvSummaryCase::createSummaryReaderInterface()
{
    m_summaryReader = nullptr;

    if ( caf::Utils::fileExists( summaryHeaderFilename() ) )
    {
        if ( m_fileType == FileType::REVEAL )
        {
            auto    reader = std::make_unique<RifRevealCsvSummaryReader>();
            QString errorMessage;
            if ( auto [ok, caseName] = reader->parse( summaryHeaderFilename(), &errorMessage ); ok )
            {
                m_summaryReader = std::move( reader );
                m_displayName   = caseName;
            }
            else
            {
                RiaLogging::error( "Failed to read Reveal summary file" );
                RiaLogging::error( errorMessage );
            }
        }
        else if ( m_fileType == FileType::STIMPLAN )
        {
            auto    reader = std::make_unique<RifStimPlanCsvSummaryReader>();
            QString errorMessage;
            if ( auto [ok, caseName] = reader->parse( summaryHeaderFilename(), m_startDate, &errorMessage ); ok )
            {
                m_summaryReader = std::move( reader );
                m_displayName   = caseName;
            }
            else
            {
                RiaLogging::error( "Failed to read StimPlan summary file" );
                RiaLogging::error( errorMessage );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimCsvSummaryCase::summaryReader()
{
    if ( !m_summaryReader )
    {
        createSummaryReaderInterface();
    }
    return m_summaryReader.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCsvSummaryCase::setFileType( FileType fileType )
{
    m_fileType = fileType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCsvSummaryCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimSummaryCase::defineUiOrdering( uiConfigName, uiOrdering );

    if ( m_fileType == FileType::STIMPLAN ) uiOrdering.add( &m_startDate );
}

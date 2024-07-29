/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimDataSourceForRftPlt.h"
#include "RimEclipseCase.h"
#include "RimObservedFmuRftData.h"
#include "RimPressureDepthData.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimWellLogLasFile.h"

#include "cafAppEnum.h"
#include "cvfAssert.h"

#include <QString>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_SOURCE_INIT( RimDataSourceForRftPlt, "RftAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSourceForRftPlt::RimDataSourceForRftPlt()
{
    initPdmObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSourceForRftPlt::RimDataSourceForRftPlt( const RifDataSourceForRftPlt& addr )
{
    initPdmObject();
    setAddress( addr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSourceForRftPlt::setAddress( const RifDataSourceForRftPlt& address )
{
    m_sourceType         = address.sourceType();
    m_eclCase            = address.eclCase();
    m_summaryCase        = address.summaryCase();
    m_ensemble           = address.ensemble();
    m_wellLogFile        = address.wellLogFile();
    m_observedFmuRftData = address.observedFmuRftData();
    m_pressureDepthData  = address.pressureDepthData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt RimDataSourceForRftPlt::address() const
{
    switch ( m_sourceType() )
    {
        case RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE:
            return RifDataSourceForRftPlt( m_wellLogFile );
        case RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA:
            return RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA, m_eclCase );
        case RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA:
            return RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, m_eclCase );
        case RifDataSourceForRftPlt::SourceType::SUMMARY_RFT:
            return RifDataSourceForRftPlt( m_summaryCase, m_ensemble, m_eclCase );
        case RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT:
            return RifDataSourceForRftPlt( m_ensemble );
        case RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT:
            return RifDataSourceForRftPlt( m_observedFmuRftData );
        case RifDataSourceForRftPlt::SourceType::OBSERVED_PRESSURE_DEPTH:
            return RifDataSourceForRftPlt( m_pressureDepthData );
    }
    return RifDataSourceForRftPlt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSourceForRftPlt::initPdmObject()
{
    CAF_PDM_InitFieldNoDefault( &m_sourceType, "SourceType", "Source Type" );
    CAF_PDM_InitFieldNoDefault( &m_eclCase, "EclipseCase", "Eclipse Case" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Summary Case" );
    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    CAF_PDM_InitFieldNoDefault( &m_wellLogFile, "WellLogFile", "Well Log File" );
    CAF_PDM_InitFieldNoDefault( &m_observedFmuRftData, "ObservedFmuRftData", "Observed FMU Data" );
    CAF_PDM_InitFieldNoDefault( &m_pressureDepthData, "PressureDepthData", "Pressure/Depth Data" );
}

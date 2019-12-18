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
#include "RimSummaryCaseCollection.h"
#include "RimWellLogFile.h"

#include "cafAppEnum.h"
#include "cvfAssert.h"
#include <QString>
#include <QTextStream>

namespace caf
{
template <>
void caf::AppEnum<RifDataSourceForRftPlt::SourceType>::setUp()
{
    addItem( RifDataSourceForRftPlt::SourceType::RFT, "RFT", "RFT Cases" );
    addItem( RifDataSourceForRftPlt::SourceType::GRID, "GRID", "Grid Cases" );
    addItem( RifDataSourceForRftPlt::SourceType::OBSERVED, "OBSERVED", "Observed Data" );
    addItem( RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT, "ENSEMBLE", "Ensembles with RFT Data" );
    addItem( RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT, "OBSERVED_FMU", "Observed FMU Data" );
    setDefault( RifDataSourceForRftPlt::SourceType::NONE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_PDM_SOURCE_INIT( RimDataSourceForRftPlt, "RftAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSourceForRftPlt::RimDataSourceForRftPlt()
{
    InitPdmObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSourceForRftPlt::RimDataSourceForRftPlt( const RifDataSourceForRftPlt& addr )
{
    InitPdmObject();
    setAddress( addr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSourceForRftPlt::setAddress( const RifDataSourceForRftPlt& address )
{
    m_sourceType         = address.sourceType();
    m_eclCase            = address.eclCase();
    m_wellLogFile        = address.wellLogFile();
    m_ensemble           = address.ensemble();
    m_observedFmuRftData = address.observedFmuRftData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt RimDataSourceForRftPlt::address() const
{
    switch ( m_sourceType() )
    {
        case RifDataSourceForRftPlt::OBSERVED:
            return RifDataSourceForRftPlt( m_sourceType(), m_wellLogFile );
        case RifDataSourceForRftPlt::RFT:
        case RifDataSourceForRftPlt::GRID:
            return RifDataSourceForRftPlt( m_sourceType(), m_eclCase );
        case RifDataSourceForRftPlt::ENSEMBLE_RFT:
            return RifDataSourceForRftPlt( m_sourceType(), m_ensemble );
        case RifDataSourceForRftPlt::OBSERVED_FMU_RFT:
            return RifDataSourceForRftPlt( m_sourceType(), m_observedFmuRftData );
        default:
            break;
    }
    return RifDataSourceForRftPlt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSourceForRftPlt::InitPdmObject()
{
    CAF_PDM_InitFieldNoDefault( &m_sourceType, "SourceType", "Source Type", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_eclCase, "EclipseCase", "Eclipse Case", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellLogFile, "WellLogFile", "Well Log File", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_observedFmuRftData, "ObservedFmuRftData", "Observed FMU Data", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataSourceForRftPlt& RimDataSourceForRftPlt::operator=( const RimDataSourceForRftPlt& other )
{
    m_sourceType         = other.m_sourceType();
    m_eclCase            = other.m_eclCase();
    m_wellLogFile        = other.m_wellLogFile();
    m_ensemble           = other.m_ensemble();
    m_observedFmuRftData = other.m_observedFmuRftData();
    return *this;
}

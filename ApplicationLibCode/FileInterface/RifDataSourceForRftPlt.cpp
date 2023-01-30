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

#include "RifDataSourceForRftPlt.h"

#include "RifReaderEclipseRft.h"
#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimObservedFmuRftData.h"
#include "RimPressureDepthData.h"
#include "RimSummaryCase.h"
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
    addItem( RifDataSourceForRftPlt::SourceType::NONE, "NONE", "None" );
    addItem( RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE, "OBSERVED", "Observed Data" );
    addItem( RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA, "RFT", "RFT Data" );
    addItem( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, "GRID", "Grid Cases" );
    addItem( RifDataSourceForRftPlt::SourceType::SUMMARY_RFT, "SUMMARY_RFT", "Summary Data" );
    addItem( RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT, "ENSEMBLE", "Ensembles with RFT Data" );
    addItem( RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT, "OBSERVED_FMU", "Observed FMU Data" );
    addItem( RifDataSourceForRftPlt::SourceType::OBSERVED_PRESSURE_DEPTH,
             "OBSERVED_PRESSURE_DEPTH",
             "Observed Pressure/Depth Data" );
    setDefault( RifDataSourceForRftPlt::SourceType::NONE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt()
    : m_sourceType( SourceType::NONE )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( SourceType sourceType, RimEclipseCase* eclCase )
{
    CVF_ASSERT( sourceType == SourceType::RFT_SIM_WELL_DATA || sourceType == SourceType::GRID_MODEL_CELL_DATA );
    CVF_ASSERT( eclCase != nullptr );

    m_sourceType = sourceType;
    m_eclCase    = eclCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( SourceType sourceType, RimWellLogFile* wellLogFile )
{
    CVF_ASSERT( sourceType == SourceType::OBSERVED_LAS_FILE );
    m_sourceType  = SourceType::OBSERVED_LAS_FILE;
    m_wellLogFile = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimSummaryCaseCollection* ensemble )
{
    m_sourceType = SourceType::ENSEMBLE_RFT;
    m_ensemble   = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimSummaryCase* summaryCase, RimSummaryCaseCollection* ensemble )
{
    m_sourceType  = SourceType::SUMMARY_RFT;
    m_summaryCase = summaryCase;
    m_ensemble    = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimObservedFmuRftData* observedFmuRftData )
{
    m_sourceType         = SourceType::OBSERVED_FMU_RFT;
    m_observedFmuRftData = observedFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimPressureDepthData* observedFmuRftData )
{
    m_sourceType        = SourceType::OBSERVED_FMU_RFT;
    m_pressureDepthData = observedFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::SourceType RifDataSourceForRftPlt::sourceType() const
{
    return m_sourceType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RifDataSourceForRftPlt::eclCase() const
{
    return m_eclCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaDefines::EclipseUnitSystem> RifDataSourceForRftPlt::availableUnitSystems() const
{
    std::vector<RiaDefines::EclipseUnitSystem> systems;

    if ( m_eclCase && m_eclCase->eclipseCaseData() )
    {
        systems.push_back( m_eclCase->eclipseCaseData()->unitsType() );
    }

    if ( m_wellLogFile && m_wellLogFile->wellLogFileData() )
    {
        auto eclipseUnit = RiaDefines::fromDepthUnit( m_wellLogFile->wellLogFileData()->depthUnit() );
        systems.push_back( eclipseUnit );
    }

    return systems;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RifDataSourceForRftPlt::rftReader() const
{
    if ( m_sourceType == SourceType::GRID_MODEL_CELL_DATA || m_sourceType == SourceType::RFT_SIM_WELL_DATA )
    {
        // TODO: Consider changing to RimEclipseResultCase to avoid casting
        auto eclResCase = dynamic_cast<RimEclipseResultCase*>( eclCase() );

        if ( eclResCase ) return eclResCase->rftReader();
    }
    else if ( m_sourceType == SourceType::SUMMARY_RFT )
    {
        if ( m_summaryCase ) return m_summaryCase->rftReader();
    }
    else if ( m_sourceType == SourceType::ENSEMBLE_RFT )
    {
        if ( m_ensemble ) return m_ensemble->rftStatisticsReader();
    }
    else if ( m_sourceType == SourceType::OBSERVED_FMU_RFT )
    {
        if ( m_observedFmuRftData ) return m_observedFmuRftData->rftReader();
    }
    else if ( m_sourceType == SourceType::OBSERVED_PRESSURE_DEPTH )
    {
        if ( m_pressureDepthData ) return m_pressureDepthData->rftReader();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RifDataSourceForRftPlt::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RifDataSourceForRftPlt::summaryCase() const
{
    return m_summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFile* RifDataSourceForRftPlt::wellLogFile() const
{
    return m_wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedFmuRftData* RifDataSourceForRftPlt::observedFmuRftData() const
{
    return m_observedFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureDepthData* RifDataSourceForRftPlt::pressureDepthData() const
{
    return m_pressureDepthData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifDataSourceForRftPlt::sourceTypeUiText( SourceType sourceType )
{
    switch ( sourceType )
    {
        case SourceType::RFT_SIM_WELL_DATA:
            return QString( "RFT File Cases" );
        case SourceType::GRID_MODEL_CELL_DATA:
            return QString( "Grid Cases" );
        case SourceType::OBSERVED_LAS_FILE:
            return QString( "Observed Data" );
        case SourceType::ENSEMBLE_RFT:
            return QString( "Ensembles with RFT Data" );
        case SourceType::SUMMARY_RFT:
            return QString( "Summary case with RFT Data" );
        case SourceType::OBSERVED_FMU_RFT:
            return QString( "Observed FMU data" );
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator==( const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2 )
{
    return addr1.sourceType() == addr2.sourceType() && addr1.eclCase() == addr2.eclCase() &&
           addr1.wellLogFile() == addr2.wellLogFile() && addr1.summaryCase() == addr2.summaryCase() &&
           addr1.ensemble() == addr2.ensemble() && addr1.observedFmuRftData() == addr2.observedFmuRftData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator<<( QTextStream& str, const RifDataSourceForRftPlt& addr )
{
    // Not implemented
    CVF_ASSERT( false );
    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator>>( QTextStream& str, RifDataSourceForRftPlt& source )
{
    // Not implemented
    CVF_ASSERT( false );
    return str;
}

//--------------------------------------------------------------------------------------------------
/// This sort order controls the plot order in PLT plot. (Layer-wise)
/// Observed data is supposed to be the bottom layers (first)
//--------------------------------------------------------------------------------------------------
bool operator<( const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2 )
{
    if ( addr1.m_sourceType != addr2.m_sourceType )
    {
        return addr1.m_sourceType < addr2.m_sourceType;
    }

    if ( addr1.m_sourceType == RifDataSourceForRftPlt::NONE ) return false; //

    if ( addr1.m_sourceType == RifDataSourceForRftPlt::OBSERVED_LAS_FILE )
    {
        if ( addr1.wellLogFile() && addr2.wellLogFile() )
        {
            return addr1.wellLogFile()->fileName() < addr2.wellLogFile()->fileName();
        }
        return addr1.wellLogFile() < addr2.wellLogFile();
    }
    else if ( addr1.m_sourceType == RifDataSourceForRftPlt::SUMMARY_RFT )
    {
        if ( addr1.summaryCase() && addr2.summaryCase() )
        {
            if ( addr1.summaryCase()->displayCaseName() == addr2.summaryCase()->displayCaseName() )
            {
                if ( addr1.ensemble() && addr2.ensemble() )
                {
                    return addr1.ensemble()->name() < addr2.ensemble()->name();
                }
                return addr1.ensemble() < addr2.ensemble();
            }
            return addr1.summaryCase()->displayCaseName() < addr2.summaryCase()->displayCaseName();
        }
        return addr1.summaryCase() < addr2.summaryCase();
    }
    else if ( addr1.m_sourceType == RifDataSourceForRftPlt::ENSEMBLE_RFT )
    {
        if ( addr1.ensemble() && addr2.ensemble() )
        {
            return addr1.ensemble()->name() < addr2.ensemble()->name();
        }
        return addr1.ensemble() < addr2.ensemble();
    }
    else if ( addr1.m_sourceType == RifDataSourceForRftPlt::OBSERVED_FMU_RFT )
    {
        if ( addr1.observedFmuRftData() && addr2.observedFmuRftData() )
        {
            return addr1.observedFmuRftData()->name() < addr2.observedFmuRftData()->name();
        }
        return addr1.observedFmuRftData() < addr2.observedFmuRftData();
    }
    else
    {
        if ( addr1.eclCase() && addr2.eclCase() )
        {
            return addr1.eclCase()->caseId() < addr2.eclCase()->caseId();
        }
        return addr1.eclCase() < addr2.eclCase();
    }
}
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator<(const RifWellRftAddress& addr1, const RifWellRftAddress& addr2)
{
    return (addr1.m_sourceType < addr2.m_sourceType) ||
        (addr1.m_sourceType == addr2.m_sourceType && 
         addr1.eclCase() != nullptr && addr2.eclCase() != nullptr ? addr1.eclCase()->caseId() < addr2.eclCase()->caseId() :
         addr1.wellLogFile() != nullptr && addr2.wellLogFile() != nullptr ?  addr1.wellLogFile()->fileName() < addr2.wellLogFile()->fileName() :
         addr1.wellLogFile() < addr2.wellLogFile());
}
#endif

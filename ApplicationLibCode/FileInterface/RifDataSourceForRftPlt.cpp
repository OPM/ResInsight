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

#include "RigEclipseCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimObservedFmuRftData.h"
#include "RimPressureDepthData.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimWellLogLasFile.h"

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
    addItem( RifDataSourceForRftPlt::SourceType::OBSERVED_PRESSURE_DEPTH, "OBSERVED_PRESSURE_DEPTH", "Observed Pressure/Depth Data" );
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
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimWellLogLasFile* wellLogFile )
{
    m_sourceType  = SourceType::OBSERVED_LAS_FILE;
    m_wellLogFile = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimSummaryEnsemble* ensemble )
{
    m_sourceType = SourceType::ENSEMBLE_RFT;
    m_ensemble   = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( RimSummaryCase* summaryCase, RimSummaryEnsemble* ensemble, RimEclipseCase* eclipseCase )
{
    m_sourceType  = SourceType::SUMMARY_RFT;
    m_summaryCase = summaryCase;
    m_ensemble    = ensemble;
    m_eclCase     = eclipseCase;
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

    if ( m_wellLogFile && m_wellLogFile->wellLogData() )
    {
        auto eclipseUnit = RiaDefines::fromDepthUnit( m_wellLogFile->wellLogData()->depthUnit() );
        systems.push_back( eclipseUnit );
    }

    return systems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto RifDataSourceForRftPlt::operator<=>( const RifDataSourceForRftPlt& addr2 ) const -> std::strong_ordering
{
    if ( m_sourceType != addr2.m_sourceType )
    {
        return m_sourceType <=> addr2.m_sourceType;
    }

    if ( m_sourceType == RifDataSourceForRftPlt::SourceType::NONE ) return std::strong_ordering::less;

    if ( m_sourceType == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
    {
        if ( wellLogFile() && addr2.wellLogFile() )
        {
            return wellLogFile()->fileName().toStdString() <=> addr2.wellLogFile()->fileName().toStdString();
        }
        return wellLogFile() <=> addr2.wellLogFile();
    }
    else if ( m_sourceType == RifDataSourceForRftPlt::SourceType::SUMMARY_RFT )
    {
        if ( summaryCase() && addr2.summaryCase() )
        {
            if ( summaryCase()->displayCaseName() == addr2.summaryCase()->displayCaseName() )
            {
                if ( ensemble() && addr2.ensemble() )
                {
                    return ensemble()->name().toStdString() <=> addr2.ensemble()->name().toStdString();
                }
                return ensemble() <=> addr2.ensemble();
            }
            return summaryCase()->displayCaseName().toStdString() <=> addr2.summaryCase()->displayCaseName().toStdString();
        }
        return summaryCase() <=> addr2.summaryCase();
    }
    else if ( m_sourceType == RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT )
    {
        if ( ensemble() && addr2.ensemble() )
        {
            return ensemble()->name().toStdString() <=> addr2.ensemble()->name().toStdString();
        }
        return ensemble() <=> addr2.ensemble();
    }
    else if ( m_sourceType == RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT )
    {
        if ( observedFmuRftData() || addr2.observedFmuRftData() )
        {
            if ( observedFmuRftData() && addr2.observedFmuRftData() )
            {
                return observedFmuRftData()->name().toStdString() <=> addr2.observedFmuRftData()->name().toStdString();
            }
            return observedFmuRftData() <=> addr2.observedFmuRftData();
        }
        if ( pressureDepthData() || addr2.pressureDepthData() )
        {
            if ( pressureDepthData() && addr2.pressureDepthData() )
            {
                return pressureDepthData()->name().toStdString() <=> addr2.pressureDepthData()->name().toStdString();
            }
            return pressureDepthData() <=> addr2.pressureDepthData();
        }
        return std::strong_ordering::less;
    }
    else
    {
        if ( eclCase() && addr2.eclCase() )
        {
            return eclCase()->caseId() <=> addr2.eclCase()->caseId();
        }
        return eclCase() <=> addr2.eclCase();
    }

    return this <=> &addr2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RifDataSourceForRftPlt::ensemble() const
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
RimWellLogLasFile* RifDataSourceForRftPlt::wellLogFile() const
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

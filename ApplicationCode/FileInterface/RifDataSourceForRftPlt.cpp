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
#include "RimEclipseCase.h"
#include "RimObservedFmuRftData.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimWellLogFile.h"

#include "RimEclipseResultCase.h"
#include "cafAppEnum.h"
#include "cvfAssert.h"
#include <QString>
#include <QTextStream>

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
    CVF_ASSERT( sourceType == SourceType::RFT || sourceType == SourceType::GRID );
    CVF_ASSERT( eclCase != nullptr );

    m_sourceType = sourceType;
    m_eclCase    = eclCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( SourceType sourceType, RimWellLogFile* wellLogFile )
{
    CVF_ASSERT( sourceType == SourceType::OBSERVED );

    m_sourceType  = sourceType;
    m_wellLogFile = wellLogFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( SourceType sourceType, RimSummaryCaseCollection* ensemble )
{
    CVF_ASSERT( sourceType == SourceType::ENSEMBLE_RFT );

    m_sourceType = sourceType;
    m_ensemble   = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( SourceType                sourceType,
                                                RimSummaryCase*           summaryCase,
                                                RimSummaryCaseCollection* ensemble )
{
    CVF_ASSERT( sourceType == SourceType::SUMMARY_RFT );

    m_sourceType  = sourceType;
    m_summaryCase = summaryCase;
    m_ensemble    = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt::RifDataSourceForRftPlt( SourceType sourceType, RimObservedFmuRftData* observedFmuRftData )
{
    CVF_ASSERT( sourceType == SourceType::OBSERVED_FMU_RFT );

    m_sourceType         = sourceType;
    m_observedFmuRftData = observedFmuRftData;
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
RifReaderRftInterface* RifDataSourceForRftPlt::rftReader() const
{
    if ( m_sourceType == RFT )
    {
        auto eclResCase = dynamic_cast<RimEclipseResultCase*>( m_eclCase.p() );

        if ( eclResCase ) return eclResCase->rftReader();
    }
    else if ( m_sourceType == SUMMARY_RFT )
    {
        if ( m_summaryCase ) return m_summaryCase->rftReader();
    }
    else if ( m_sourceType == ENSEMBLE_RFT )
    {
        if ( m_ensemble ) return m_ensemble->rftStatisticsReader();
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
QString RifDataSourceForRftPlt::sourceTypeUiText( SourceType sourceType )
{
    switch ( sourceType )
    {
        case SourceType::RFT:
            return QString( "RFT File Cases" );
        case SourceType::GRID:
            return QString( "Grid Cases" );
        case SourceType::OBSERVED:
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

    if ( addr1.m_sourceType == RifDataSourceForRftPlt::OBSERVED )
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

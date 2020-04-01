/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimcSummaryCase.h"

#include "RiaQDateTimeTools.h"
#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

#include "RimcDataContainerDouble.h"
#include "RimcDataContainerString.h"
#include "RimcDataContainerTime.h"
#include "RimcSummaryResampleData.h"

#include "cafPdmFieldIOScriptability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimcSummaryCase_summaryVectorValues, "SummaryVectorValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryCase_summaryVectorValues::RimcSummaryCase_summaryVectorValues( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create Summary Plot", "", "", "Create a new Summary Plot" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_addressString,
                                                "Address",
                                                "",
                                                "",
                                                "",
                                                "Formatted address specifying the summary vector" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSummaryCase_summaryVectorValues::execute()
{
    QStringList addressStrings = m_addressString().split( ";", QString::SkipEmptyParts );

    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( m_addressString().toStdString() );

    std::vector<double> values;
    bool                isOk = sumReader->values( adr, &values );

    if ( isOk )
    {
        auto dataObject            = new RimcDataContainerDouble();
        dataObject->m_doubleValues = values;

        return dataObject;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSummaryCase_summaryVectorValues::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSummaryCase_summaryVectorValues::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerDouble );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimcSummaryCase_AvailableAddresses, "AvailableAddresses" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryCase_AvailableAddresses::RimcSummaryCase_AvailableAddresses( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Available Addresses", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSummaryCase_AvailableAddresses::execute()
{
    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    const std::set<RifEclipseSummaryAddress>& addresses = sumReader->allResultAddresses();

    std::vector<QString> adr;
    for ( const auto& a : addresses )
    {
        adr.push_back( QString::fromStdString( a.uiText() ) );
    }

    auto dataObject            = new RimcDataContainerString();
    dataObject->m_stringValues = adr;

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSummaryCase_AvailableAddresses::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSummaryCase_AvailableAddresses::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerString );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimcSummaryCase_TimeSteps, "AvailableTimeSteps" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryCase_TimeSteps::RimcSummaryCase_TimeSteps( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Available TimeSteps", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSummaryCase_TimeSteps::execute()
{
    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    RifEclipseSummaryAddress adr;
    auto                     timeValues = sumReader->timeSteps( adr );

    auto dataObject          = new RimcDataContainerTime();
    dataObject->m_timeValues = timeValues;

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSummaryCase_TimeSteps::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSummaryCase_TimeSteps::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerTime );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimcSummaryCase_ResampleValues, "ResampleValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSummaryCase_ResampleValues::RimcSummaryCase_ResampleValues( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Resample Values", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_addressString,
                                                "Address",
                                                "",
                                                "",
                                                "",
                                                "Formatted address specifying the summary vector" );

    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_resamplingPeriod, "ResamplingPeriod", "", "", "", "Resampling Period" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSummaryCase_ResampleValues::execute()
{
    QStringList addressStrings = m_addressString().split( ";", QString::SkipEmptyParts );

    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    auto adr = RifEclipseSummaryAddress::fromEclipseTextAddress( m_addressString().toStdString() );

    std::vector<double> values;
    bool                isOk = sumReader->values( adr, &values );
    if ( !isOk ) return nullptr;

    auto           timeValues = sumReader->timeSteps( adr );
    DateTimePeriod period     = DateTimePeriod::NONE;

    {
        QString periodString = m_resamplingPeriod().trimmed();

        if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_DAY_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::DAY;
        }
        else if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_WEEK_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::WEEK;
        }
        else if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_MONTH_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::MONTH;
        }
        else if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_QUARTER_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::QUARTER;
        }
        else if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_HALFYEAR_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::HALFYEAR;
        }
        else if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_YEAR_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::YEAR;
        }
        else if ( periodString.compare( RiaQDateTimeTools::TIMESPAN_DECADE_NAME, Qt::CaseInsensitive ) == 0 )
        {
            period = DateTimePeriod::DECADE;
        }
    }

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData( values, timeValues );

    if ( RiaSummaryTools::hasAccumulatedData( adr ) )
    {
        resampler.resampleAndComputePeriodEndValues( period );
    }
    else
    {
        resampler.resampleAndComputeWeightedMeanValues( period );
    }

    auto dataObject            = new RimcSummaryResampleData();
    dataObject->m_timeValues   = resampler.resampledTimeSteps();
    dataObject->m_doubleValues = resampler.resampledValues();

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSummaryCase_ResampleValues::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSummaryCase_ResampleValues::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcSummaryResampleData );
}

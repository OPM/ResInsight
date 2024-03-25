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

#include "RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimFileSummaryCase.h"
#include "RimSummaryCase.h"

#include "RimcDataContainerDouble.h"
#include "RimcDataContainerString.h"
#include "RimcDataContainerTime.h"
#include "RimcSummaryResampleData.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimSummaryCase_summaryVectorValues, "summaryVectorValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase_summaryVectorValues::RimSummaryCase_summaryVectorValues( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Summary Vector Values", "", "", "Get all values for a summary vector" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_addressString, "Address", "", "", "", "Formatted address specifying the summary vector" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimSummaryCase_summaryVectorValues::execute()
{
    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    auto adr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( m_addressString().toStdString() );

    if ( sumReader )
    {
        auto [isOk, values] = sumReader->values( adr );
        if ( isOk )
        {
            return RimcDataContainerDouble::create( values );
        }
    }

    return RimcDataContainerDouble::create( {} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase_summaryVectorValues::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimSummaryCase_summaryVectorValues::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerDouble );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimSummaryCase_availableAddresses, "availableAddresses" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase_availableAddresses::RimSummaryCase_availableAddresses( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Available Addresses" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimSummaryCase_availableAddresses::execute()
{
    auto* summaryCase = self<RimSummaryCase>();

    std::vector<QString> adr;

    RifSummaryReaderInterface* sumReader = summaryCase->summaryReader();
    if ( sumReader )
    {
        const std::set<RifEclipseSummaryAddress>& addresses = sumReader->allResultAddresses();
        for ( const auto& a : addresses )
        {
            adr.push_back( QString::fromStdString( a.uiText() ) );
        }
    }

    auto dataObject            = new RimcDataContainerString();
    dataObject->m_stringValues = adr;

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase_availableAddresses::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimSummaryCase_availableAddresses::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerString );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimSummaryCase_availableTimeSteps, "availableTimeSteps" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase_availableTimeSteps::RimSummaryCase_availableTimeSteps( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Available TimeSteps" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimSummaryCase_availableTimeSteps::execute()
{
    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    auto dataObject = new RimcDataContainerTime();
    if ( sumReader )
    {
        RifEclipseSummaryAddress adr;
        dataObject->m_timeValues = sumReader->timeSteps( adr );
    }

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase_availableTimeSteps::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimSummaryCase_availableTimeSteps::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerTime );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimSummaryCase_resampleValues, "resampleValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase_resampleValues::RimSummaryCase_resampleValues( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Resample Values" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_addressString, "Address", "", "", "", "Formatted address specifying the summary vector" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resamplingPeriod, "ResamplingPeriod", "", "", "", "Resampling Period" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimSummaryCase_resampleValues::execute()
{
    auto*                      summaryCase = self<RimSummaryCase>();
    RifSummaryReaderInterface* sumReader   = summaryCase->summaryReader();

    auto adr = RifEclipseSummaryAddress::fromEclipseTextAddressParseErrorTokens( m_addressString().toStdString() );

    auto dataObject = new RimcSummaryResampleData();

    if ( sumReader )
    {
        auto [isOk, values] = sumReader->values( adr );
        if ( !isOk )
        {
            // Error message
        }

        const auto& timeValues = sumReader->timeSteps( adr );

        QString                    periodString = m_resamplingPeriod().trimmed();
        RiaDefines::DateTimePeriod period       = RiaDefines::DateTimePeriodEnum::fromText( periodString );

        if ( period != RiaDefines::DateTimePeriod::NONE )
        {
            auto [resampledTimeSteps, resampledValues] = RiaSummaryTools::resampledValuesForPeriod( adr, timeValues, values, period );

            dataObject->m_timeValues   = resampledTimeSteps;
            dataObject->m_doubleValues = resampledValues;
        }
        else
        {
            dataObject->m_timeValues   = timeValues;
            dataObject->m_doubleValues = values;
        }
    }

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase_resampleValues::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimSummaryCase_resampleValues::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcSummaryResampleData );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSummaryCase, RimSummaryCase_setSummaryVectorValues, "setSummaryValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase_setSummaryVectorValues::RimSummaryCase_setSummaryVectorValues( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Summary Values" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_addressString, "Address", "", "", "", "Formatted address specifying the summary vector" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_unitString, "Unit", "", "", "", "Unit" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_values, "Values", "", "", "", "Values" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimSummaryCase_setSummaryVectorValues::execute()
{
    auto* summaryCase     = self<RimSummaryCase>();
    auto* fileSummaryCase = dynamic_cast<RimFileSummaryCase*>( summaryCase );
    if ( fileSummaryCase )
    {
        bool rebuildUserInterface = true;

        if ( auto reader = fileSummaryCase->summaryReader() )
        {
            auto allAddr = reader->allResultAddresses();
            for ( auto adr : allAddr )
            {
                if ( adr.uiText() == m_addressString().toStdString() )
                {
                    rebuildUserInterface = false;
                    break;
                }
            }
        }

        fileSummaryCase->setSummaryData( m_addressString().toStdString(), m_unitString().toStdString(), m_values() );

        if ( rebuildUserInterface )
        {
            fileSummaryCase->refreshMetaData();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase_setSummaryVectorValues::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimSummaryCase_setSummaryVectorValues::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCase_setSummaryVectorValues::isNullptrValidResult() const
{
    return true;
}

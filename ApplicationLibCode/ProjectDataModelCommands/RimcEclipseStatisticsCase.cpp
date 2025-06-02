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

#include "RimcEclipseStatisticsCase.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicImportSummaryCasesFeature.h"

#include "RimEclipseStatisticsCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseStatisticsCase, RimcEclipseStatisticsCase_setSourceProperties, "set_source_properties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseStatisticsCase_setSourceProperties::RimcEclipseStatisticsCase_setSourceProperties( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Define Source Properties" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyNames, "PropertyNames", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseStatisticsCase_setSourceProperties::execute()
{
    RiaDefines::ResultCatType myEnum = caf::AppEnum<RiaDefines::ResultCatType>::fromText( m_propertyType() );

    if ( myEnum == RiaDefines::ResultCatType::DYNAMIC_NATIVE || myEnum == RiaDefines::ResultCatType::STATIC_NATIVE ||
         myEnum == RiaDefines::ResultCatType::INPUT_PROPERTY || myEnum == RiaDefines::ResultCatType::GENERATED )
    {
        auto eclipseCase = self<RimEclipseStatisticsCase>();
        eclipseCase->setSourceProperties( myEnum, m_propertyNames );
        return nullptr;
    }
    else
    {
        return std::unexpected( "Wrong result type. Supported types are DYNAMIC_NATIVE, STATIC_NATIVE, INPUT_PROPERTY, GENERATED" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_setSourceProperties::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcEclipseStatisticsCase_setSourceProperties::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_setSourceProperties::isNullptrValidResult_obsolete() const
{
    return true;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseStatisticsCase, RimcEclipseStatisticsCase_computeStatistics, "compute_statistics" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseStatisticsCase_computeStatistics::RimcEclipseStatisticsCase_computeStatistics( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseStatisticsCase_computeStatistics::execute()
{
    auto eclipseCase = self<RimEclipseStatisticsCase>();
    eclipseCase->computeStatistics();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_computeStatistics::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcEclipseStatisticsCase_computeStatistics::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_computeStatistics::isNullptrValidResult_obsolete() const
{
    return true;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseStatisticsCase, RimcEclipseStatisticsCase_clearSourceProperties, "clear_source_properties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseStatisticsCase_clearSourceProperties::RimcEclipseStatisticsCase_clearSourceProperties( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseStatisticsCase_clearSourceProperties::execute()
{
    auto myEnums = { RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                     RiaDefines::ResultCatType::STATIC_NATIVE,
                     RiaDefines::ResultCatType::INPUT_PROPERTY,
                     RiaDefines::ResultCatType::GENERATED };

    auto eclipseCase = self<RimEclipseStatisticsCase>();

    for ( auto myEnum : myEnums )
    {
        eclipseCase->setSourceProperties( myEnum, {} );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_clearSourceProperties::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcEclipseStatisticsCase_clearSourceProperties::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_clearSourceProperties::isNullptrValidResult_obsolete() const
{
    return true;
}

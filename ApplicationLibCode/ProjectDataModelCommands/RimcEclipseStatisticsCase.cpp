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

#include "RimcEclipseStatisticsCase.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"

#include "RicImportSummaryCasesFeature.h"

/*
#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSurfaceCollection.h"
#include "RiuPlotMainWindow.h"
*/

#include "RiaLogging.h"
#include "RimEclipseStatisticsCase.h"
#include "cafPdmFieldScriptingCapability.h"

/*
#include <QDir>
#include <QFileInfo>

#include <memory>
*/

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseStatisticsCase, RimcEclipseStatisticsCase_defineSourceProperties, "set_source_result" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseStatisticsCase_defineSourceProperties::RimcEclipseStatisticsCase_defineSourceProperties( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Define Source Properties" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyType, "PropertyType", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_propertyNames, "PropertyNames", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcEclipseStatisticsCase_defineSourceProperties::execute()
{
    RiaDefines::ResultCatType myEnum = caf::AppEnum<RiaDefines::ResultCatType>::fromText( m_propertyType() );

    if ( myEnum == RiaDefines::ResultCatType::DYNAMIC_NATIVE || myEnum == RiaDefines::ResultCatType::STATIC_NATIVE ||
         myEnum == RiaDefines::ResultCatType::INPUT_PROPERTY || myEnum == RiaDefines::ResultCatType::GENERATED )
    {
        auto eclipseCase = self<RimEclipseStatisticsCase>();
        eclipseCase->setSourceProperties( myEnum, m_propertyNames );
    }
    else
    {
        RiaLogging::error( "Wrong result type. Supported types are DYNAMIC_NATIVE, STATIC_NATIVE, INPUT_PROPERTY, GENERATED" );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_defineSourceProperties::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcEclipseStatisticsCase_defineSourceProperties::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_defineSourceProperties::isNullptrValidResult() const
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
caf::PdmObjectHandle* RimcEclipseStatisticsCase_computeStatistics::execute()
{
    auto eclipseCase = self<RimEclipseStatisticsCase>();
    eclipseCase->computeStatistics();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseStatisticsCase_computeStatistics::resultIsPersistent() const
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
bool RimcEclipseStatisticsCase_computeStatistics::isNullptrValidResult() const
{
    return true;
}

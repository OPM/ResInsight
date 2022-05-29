/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RimEclipseResultAddress.h"

#include "RimEclipseCase.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT( RimEclipseResultAddress, "EclipseResultAddress" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultAddress::RimEclipseResultAddress()
{
    CAF_PDM_InitObject( "EclipseResultAddress", ":/DataVector.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_resultName, "ResultName", "Result Name" );
    m_resultName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_resultType, "ResultType", "Type" );
    m_resultType.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );
    m_eclipseCase.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultAddress::~RimEclipseResultAddress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultAddress::resultName() const
{
    return m_resultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultAddress::setResultName( const QString& resultName )
{
    m_resultName = resultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultAddress::setResultType( RiaDefines::ResultCatType val )
{
    m_resultType = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ResultCatType RimEclipseResultAddress::resultType() const
{
    return m_resultType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultAddress::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseResultAddress::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseResultAddress::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType( proj );
        if ( proj )
        {
            std::vector<RimEclipseCase*> cases;
            proj->descendantsIncludingThisOfType( cases );
            for ( auto* c : cases )
            {
                options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
            }
        }
    }

    return options;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimCaseCollection.h"

#include "RimEclipseCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimReservoirGridEnsemble.h"

CAF_PDM_SOURCE_INIT( RimCaseCollection, "RimCaseCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseCollection::RimCaseCollection()
{
    CAF_PDM_InitObject( "Derived Statistics" );

    CAF_PDM_InitFieldNoDefault( &reservoirs, "Reservoirs", "Reservoirs ChildArrayField" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCase*> RimCaseCollection::cases() const
{
    std::vector<RimCase*> caseVector;

    for ( auto rimCase : reservoirs )
    {
        caseVector.push_back( rimCase );
    }

    return caseVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimCaseCollection::parentCaseGroup()
{
    return dynamic_cast<RimIdenticalGridCaseGroup*>( parentField()->ownerObject() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsemble* RimCaseCollection::parentGridEnsemble()
{
    return dynamic_cast<RimReservoirGridEnsemble*>( parentField()->ownerObject() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsembleBase* RimCaseCollection::parentGridEnsembleBase()
{
    auto* owner = parentField()->ownerObject();

    if ( auto* caseGroup = dynamic_cast<RimIdenticalGridCaseGroup*>( owner ) )
    {
        return caseGroup;
    }
    return dynamic_cast<RimReservoirGridEnsemble*>( owner );
}

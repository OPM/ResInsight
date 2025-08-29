/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimSummaryEnsembleParameterCollection.h"

#include "RimSummaryEnsembleParameter.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimSummaryEnsembleParameterCollection, "RimSummaryEnsembleParameterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsembleParameterCollection::RimSummaryEnsembleParameterCollection()
{
    CAF_PDM_InitObject( "Ensemble Parameters", ":/Parameter.svg", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_parameters, "Parameters", "Parameters" );

    CAF_PDM_InitField( &m_ensembleId, "EnsembleId", -1, "EnsembleId" );
    m_ensembleId.uiCapability()->setUiHidden( true );

    nameField()->uiCapability()->setUiHidden( true );
    nameField()->uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsembleParameterCollection::~RimSummaryEnsembleParameterCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsembleParameterCollection::hasParameter( const QString name ) const
{
    for ( const auto& p : m_parameters )
    {
        if ( p->name() == name ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleParameterCollection::addParameter( QString name, bool checkDuplicates )
{
    bool okToAdd = checkDuplicates ? !hasParameter( name ) : true;

    if ( okToAdd )
    {
        auto p = new RimSummaryEnsembleParameter();
        p->setName( name );
        p->setEnsembleId( ensembleId() );
        m_parameters.push_back( p );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleParameterCollection::deleteChildren()
{
    m_parameters.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsembleParameterCollection::isEmpty() const
{
    return m_parameters.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleParameterCollection::updateUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering ) const
{
    if ( isEmpty() ) return;

    auto subnode = uiTreeOrdering.add( "Parameters", ":/Parameter.svg" );

    for ( auto& p : m_parameters() )
    {
        subnode->add( p );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsembleParameterCollection::setEnsembleId( int ensembleId )
{
    m_ensembleId = ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryEnsembleParameterCollection::ensembleId() const
{
    return m_ensembleId;
}

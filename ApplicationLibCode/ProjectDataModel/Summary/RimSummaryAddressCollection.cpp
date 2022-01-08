/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimSummaryAddressCollection.h"

#include "RimSummaryAddress.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimSummaryAddressCollection, "RimSummaryAddressCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection::RimSummaryAddressCollection()
{
    CAF_PDM_InitObject( "Surfaces", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_adresses, "SummaryAddresses", "Addresses" );
    m_adresses.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_subfolders, "AddressSubfolders", "Subfolders" );
    m_subfolders.uiCapability()->setUiTreeHidden( true );
    // m_subfolders.uiCapability()->setUiTreeChildrenHidden( false );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection::~RimSummaryAddressCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                        QString                 uiConfigName /*= ""*/ )
{
    // uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::initAfterRead()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addAddress( RimSummaryAddress* address )
{
    m_adresses.push_back( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addToSubfolder( RimSummaryAddress* address, QString foldername )
{
    for ( auto& folder : m_subfolders )
    {
        if ( folder->name() == foldername )
        {
            folder->addAddress( address );
            return;
        }
    }

    RimSummaryAddressCollection* newfolder = new RimSummaryAddressCollection();
    newfolder->setName( foldername );
    newfolder->addAddress( address );

    m_subfolders.push_back( newfolder );
}

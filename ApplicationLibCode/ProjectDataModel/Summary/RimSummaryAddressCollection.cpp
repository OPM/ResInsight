/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryAddress.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimSummaryAddressCollection, "RimSummaryAddressCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection::RimSummaryAddressCollection()
{
    CAF_PDM_InitObject( "Folder", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_adresses, "SummaryAddresses", "Addresses" );
    m_adresses.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_subfolders, "AddressSubfolders", "Subfolders" );
    m_subfolders.uiCapability()->setUiTreeHidden( true );

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
bool RimSummaryAddressCollection::hasDataVector( const QString quantityName ) const
{
    for ( auto& address : m_adresses )
    {
        if ( address->quantityName() == quantityName ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddressCollection::hasDataVector( const std::string quantityName ) const
{
    return hasDataVector( QString::fromStdString( quantityName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addAddress( const RifEclipseSummaryAddress& address, int caseId, int ensembleId )
{
    if ( !hasDataVector( address.quantityName() ) )
    {
        m_adresses.push_back( RimSummaryAddress::wrapFileReaderAddress( address, caseId, ensembleId ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addToSubfolder( QString                         foldername,
                                                  const RifEclipseSummaryAddress& address,
                                                  int                             caseId,
                                                  int                             ensembleId )
{
    RimSummaryAddressCollection* folder = getOrCreateSubfolder( foldername );
    folder->addAddress( address, caseId, ensembleId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::updateFolderStructure( const std::set<RifEclipseSummaryAddress>& addresses,
                                                         int                                       caseId,
                                                         int                                       ensembleId )
{
    if ( addresses.size() == 0 ) return;

    RimSummaryAddressCollection* misc    = getOrCreateSubfolder( "Miscellaneous" );
    RimSummaryAddressCollection* fields  = getOrCreateSubfolder( "Field" );
    RimSummaryAddressCollection* regions = getOrCreateSubfolder( "Regions" );
    RimSummaryAddressCollection* wells   = getOrCreateSubfolder( "Wells" );
    RimSummaryAddressCollection* groups  = getOrCreateSubfolder( "Groups" );

    for ( const auto& address : addresses )
    {
        switch ( address.category() )
        {
            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_MISC:
                misc->addAddress( address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_FIELD:
                fields->addAddress( address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION:
                regions->addToSubfolder( QString::number( address.regionNumber() ), address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_GROUP:
                groups->addToSubfolder( QString::fromStdString( address.wellGroupName() ), address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL:
                wells->addToSubfolder( QString::fromStdString( address.wellName() ), address, caseId, ensembleId );
                break;

            default:
                continue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection* RimSummaryAddressCollection::getOrCreateSubfolder( const QString folderName )
{
    for ( auto& folder : m_subfolders )
    {
        if ( folder->name() == folderName )
        {
            return folder;
        }
    }

    RimSummaryAddressCollection* newFolder = new RimSummaryAddressCollection();
    newFolder->setName( folderName );
    m_subfolders.push_back( newFolder );
    return newFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::clear()
{
    m_adresses.clear();
    m_subfolders.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddressCollection::isEmpty() const
{
    return ( ( m_adresses.size() == 0 ) && ( m_subfolders.size() == 0 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::updateUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering ) const
{
    for ( auto& folder : m_subfolders() )
    {
        uiTreeOrdering.add( folder );
    }

    for ( auto& address : m_adresses() )
    {
        uiTreeOrdering.add( address );
    }
}

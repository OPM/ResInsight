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

template <>
void caf::AppEnum<RimSummaryAddressCollection::CollectionContentType>::setUp()
{
    addItem( RimSummaryAddressCollection::CollectionContentType::NOT_DEFINED, "NOT_DEFINED", "Not Defined" );
    addItem( RimSummaryAddressCollection::CollectionContentType::WELL, "WELL", "Well" );
    addItem( RimSummaryAddressCollection::CollectionContentType::WELL_GROUP, "WELL_GROUP", "Well Group" );
    addItem( RimSummaryAddressCollection::CollectionContentType::REGION, "REGION", "Region" );
    addItem( RimSummaryAddressCollection::CollectionContentType::MISC, "MISC", "Miscellaneous" );
    addItem( RimSummaryAddressCollection::CollectionContentType::FIELD, "FIELD", "Field" );
    setDefault( RimSummaryAddressCollection::CollectionContentType::NOT_DEFINED );
}

CAF_PDM_SOURCE_INIT( RimSummaryAddressCollection, "RimSummaryAddressCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection::RimSummaryAddressCollection()
{
    CAF_PDM_InitObject( "Folder", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_contentType, "ContentsType", "Contents" );
    m_contentType = RimSummaryAddressCollection::CollectionContentType::NOT_DEFINED;
    m_contentType.uiCapability()->setUiReadOnly( true );
    m_contentType.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_adresses, "SummaryAddresses", "Addresses" );
    m_adresses.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_subfolders, "AddressSubfolders", "Subfolders" );
    m_subfolders.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_caseId, "CaseId", -1, "CaseId" );
    m_caseId.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_ensembleId, "EnsembleId", -1, "EnsembleId" );
    m_ensembleId.uiCapability()->setUiHidden( true );

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
        if ( m_caseId == -1 ) m_caseId = caseId;
        if ( m_ensembleId == -1 ) m_ensembleId = ensembleId;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addToSubfolder( QString                         foldername,
                                                  CollectionContentType           folderType,
                                                  const RifEclipseSummaryAddress& address,
                                                  int                             caseId,
                                                  int                             ensembleId )
{
    RimSummaryAddressCollection* folder = getOrCreateSubfolder( foldername, folderType );
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

    RimSummaryAddressCollection* misc    = getOrCreateSubfolder( "Miscellaneous", CollectionContentType::MISC );
    RimSummaryAddressCollection* fields  = getOrCreateSubfolder( "Field", CollectionContentType::FIELD );
    RimSummaryAddressCollection* regions = getOrCreateSubfolder( "Regions", CollectionContentType::REGION_FOLDER );
    RimSummaryAddressCollection* wells   = getOrCreateSubfolder( "Wells", CollectionContentType::WELL_FOLDER );
    RimSummaryAddressCollection* groups  = getOrCreateSubfolder( "Groups", CollectionContentType::GROUP_FOLDER );

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
                regions->addToSubfolder( QString::number( address.regionNumber() ),
                                         CollectionContentType::REGION,
                                         address,
                                         caseId,
                                         ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_GROUP:
                groups->addToSubfolder( QString::fromStdString( address.groupName() ),
                                        CollectionContentType::WELL_GROUP,
                                        address,
                                        caseId,
                                        ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL:
                wells->addToSubfolder( QString::fromStdString( address.wellName() ),
                                       CollectionContentType::WELL,
                                       address,
                                       caseId,
                                       ensembleId );
                break;

            default:
                continue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection* RimSummaryAddressCollection::getOrCreateSubfolder( const QString         folderName,
                                                                                CollectionContentType createFolderType )
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
    newFolder->setContentType( createFolderType );
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
bool RimSummaryAddressCollection::canBeDragged() const
{
    bool ok = m_subfolders.size() == 0;

    ok = ok && ( m_contentType == CollectionContentType::WELL || m_contentType == CollectionContentType::WELL_GROUP ||
                 m_contentType == CollectionContentType::REGION );

    return ok;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::setContentType( CollectionContentType content )
{
    m_contentType = content;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection::CollectionContentType RimSummaryAddressCollection::contentType() const
{
    return m_contentType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::setEnsembleId( int ensembleId )
{
    m_ensembleId = ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::setCaseId( int caseId )
{
    m_caseId = caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddressCollection::isEnsemble() const
{
    return m_ensembleId >= 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryAddressCollection::ensembleId() const
{
    return m_ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryAddressCollection*> RimSummaryAddressCollection::subFolders() const
{
    return m_subfolders.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryAddressCollection::caseId() const
{
    return m_caseId;
}

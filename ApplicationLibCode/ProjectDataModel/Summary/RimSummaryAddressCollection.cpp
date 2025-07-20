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

#include "Summary/RiaSummaryDefines.h"

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryAddress.h"

#include "cafPdmUiTreeOrdering.h"

using namespace RifEclipseSummaryAddressDefines;

template <>
void caf::AppEnum<RimSummaryAddressCollection::CollectionContentType>::setUp()
{
    using CollectionContentType = RimSummaryAddressCollection::CollectionContentType;
    addItem( CollectionContentType::NOT_DEFINED, "NOT_DEFINED", "Not Defined" );
    addItem( CollectionContentType::WELL, "WELL", RiaDefines::summaryWell() );
    addItem( CollectionContentType::GROUP, "GROUP", RiaDefines::summaryWellGroup() );
    addItem( CollectionContentType::REGION, "REGION", RiaDefines::summaryRegion() );
    addItem( CollectionContentType::FIELD, "FIELD", RiaDefines::summaryField() );
    addItem( CollectionContentType::MISC, "MISC", RiaDefines::summaryMisc() );
    addItem( CollectionContentType::WELL_FOLDER, "WELL_FOLDER", RiaDefines::summaryWell() );
    addItem( CollectionContentType::GROUP_FOLDER, "GROUP_FOLDER", RiaDefines::summaryWellGroup() );
    addItem( CollectionContentType::REGION_FOLDER, "REGION_FOLDER", RiaDefines::summaryRegion() );
    addItem( CollectionContentType::NETWORK_FOLDER, "NETWORK_FOLDER", RiaDefines::summaryNetwork() );
    addItem( CollectionContentType::BLOCK, "BLOCK", RiaDefines::summaryBlock() );
    addItem( CollectionContentType::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( CollectionContentType::AQUIFER, "AQUIFER", RiaDefines::summaryAquifer() );
    addItem( CollectionContentType::NETWORK, "NETWORK", RiaDefines::summaryNetwork() );
    addItem( CollectionContentType::REGION_2_REGION, "REGION_2_REGION", RiaDefines::summaryRegion2Region() );
    addItem( CollectionContentType::WELL_CONNECTION, "WELL_CONNECTION", RiaDefines::summaryWellConnection() );
    addItem( CollectionContentType::WELL_COMPLETION, "WELL_COMPLETION", RiaDefines::summaryWellCompletion() );
    addItem( CollectionContentType::WELL_LGR, "WELL_LGR", RiaDefines::summaryLgrWell() );
    addItem( CollectionContentType::WELL_CONNECTION_LGR, "WELL_CONNECTION_LGR", RiaDefines::summaryLgrConnection() );
    addItem( CollectionContentType::WELL_SEGMENT, "WELL_SEGMENT", RiaDefines::summaryWellSegment() );
    addItem( CollectionContentType::BLOCK_LGR, "BLOCK_LGR", RiaDefines::summaryLgrBlock() );
    addItem( CollectionContentType::CALCULATED, "CALCULATED", RiaDefines::summaryCalculated() );
    addItem( CollectionContentType::IMPORTED, "IMPORTED", "Imported" );
    setDefault( CollectionContentType::NOT_DEFINED );
}

CAF_PDM_SOURCE_INIT( RimSummaryAddressCollection, "RimSummaryAddressCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::SummaryCategory RimSummaryAddressCollection::contentTypeToSummaryCategory( CollectionContentType contentType )
{
    switch ( contentType )
    {
        case CollectionContentType::WELL:
            return SummaryCategory::SUMMARY_WELL;
        case CollectionContentType::GROUP:
            return SummaryCategory::SUMMARY_GROUP;
        case CollectionContentType::REGION:
            return SummaryCategory::SUMMARY_REGION;
        case CollectionContentType::FIELD:
            return SummaryCategory::SUMMARY_FIELD;
        case CollectionContentType::MISC:
            return SummaryCategory::SUMMARY_MISC;
        case CollectionContentType::AQUIFER:
            return SummaryCategory::SUMMARY_AQUIFER;
        case CollectionContentType::NETWORK:
            return SummaryCategory::SUMMARY_NETWORK;
        case CollectionContentType::REGION_2_REGION:
            return SummaryCategory::SUMMARY_REGION_2_REGION;
        case CollectionContentType::WELL_CONNECTION:
            return SummaryCategory::SUMMARY_WELL_CONNECTION;
        case CollectionContentType::WELL_LGR:
            return SummaryCategory::SUMMARY_WELL_LGR;
        case CollectionContentType::WELL_CONNECTION_LGR:
            return SummaryCategory::SUMMARY_WELL_CONNECTION_LGR;
        case CollectionContentType::WELL_SEGMENT:
            return SummaryCategory::SUMMARY_WELL_SEGMENT;
        case CollectionContentType::BLOCK:
            return SummaryCategory::SUMMARY_BLOCK;
        case CollectionContentType::BLOCK_LGR:
            return SummaryCategory::SUMMARY_BLOCK_LGR;
        case CollectionContentType::IMPORTED:
            return SummaryCategory::SUMMARY_IMPORTED;
        case CollectionContentType::CALCULATED:
        case CollectionContentType::NOT_DEFINED:
        case CollectionContentType::WELL_FOLDER:
        case CollectionContentType::GROUP_FOLDER:
        case CollectionContentType::REGION_FOLDER:
        case CollectionContentType::NETWORK_FOLDER:
        case CollectionContentType::SUMMARY_CASE:
        default:
            return SummaryCategory::SUMMARY_INVALID;
    }
}

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

    CAF_PDM_InitFieldNoDefault( &m_subfolders, "AddressSubfolders", "Subfolders" );

    CAF_PDM_InitField( &m_caseId, "CaseId", -1, "CaseId" );
    m_caseId.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_ensembleId, "EnsembleId", -1, "EnsembleId" );
    m_ensembleId.uiCapability()->setUiHidden( true );

    nameField()->uiCapability()->setUiHidden( true );
    nameField()->uiCapability()->setUiReadOnly( true );

    setUiIconFromResourceString( iconResourceText() );
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
    for ( const auto& address : m_adresses )
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
void RimSummaryAddressCollection::addAddress( RimSummaryAddress* address )
{
    if ( !hasDataVector( address->quantityName() ) )
    {
        m_adresses.push_back( address );

        if ( m_caseId == -1 ) m_caseId = address->caseId();
        if ( m_ensembleId == -1 ) m_ensembleId = address->ensembleId();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addToSubfolder( QString foldername, CollectionContentType folderType, RimSummaryAddress* address )
{
    RimSummaryAddressCollection* folder = getOrCreateSubfolder( foldername, folderType );
    folder->addAddress( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::addToSubfolderTree( std::vector<std::pair<QString, CollectionContentType>> folders, RimSummaryAddress* address )
{
    RimSummaryAddressCollection* thefolder = this;
    for ( auto& [subfoldername, folderType] : folders )
    {
        thefolder = thefolder->getOrCreateSubfolder( subfoldername, folderType );
    }
    thefolder->addAddress( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::updateFolderStructure( const std::set<RifEclipseSummaryAddress>& addresses, int caseId, int ensembleId )
{
    if ( addresses.empty() ) return;

    auto* fields          = getOrCreateSubfolder( CollectionContentType::FIELD );
    auto* groups          = getOrCreateSubfolder( CollectionContentType::GROUP_FOLDER );
    auto* wells           = getOrCreateSubfolder( CollectionContentType::WELL_FOLDER );
    auto* aquifer         = getOrCreateSubfolder( CollectionContentType::AQUIFER );
    auto* networks        = getOrCreateSubfolder( CollectionContentType::NETWORK_FOLDER );
    auto* misc            = getOrCreateSubfolder( CollectionContentType::MISC );
    auto* regions         = getOrCreateSubfolder( CollectionContentType::REGION_FOLDER );
    auto* region2region   = getOrCreateSubfolder( CollectionContentType::REGION_2_REGION );
    auto* segment         = getOrCreateSubfolder( CollectionContentType::WELL_SEGMENT );
    auto* wellCompletions = getOrCreateSubfolder( CollectionContentType::WELL_COMPLETION );
    auto* wellConnection  = getOrCreateSubfolder( CollectionContentType::WELL_CONNECTION );
    auto* blocks          = getOrCreateSubfolder( CollectionContentType::BLOCK );
    auto* lgrwell         = getOrCreateSubfolder( CollectionContentType::WELL_LGR );
    auto* lgrConnection   = getOrCreateSubfolder( CollectionContentType::WELL_CONNECTION_LGR );
    auto* lgrblock        = getOrCreateSubfolder( CollectionContentType::BLOCK_LGR );
    auto* imported        = getOrCreateSubfolder( CollectionContentType::IMPORTED );

    // Sort addresses to have calculated results last per category
    std::vector<RifEclipseSummaryAddress> sortedAddresses;
    std::copy_if( addresses.begin(),
                  addresses.end(),
                  std::back_inserter( sortedAddresses ),
                  []( RifEclipseSummaryAddress x ) { return !x.isErrorResult(); } );
    std::sort( sortedAddresses.begin(),
               sortedAddresses.end(),
               []( const RifEclipseSummaryAddress& a, const RifEclipseSummaryAddress& b ) -> bool
               {
                   if ( a.category() != b.category() ) return a.category() < b.category();
                   if ( a.wellName() != b.wellName() ) return a.wellName() < b.wellName();
                   if ( a.regionNumber() != b.regionNumber() ) return a.regionNumber() < b.regionNumber();
                   if ( a.regionNumber2() != b.regionNumber2() ) return a.regionNumber2() < b.regionNumber2();
                   if ( a.groupName() != b.groupName() ) return a.groupName() < b.groupName();
                   if ( a.networkName() != b.networkName() ) return a.networkName() < b.networkName();
                   if ( a.lgrName() != b.lgrName() ) return a.lgrName() < b.lgrName();
                   if ( a.cellK() != b.cellK() ) return a.cellK() < b.cellK();
                   if ( a.cellJ() != b.cellJ() ) return a.cellJ() < b.cellJ();
                   if ( a.cellI() != b.cellI() ) return a.cellI() < b.cellI();
                   if ( a.wellSegmentNumber() != b.wellSegmentNumber() ) return a.wellSegmentNumber() < b.wellSegmentNumber();
                   if ( a.aquiferNumber() != b.aquiferNumber() ) return a.aquiferNumber() < b.aquiferNumber();
                   if ( a.wellCompletionNumber() != b.wellCompletionNumber() ) return a.wellCompletionNumber() < b.wellCompletionNumber();
                   if ( a.isErrorResult() != b.isErrorResult() ) return !a.isErrorResult();

                   // Calculated results are sorted last.
                   if ( a.isCalculated() != b.isCalculated() ) return a.isCalculated() < b.isCalculated();
                   return a.vectorName() < b.vectorName();
               } );

    std::vector<RimSummaryAddress*> rimAddresses;
    rimAddresses.resize( sortedAddresses.size() );

    // Make sure that the first object is created in the main thread. This is needed to avoid a race condition related to initialization of
    // static data for a PDM object.
    // See CreateObjectInMultipleThreads test in cafPdmBasicTest.cpp
    // This is a workaround until initialization of PDM objects is thread safe.
    {
        RimSummaryAddress dummyObject;
    }

#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( sortedAddresses.size() ); ++i )
    {
        rimAddresses[i] = RimSummaryAddress::wrapFileReaderAddress( sortedAddresses[i], caseId, ensembleId );
    }

    for ( auto rimAdr : rimAddresses )
    {
        auto address = rimAdr->address();
        switch ( address.category() )
        {
            case SummaryCategory::SUMMARY_FIELD:
                fields->addAddress( rimAdr );
                break;

            case SummaryCategory::SUMMARY_AQUIFER:
                aquifer->addToSubfolder( QString::number( address.aquiferNumber() ), CollectionContentType::AQUIFER, rimAdr );
                break;

            case SummaryCategory::SUMMARY_NETWORK:
                networks->addToSubfolder( QString::fromStdString( address.networkName() ), CollectionContentType::NETWORK, rimAdr );
                break;

            case SummaryCategory::SUMMARY_MISC:
                misc->addAddress( rimAdr );
                break;

            case SummaryCategory::SUMMARY_REGION:
                regions->addToSubfolder( QString::number( address.regionNumber() ), CollectionContentType::REGION, rimAdr );
                break;

            case SummaryCategory::SUMMARY_REGION_2_REGION:
                region2region->addToSubfolder( QString::fromStdString( address.itemUiText() ), CollectionContentType::REGION_2_REGION, rimAdr );
                break;

            case SummaryCategory::SUMMARY_GROUP:
                groups->addToSubfolder( QString::fromStdString( address.groupName() ), CollectionContentType::GROUP, rimAdr );
                break;

            case SummaryCategory::SUMMARY_WELL:
                wells->addToSubfolder( QString::fromStdString( address.wellName() ), CollectionContentType::WELL, rimAdr );
                break;

            case SummaryCategory::SUMMARY_WELL_CONNECTION:
                wellConnection->addToSubfolderTree( { { QString::fromStdString( address.wellName() ), CollectionContentType::WELL },
                                                      { QString::fromStdString( address.connectionAsString() ),
                                                        CollectionContentType::WELL_CONNECTION } },
                                                    rimAdr );
                break;

            case SummaryCategory::SUMMARY_WELL_COMPLETION:
                wellCompletions->addToSubfolderTree( { { QString::fromStdString( address.wellName() ), CollectionContentType::WELL },
                                                       { QString::number( address.wellCompletionNumber() ),
                                                         CollectionContentType::WELL_COMPLETION } },
                                                     rimAdr );
                break;

            case SummaryCategory::SUMMARY_WELL_SEGMENT:
                segment->addToSubfolderTree( { { QString::fromStdString( address.wellName() ), CollectionContentType::WELL },
                                               { QString::number( address.wellSegmentNumber() ), CollectionContentType::WELL_SEGMENT } },
                                             rimAdr );
                break;

            case SummaryCategory::SUMMARY_BLOCK:
                blocks->addToSubfolder( QString::fromStdString( address.blockAsString() ), CollectionContentType::BLOCK, rimAdr );
                break;

            case SummaryCategory::SUMMARY_WELL_LGR:
                lgrwell->addToSubfolderTree( { { QString::fromStdString( address.lgrName() ), CollectionContentType::WELL_LGR },
                                               { QString::fromStdString( address.wellName() ), CollectionContentType::WELL } },
                                             rimAdr );
                break;

            case SummaryCategory::SUMMARY_WELL_CONNECTION_LGR:
                lgrConnection->addToSubfolderTree( { { QString::fromStdString( address.lgrName() ), CollectionContentType::WELL_LGR },
                                                     { QString::fromStdString( address.wellName() ), CollectionContentType::WELL },
                                                     { QString::fromStdString( address.blockAsString() ), CollectionContentType::BLOCK } },
                                                   rimAdr );
                break;

            case SummaryCategory::SUMMARY_BLOCK_LGR:
                lgrblock->addToSubfolderTree( { { QString::fromStdString( address.lgrName() ), CollectionContentType::WELL_LGR },
                                                { QString::fromStdString( address.blockAsString() ), CollectionContentType::BLOCK } },
                                              rimAdr );
                break;

            case SummaryCategory::SUMMARY_IMPORTED:
                imported->addAddress( rimAdr );
                break;

            default:
                continue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection* RimSummaryAddressCollection::getOrCreateSubfolder( const QString folderName, CollectionContentType createFolderType )
{
    for ( auto& folder : m_subfolders )
    {
        if ( folder->name() == folderName )
        {
            return folder;
        }
    }

    auto* newFolder = new RimSummaryAddressCollection();
    newFolder->setName( folderName );
    newFolder->setContentType( createFolderType );
    m_subfolders.push_back( newFolder );
    return newFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAddressCollection* RimSummaryAddressCollection::getOrCreateSubfolder( CollectionContentType createFolderType )
{
    auto name = caf::AppEnum<CollectionContentType>::uiText( createFolderType );

    return getOrCreateSubfolder( name, createFolderType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::deleteChildren()
{
    m_adresses.deleteChildren();
    m_subfolders.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryAddressCollection::deleteCalculatedAddresses()
{
    std::vector<RimSummaryAddress*> toDelete;
    for ( const auto& a : m_adresses )
    {
        if ( a->address().isCalculated() )
        {
            toDelete.push_back( a );
        }
    }

    int calculationAddressCount = static_cast<int>( toDelete.size() );

    for ( auto a : toDelete )
    {
        m_adresses.removeChild( a );
        delete a;
    }

    for ( auto& folder : m_subfolders )
    {
        calculationAddressCount += folder->deleteCalculatedAddresses();
    }

    return calculationAddressCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddressCollection::isEmpty() const
{
    return ( ( m_adresses.empty() ) && ( m_subfolders.empty() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAddressCollection::canBeDragged() const
{
    bool ok = m_subfolders.empty();

    ok = ok && ( m_contentType == CollectionContentType::WELL || m_contentType == CollectionContentType::GROUP ||
                 m_contentType == CollectionContentType::NETWORK || m_contentType == CollectionContentType::REGION ||
                 m_contentType == CollectionContentType::WELL_SEGMENT );

    return ok || isFolder();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::updateUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering ) const
{
    for ( auto& folder : m_subfolders() )
    {
        if ( !folder->isEmpty() ) uiTreeOrdering.add( folder );
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
    setUiIconFromResourceString( iconResourceText() );
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
bool RimSummaryAddressCollection::isFolder() const
{
    return contentType() == CollectionContentType::WELL_FOLDER || contentType() == CollectionContentType::GROUP_FOLDER ||
           contentType() == CollectionContentType::NETWORK_FOLDER || contentType() == CollectionContentType::REGION_FOLDER;
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
    return m_subfolders.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryAddressCollection::caseId() const
{
    return m_caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryAddressCollection::iconResourceText() const
{
    switch ( m_contentType() )
    {
        case RimSummaryAddressCollection::CollectionContentType::WELL:
            return ":/summary/components/images/well.svg";
        case RimSummaryAddressCollection::CollectionContentType::GROUP:
            return ":/summary/components/images/group.svg";
        case RimSummaryAddressCollection::CollectionContentType::REGION:
            return ":/summary/components/images/region.svg";
        case RimSummaryAddressCollection::CollectionContentType::FIELD:
            return ":/summary/components/images/field.svg";
        case RimSummaryAddressCollection::CollectionContentType::MISC:
            return ":/summary/components/images/misc.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_FOLDER:
            return ":/summary/components/images/well.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_COMPLETION:
            return ":/summary/components/images/well-completion-01.svg";
        case RimSummaryAddressCollection::CollectionContentType::GROUP_FOLDER:
            return ":/summary/components/images/group.svg";
        case RimSummaryAddressCollection::CollectionContentType::NETWORK_FOLDER:
            return ":/summary/components/images/network.svg";
        case RimSummaryAddressCollection::CollectionContentType::REGION_FOLDER:
            return ":/summary/components/images/region.svg";
        case RimSummaryAddressCollection::CollectionContentType::BLOCK:
            return ":/summary/components/images/block.svg";
        case RimSummaryAddressCollection::CollectionContentType::SUMMARY_CASE:
            return ":/SummaryCase.svg";
        case RimSummaryAddressCollection::CollectionContentType::AQUIFER:
            return ":/summary/components/images/aquifer.svg";
        case RimSummaryAddressCollection::CollectionContentType::NETWORK:
            return ":/summary/components/images/network.svg";
        case RimSummaryAddressCollection::CollectionContentType::REGION_2_REGION:
            return ":/summary/components/images/region-region.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_CONNECTION:
            return ":/summary/components/images/block.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_LGR:
            return ":/summary/components/images/well.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_CONNECTION_LGR:
            return ":/summary/components/images/block.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_SEGMENT:
            return ":/summary/components/images/segment.svg";
        case RimSummaryAddressCollection::CollectionContentType::BLOCK_LGR:
            return ":/summary/components/images/block.svg";
        case RimSummaryAddressCollection::CollectionContentType::CALCULATED:
            return ":/summary/components/images/calculated.svg";
        case RimSummaryAddressCollection::CollectionContentType::IMPORTED:
            return ":/summary/components/images/others.svg";
        case RimSummaryAddressCollection::CollectionContentType::NOT_DEFINED:
        default:
            break;
    }
    return ":/Folder.png";
}

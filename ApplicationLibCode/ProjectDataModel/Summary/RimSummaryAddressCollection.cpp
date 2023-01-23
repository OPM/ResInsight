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

#include "RiaSummaryDefines.h"

#include "RifEclipseSummaryAddress.h"

#include "RimSummaryAddress.h"

#include "cafPdmUiTreeOrdering.h"

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
    addItem( CollectionContentType::BLOCK, "BLOCK", RiaDefines::summaryBlock() );
    addItem( CollectionContentType::SUMMARY_CASE, "SUMMARY_CASE", "Summary Case" );
    addItem( CollectionContentType::AQUIFER, "AQUIFER", RiaDefines::summaryAquifer() );
    addItem( CollectionContentType::NETWORK, "NETWORK", RiaDefines::summaryNetwork() );
    addItem( CollectionContentType::REGION_2_REGION, "REGION_2_REGION", RiaDefines::summaryRegion2Region() );
    addItem( CollectionContentType::WELL_COMPLETION, "WELL_COMPLETION", RiaDefines::summaryCompletion() );
    addItem( CollectionContentType::WELL_LGR, "WELL_LGR", RiaDefines::summaryLgrWell() );
    addItem( CollectionContentType::WELL_COMPLETION_LGR, "WELL_COMPLETION_LGR", RiaDefines::summaryLgrCompletion() );
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
    if ( !hasDataVector( address.vectorName() ) )
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
void RimSummaryAddressCollection::addToSubfolderTree( std::vector<QString>            folders,
                                                      CollectionContentType           folderType,
                                                      const RifEclipseSummaryAddress& address,
                                                      int                             caseId,
                                                      int                             ensembleId )
{
    RimSummaryAddressCollection* thefolder = this;
    for ( auto& subfoldername : folders )
    {
        thefolder = thefolder->getOrCreateSubfolder( subfoldername, folderType );
    }
    thefolder->setContentType( folderType );
    thefolder->addAddress( address, caseId, ensembleId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAddressCollection::updateFolderStructure( const std::set<RifEclipseSummaryAddress>& addresses,
                                                         int                                       caseId,
                                                         int                                       ensembleId )
{
    if ( addresses.empty() ) return;

    auto* fields        = getOrCreateSubfolder( CollectionContentType::FIELD );
    auto* aquifer       = getOrCreateSubfolder( CollectionContentType::AQUIFER );
    auto* network       = getOrCreateSubfolder( CollectionContentType::NETWORK );
    auto* misc          = getOrCreateSubfolder( CollectionContentType::MISC );
    auto* regions       = getOrCreateSubfolder( CollectionContentType::REGION_FOLDER );
    auto* region2region = getOrCreateSubfolder( CollectionContentType::REGION_2_REGION );
    auto* groups        = getOrCreateSubfolder( CollectionContentType::GROUP_FOLDER );
    auto* wells         = getOrCreateSubfolder( CollectionContentType::WELL_FOLDER );
    auto* completion    = getOrCreateSubfolder( CollectionContentType::WELL_COMPLETION );
    auto* segment       = getOrCreateSubfolder( CollectionContentType::WELL_SEGMENT );
    auto* blocks        = getOrCreateSubfolder( CollectionContentType::BLOCK );
    auto* lgrwell       = getOrCreateSubfolder( CollectionContentType::WELL_LGR );
    auto* lgrcompletion = getOrCreateSubfolder( CollectionContentType::WELL_COMPLETION_LGR );
    auto* lgrblock      = getOrCreateSubfolder( CollectionContentType::BLOCK_LGR );
    auto* imported      = getOrCreateSubfolder( CollectionContentType::IMPORTED );

    for ( const auto& address : addresses )
    {
        switch ( address.category() )
        {
            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_FIELD:
                fields->addAddress( address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_AQUIFER:
                aquifer->addToSubfolder( QString::number( address.aquiferNumber() ),
                                         CollectionContentType::AQUIFER,
                                         address,
                                         caseId,
                                         ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_NETWORK:
                network->addAddress( address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_MISC:
                misc->addAddress( address, caseId, ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION:
                regions->addToSubfolder( QString::number( address.regionNumber() ),
                                         CollectionContentType::REGION,
                                         address,
                                         caseId,
                                         ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_REGION_2_REGION:
                region2region->addToSubfolder( QString::fromStdString( address.itemUiText() ),
                                               CollectionContentType::REGION_2_REGION,
                                               address,
                                               caseId,
                                               ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_GROUP:
                groups->addToSubfolder( QString::fromStdString( address.groupName() ),
                                        CollectionContentType::GROUP,
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

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_COMPLETION:
                completion->addToSubfolderTree( { QString::fromStdString( address.wellName() ),
                                                  QString::fromStdString( address.blockAsString() ) },
                                                CollectionContentType::WELL_COMPLETION,
                                                address,
                                                caseId,
                                                ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_SEGMENT:
                segment->addToSubfolderTree( { QString::fromStdString( address.wellName() ),
                                               QString::number( address.wellSegmentNumber() ) },
                                             CollectionContentType::WELL_SEGMENT,
                                             address,
                                             caseId,
                                             ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_BLOCK:
                blocks->addToSubfolder( QString::fromStdString( address.blockAsString() ),
                                        CollectionContentType::BLOCK,
                                        address,
                                        caseId,
                                        ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_LGR:
                lgrwell->addToSubfolderTree( { QString::fromStdString( address.lgrName() ),
                                               QString::fromStdString( address.wellName() ) },
                                             CollectionContentType::WELL_LGR,
                                             address,
                                             caseId,
                                             ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL_COMPLETION_LGR:
                lgrcompletion->addToSubfolderTree( { QString::fromStdString( address.lgrName() ),
                                                     QString::fromStdString( address.wellName() ),
                                                     QString::fromStdString( address.blockAsString() ) },
                                                   CollectionContentType::WELL_COMPLETION_LGR,
                                                   address,
                                                   caseId,
                                                   ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_BLOCK_LGR:
                lgrblock->addToSubfolderTree( { QString::fromStdString( address.lgrName() ),
                                                QString::fromStdString( address.blockAsString() ) },
                                              CollectionContentType::BLOCK_LGR,
                                              address,
                                              caseId,
                                              ensembleId );
                break;

            case RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_IMPORTED:
                imported->addToSubfolder( QString::fromStdString( address.itemUiText() ),
                                          CollectionContentType::IMPORTED,
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
                 m_contentType == CollectionContentType::REGION );

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
    if ( contentType() == CollectionContentType::WELL_FOLDER || contentType() == CollectionContentType::GROUP_FOLDER ||
         contentType() == CollectionContentType::REGION_FOLDER )
    {
        return true;
    }

    return false;
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
    return m_subfolders.children();
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
        case RimSummaryAddressCollection::CollectionContentType::GROUP_FOLDER:
            return ":/summary/components/images/group.svg";
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
        case RimSummaryAddressCollection::CollectionContentType::WELL_COMPLETION:
            return ":/summary/components/images/well-completion.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_LGR:
            return ":/summary/components/images/well.svg";
        case RimSummaryAddressCollection::CollectionContentType::WELL_COMPLETION_LGR:
            return ":/summary/components/images/well-completion.svg";
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

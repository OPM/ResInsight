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
#pragma once

#include "RimNamedObject.h"

#include "RifEclipseSummaryAddressDefines.h"

#include "cafPdmChildArrayField.h"

#include <QString>
#include <string>
#include <vector>

class RimSummaryAddress;
class RifEclipseSummaryAddress;

class RimSummaryAddressCollection : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class CollectionContentType
    {
        NOT_DEFINED,
        WELL,
        GROUP,
        REGION,
        FIELD,
        MISC,
        WELL_FOLDER,
        GROUP_FOLDER,
        REGION_FOLDER,
        NETWORK_FOLDER,
        BLOCK,
        SUMMARY_CASE,
        AQUIFER,
        NETWORK,
        REGION_2_REGION,
        WELL_COMPLETION,
        WELL_CONNECTION,
        WELL_LGR,
        WELL_CONNECTION_LGR,
        WELL_SEGMENT,
        BLOCK_LGR,
        CALCULATED,
        IMPORTED
    };

    static RifEclipseSummaryAddressDefines::SummaryCategory contentTypeToSummaryCategory( CollectionContentType contentType );

public:
    RimSummaryAddressCollection();
    ~RimSummaryAddressCollection() override;

    void updateFolderStructure( const std::set<RifEclipseSummaryAddress>& addresses, int caseId, int ensembleId = -1 );

    void deleteChildren();
    int  deleteCalculatedAddresses();

    bool isEmpty() const;
    bool isEnsemble() const;
    bool isFolder() const;

    bool canBeDragged() const;

    void updateUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering ) const;

    void                  setContentType( CollectionContentType content );
    CollectionContentType contentType() const;

    void setCaseId( int caseId );
    int  caseId() const;
    void setEnsembleId( int ensembleId );
    int  ensembleId() const;

    std::vector<RimSummaryAddressCollection*> subFolders() const;

private:
    RimSummaryAddressCollection* getOrCreateSubfolder( const QString         folderName,
                                                       CollectionContentType createFolderType = CollectionContentType::NOT_DEFINED );

    RimSummaryAddressCollection* getOrCreateSubfolder( CollectionContentType createFolderType );

    bool hasDataVector( const QString quantityName ) const;
    bool hasDataVector( const std::string quantityName ) const;

    void addAddress( RimSummaryAddress* address );
    void addToSubfolder( QString foldername, CollectionContentType folderType, RimSummaryAddress* address );
    void addToSubfolderTree( std::vector<std::pair<QString, CollectionContentType>> folders, RimSummaryAddress* address );

    QString iconResourceText() const;

private:
    caf::PdmChildArrayField<RimSummaryAddress*>           m_adresses;
    caf::PdmChildArrayField<RimSummaryAddressCollection*> m_subfolders;
    caf::PdmField<caf::AppEnum<CollectionContentType>>    m_contentType;
    caf::PdmField<int>                                    m_caseId;
    caf::PdmField<int>                                    m_ensembleId;
};

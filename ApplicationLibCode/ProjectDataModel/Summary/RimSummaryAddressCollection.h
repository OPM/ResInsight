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
        BLOCK,
        BLOCK_FOLDER,
        SUMMARY_CASE
    };

public:
    RimSummaryAddressCollection();
    ~RimSummaryAddressCollection() override;

    void updateFolderStructure( const std::set<RifEclipseSummaryAddress>& addresses, int caseId, int ensembleId = -1 );

    void clear();

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
    RimSummaryAddressCollection*
        getOrCreateSubfolder( const QString         folderName,
                              CollectionContentType createFolderType = CollectionContentType::NOT_DEFINED );

    bool hasDataVector( const QString quantityName ) const;
    bool hasDataVector( const std::string quantityName ) const;

    void addAddress( const RifEclipseSummaryAddress& address, int caseId, int ensembleId = -1 );
    void addToSubfolder( QString                         foldername,
                         CollectionContentType           folderType,
                         const RifEclipseSummaryAddress& address,
                         int                             caseId,
                         int                             ensembleId = -1 );

private:
    caf::PdmChildArrayField<RimSummaryAddress*>           m_adresses;
    caf::PdmChildArrayField<RimSummaryAddressCollection*> m_subfolders;
    caf::PdmField<caf::AppEnum<CollectionContentType>>    m_contentType;
    caf::PdmField<int>                                    m_caseId;
    caf::PdmField<int>                                    m_ensembleId;
};

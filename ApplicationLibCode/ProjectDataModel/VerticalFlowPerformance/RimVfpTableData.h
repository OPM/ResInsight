/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "cafFilePath.h"

#include <memory>

class RigVfpTables;
class RimVfpTable;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimVfpTableData : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpTableData();

    void    setFileName( const QString& filename );
    QString baseFileName();
    void    ensureDataIsImported();

    std::vector<RimVfpTable*> tableDataSources() const;

    size_t tableCount() const;

    const RigVfpTables* vfpTables() const;

private:
    void updateObjectName();
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    caf::PdmField<caf::FilePath>          m_filePath;
    caf::PdmChildArrayField<RimVfpTable*> m_tables;

    std::unique_ptr<RigVfpTables> m_vfpTables;
};

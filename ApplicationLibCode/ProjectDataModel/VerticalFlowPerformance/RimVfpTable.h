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

#include "RimVfpDefines.h"
#include "RimVfpTableData.h"

#include "cafPdmPtrField.h"

class RimVfpTableData;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimVfpTable : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpTable();

    RimVfpTableData*         dataSource() const;
    int                      tableNumber() const;
    RimVfpDefines::TableType tableType() const;

    void setDataSource( RimVfpTableData* dataSource );
    void setTableNumber( int tableNumber );
    void setTableType( RimVfpDefines::TableType tableType );

    void ensureDataIsImported();

private:
    void updateObjectName();
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    caf::PdmPtrField<RimVfpTableData*>                    m_dataSource;
    caf::PdmField<int>                                    m_tableNumber;
    caf::PdmField<caf::AppEnum<RimVfpDefines::TableType>> m_tableType;
};

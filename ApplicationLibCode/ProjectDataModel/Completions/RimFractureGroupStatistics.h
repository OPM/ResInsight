/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

//==================================================================================================
///
///
//==================================================================================================
class RimFractureGroupStatistics : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureGroupStatistics();
    ~RimFractureGroupStatistics() override;
    void addFilePath( const QString& filePath );
    void loadAndUpdateData();

protected:
    void    defineEditorAttribute( const caf::PdmFieldHandle* field,
                                   QString                    uiConfigName,
                                   caf::PdmUiEditorAttribute* attribute ) override;
    QString generateFilePathsTable();

    caf::PdmField<std::vector<caf::FilePath>> m_filePaths;
    caf::PdmField<QString>                    m_filePathsTable;
};

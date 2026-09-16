/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>

#include <memory>

//==================================================================================================
///
//==================================================================================================
class RimcIdenticalGridCaseGroup_createStatisticsCase : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcIdenticalGridCaseGroup_createStatisticsCase( caf::PdmObjectHandle* self );

    void setPopulateResultSelection( bool populate );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<bool> m_populateResultSelection;
};

//==================================================================================================
/// Replace all source cases of the grid case group with the given grid files and reload the project.
///
/// The project must be saved to file, as the replacement is performed by reloading the project
/// through a RiaProjectModifier. Statistics are recomputed after the reload.
//==================================================================================================
class RimIdenticalGridCaseGroup_replaceSourceCases : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimIdenticalGridCaseGroup_replaceSourceCases( caf::PdmObjectHandle* self );

    void setGridFiles( const std::vector<QString>& gridFiles );
    void setProjectFile( const QString& projectFile );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<std::vector<QString>> m_gridFiles;
    caf::PdmField<QString>              m_projectFile;
};

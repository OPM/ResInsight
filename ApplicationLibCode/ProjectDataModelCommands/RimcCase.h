/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

//==================================================================================================
/// Replace the grid file of a case (Eclipse result case or GeoMech case) and reload the project.
///
/// The project must be saved to file, as the replacement is performed by reloading the project
/// through a RiaProjectModifier. An explicit project file can be given to override the current
/// project file name (used by the legacy command file interface).
//==================================================================================================
class RimCase_replaceGrid : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimCase_replaceGrid( caf::PdmObjectHandle* self );

    void setNewGridFile( const QString& newGridFile );
    void setProjectFile( const QString& projectFile );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_newGridFile;
    caf::PdmField<QString> m_projectFile;
};

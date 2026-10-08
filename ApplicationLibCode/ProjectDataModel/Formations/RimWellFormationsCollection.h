/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

namespace caf
{
class CmdFeatureMenuBuilder;
}

class RimWellFormationsFile;

//==================================================================================================
/// Project-level collection of well formations files (FMU formations.csv, well picks, ...), owned by
/// RimOilField. RimEnsembleFileSet and RimWellPath point into this collection, so a given file is
/// only parsed once even when it is used by several of them.
//==================================================================================================
class RimWellFormationsCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellFormationsCollection();

    const caf::PdmChildArrayField<RimWellFormationsFile*>& wellFormationsFiles() const { return m_wellFormationsFiles; }

    RimWellFormationsFile* findOrCreate( const QString& filePath );

    RimWellFormationsFile* findFileForWell( const QString& wellName ) const;

    std::vector<RimWellFormationsFile*> importFiles( const QStringList& filePaths );

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    caf::PdmChildArrayField<RimWellFormationsFile*> m_wellFormationsFiles;
};

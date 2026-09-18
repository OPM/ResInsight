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

#include "RiaDefines.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>

//==================================================================================================
/// Export a snapshot of a view window (3D view or plot) to a file in the given folder.
///
/// The file name is generated from the view window and prefixed with the given prefix. PDF output
/// is only supported for plots; 3D views are always exported as PNG. Requires a GUI.
//==================================================================================================
class RimViewWindow_exportSnapshot : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimViewWindow_exportSnapshot( caf::PdmObjectHandle* self );

    void setExportFolder( const QString& exportFolder );
    void setPrefix( const QString& prefix );
    void setWidth( int width );
    void setHeight( int height );
    void setFileFormat( RiaDefines::SnapshotFileFormat fileFormat );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

    /// Resolve the export folder: an explicit folder if given, otherwise the project relative "snapshots" folder.
    static QString resolveExportFolder( const QString& exportFolder );

private:
    caf::PdmField<QString>                                      m_exportFolder;
    caf::PdmField<QString>                                      m_prefix;
    caf::PdmField<int>                                          m_width;
    caf::PdmField<int>                                          m_height;
    caf::PdmField<caf::AppEnum<RiaDefines::SnapshotFileFormat>> m_fileFormat;
};

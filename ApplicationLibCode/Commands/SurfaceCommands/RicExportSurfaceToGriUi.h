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

#include "RifSurfio.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
/// PdmObject holding the parameters for resampling a surface onto a regular grid before export.
//==================================================================================================
class RicExportSurfaceToGriUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ExportFormat
    {
        GRI,
        IRAP
    };

    RicExportSurfaceToGriUi();

    void setDefaults( const QString& exportFolder, int nx, int ny, double originX, double originY, double incrementX, double incrementY );

    RigRegularSurfaceData gridParams() const;
    QString               exportFolder() const;
    ExportFormat          exportFormat() const;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<caf::AppEnum<ExportFormat>> m_exportFormat;
    caf::PdmField<QString>                    m_exportFolder;
    caf::PdmField<int>                        m_nx;
    caf::PdmField<int>                        m_ny;
    caf::PdmField<double>                     m_originX;
    caf::PdmField<double>                     m_originY;
    caf::PdmField<double>                     m_incrementX;
    caf::PdmField<double>                     m_incrementY;
};

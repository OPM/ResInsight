/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "cafPdmObject.h"

//==================================================================================================
///
//==================================================================================================
class RicExportContourMapToTextUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicExportContourMapToTextUi();

    QString exportFileName() const;
    void    setExportFileName( const QString& exportFileName );
    bool    exportLocalCoordinates() const;
    QString undefinedValueLabel() const;
    bool    excludeUndefinedValues() const;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<QString> m_exportFileName;
    caf::PdmField<bool>    m_exportLocalCoordinates;
    caf::PdmField<QString> m_undefinedValueLabel;
    caf::PdmField<bool>    m_excludeUndefinedValues;
};

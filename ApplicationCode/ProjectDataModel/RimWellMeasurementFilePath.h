/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
///
//==================================================================================================
class RimWellMeasurementFilePath : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementFilePath();
    ~RimWellMeasurementFilePath() override;

    QString filePath() const;
    void    setFilePath( const QString& filePath );

private:
    caf::PdmFieldHandle* userDescriptionField() override;

    caf::PdmField<QString>       m_userDescription;
    caf::PdmField<caf::FilePath> m_filePath;
};

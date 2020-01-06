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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfObject.h"

#include "cafPdmFieldCvfColor.h"

class RigSurface;

class RimSurface : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurface();
    ~RimSurface() override;

    void    setSurfaceFilePath( const QString& filePath );
    QString surfaceFilePath()
    {
        return m_surfaceDefinitionFilePath().path();
    }
    void setColor( const cvf::Color3f& color )
    {
        m_color = color;
    }

    bool updateDataFromFile();

    QString userDescription();

private:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue ) override;

    caf::PdmField<caf::FilePath> m_surfaceDefinitionFilePath;
    caf::PdmField<QString>       m_userDescription;
    caf::PdmField<cvf::Color3f>  m_color;

    cvf::ref<RigSurface> m_surfaceData;
};

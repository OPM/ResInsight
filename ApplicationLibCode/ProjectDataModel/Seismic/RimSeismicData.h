/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

#include <memory>

class RimGenericParameter;

namespace cvf
{
class BoundingBox;
}

class RimSeismicData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicData();
    ~RimSeismicData() override;

    void    setFileName( QString filename );
    QString fileName() const;

    QString userDescription();
    void    setUserDescription( QString description );

    void updateMetaData();

    double zMin() const;
    double zMax() const;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

    cvf::BoundingBox* boundingBox() const;

private:
    caf::PdmField<QString>                        m_filename;
    caf::PdmField<QString>                        m_userDescription;
    caf::PdmChildArrayField<RimGenericParameter*> m_metadata;

    std::shared_ptr<cvf::BoundingBox> m_boundingBox;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"

class RimPolygon;

class RimPolygonFile : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonFile();
    ~RimPolygonFile() override;

    void loadData();

    std::vector<RimPolygon*> polygons() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void loadPolygonsFromFile();

private:
    caf::PdmField<caf::FilePath> m_fileName;

    caf::PdmChildArrayField<RimPolygon*> m_polygons;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaSeismicDefines.h"

#include "cafPdmField.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <QString>

class RimSeismicView;
class RimSeismicData;

class RimSeismicViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicViewCollection();
    ~RimSeismicViewCollection() override;

    bool isEmpty();

    RimSeismicView* addView( RimSeismicData* data, RiaDefines::SeismicSectionType defaultSection );

    std::vector<RimSeismicView*> views() const;

private:
    caf::PdmChildArrayField<RimSeismicView*> m_views;
};

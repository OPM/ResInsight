/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "cafPdmObjectCollection.h"

class RimDataView;

class RimDataViewCollection : public caf::PdmObjectCollection<RimDataView>
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataViewCollection();
    ~RimDataViewCollection() override;

    RimDataView* addView();

    std::vector<RimDataView*> views() const;
};

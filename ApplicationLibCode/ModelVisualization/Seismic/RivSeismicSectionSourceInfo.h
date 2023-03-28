/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "cafPdmPointer.h"

#include "cvfObject.h"

class RimSeismicSection;

class RivSeismicSectionSourceInfo : public cvf::Object
{
public:
    RivSeismicSectionSourceInfo( RimSeismicSection* section, int partIndex );

    RimSeismicSection* section() const;
    int                partIndex() const;

private:
    caf::PdmPointer<RimSeismicSection> m_section;
    int                                m_partIndex;
};

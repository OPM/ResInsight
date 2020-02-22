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

namespace cvf
{
class GeometryBuilder;
}

class RivDiskGeometryGenerator
{
public:
    RivDiskGeometryGenerator();

    void setRelativeRadius( float relativeRadius );
    void setRelativeLength( float relativeLength );

    void setNumSlices( unsigned int numSlices );

    void generate( cvf::GeometryBuilder* builder );

private:
    float m_relativeRadius;
    float m_relativeLength;

    unsigned int m_numSlices;
};

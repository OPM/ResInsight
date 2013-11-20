//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfDrawableGeo.h"

namespace cvf {
    class GeometryBuilder;
}

namespace cvfu {


//==================================================================================================
//
// Import of (partial) Wavefront OBJ files
//
//==================================================================================================
class WavefrontObjImport
{
public:
    WavefrontObjImport();

    bool    readFile(const cvf::String& fileName);

    void    buildGeometry(cvf::GeometryBuilder* builder);

private:
    static std::string                  nextRecord(std::ifstream* stream);
    static std::vector<std::string>&    split(const std::string& s, char delim, std::vector<std::string>& elems);

private:
    std::vector<double>     m_nodes;
    std::vector<cvf::uint>  m_faceList;

};

}

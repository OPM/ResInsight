/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RifElementPropertyTableReader.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <map>
#include <string>
#include <vector>

//==================================================================================================
//
//
//==================================================================================================
class RifElementPropertyReader : public cvf::Object
{
public:
    RifElementPropertyReader();
    virtual ~RifElementPropertyReader();

    void addFile(const std::string& fileName);

    std::map<std::string, std::vector<std::string>> scalarElementFields();

    std::map<std::string, std::vector<float>> readAllElementPropertiesInFileContainingField(const std::string& fieldName);

private:
    std::map<std::string, RifElementPropertyMetadata> m_fields;
};

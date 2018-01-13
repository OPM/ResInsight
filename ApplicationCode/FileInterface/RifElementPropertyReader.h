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
#include <unordered_map>
#include <vector>

//==================================================================================================
//
//
//==================================================================================================
class RifElementPropertyReader : public cvf::Object
{
public:
    RifElementPropertyReader(const std::vector<int>& elementIdxToId);
    virtual ~RifElementPropertyReader();

    void addFile(const std::string& fileName);
    void removeFile(const std::string& fileName);

    std::map<std::string, std::vector<std::string>> scalarElementFields() const;

    std::map<std::string, std::vector<float>> readAllElementPropertiesInFileContainingField(const std::string& fieldName);

    std::vector<std::string> fieldsInFile(const std::string& fileName) const;

private:
    void makeElementIdToIdxMap();
    static void outputWarningAboutWrongFileData();

private:
    std::map<std::string, RifElementPropertyMetadata> m_fieldsMetaData;
    std::vector<int> m_elementIdxToId;
    std::unordered_map<int, int> m_elementIdToIdx;
};

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigWellPathFormations.h"

#include <map>
#include <utility>
#include <vector>

#include "cvfBase.h"
#include "cvfObject.h"

#include <QString>

class ElementPropertyTable;
class ElementPropertyMetadata;

//==================================================================================================
///
//==================================================================================================
class RifElementPropertyTableReader : cvf::Object
{
public:
    static ElementPropertyMetadata  readMetadata(const QString& filePath);
    static void                     readData(const ElementPropertyMetadata *metadata, ElementPropertyTable *table);
};

//==================================================================================================
///
//==================================================================================================
class FileParseException
{
public:
    FileParseException(const QString &message) : message(message) {}
    QString  message;
};

//==================================================================================================
///
//==================================================================================================
class ElementPropertyMetadata
{
public:
    QString                 fileName;
    std::vector<QString>    dataColumns;
};


//==================================================================================================
///
//==================================================================================================
class ElementPropertyTable
{
public:
    ElementPropertyTable() : hasData(false) {}

    ElementPropertyMetadata         metadata;
    bool                            hasData;
    std::vector<int>                elementIds;
    std::vector<std::vector<float>> data;
};


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

#include <map>
#include <utility>
#include <vector>

#include "cvfBase.h"
#include "cvfObject.h"

#include <QString>

class RifElementPropertyTable;
class RifElementPropertyMetadata;

//==================================================================================================
///
//==================================================================================================
class RifElementPropertyTableReader : cvf::Object
{
public:
    static RifElementPropertyMetadata   readMetadata(const QString& filePath);
    static void                         readData(const RifElementPropertyMetadata *metadata, RifElementPropertyTable *table);
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
class RifElementPropertyMetadata
{
public:
    QString                 fileName;
    std::vector<QString>    dataColumns;
};


//==================================================================================================
///
//==================================================================================================
class RifElementPropertyTable
{
public:
    RifElementPropertyTable() : hasData(false) {}

    RifElementPropertyMetadata      metadata;
    bool                            hasData;
    std::vector<int>                elementIds;
    std::vector<std::vector<float>> data;
};


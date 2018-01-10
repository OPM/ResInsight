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

#include "RifElementPropertyReader.h"

#include "cvfAssert.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifElementPropertyReader::RifElementPropertyReader() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifElementPropertyReader::~RifElementPropertyReader() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElementPropertyReader::addFile(const std::string& fileName)
{
    RifElementPropertyMetadata metaData = RifElementPropertyTableReader::readMetadata(QString::fromStdString(fileName));
    for (QString field : metaData.dataColumns)
    {
        m_fields[field.toStdString()] = metaData;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifElementPropertyReader::scalarElementFields()
{
    std::map<std::string, std::vector<std::string>> fields;

    for (std::map<std::string, RifElementPropertyMetadata>::iterator field = m_fields.begin(); field != m_fields.end(); field++)
    {
        fields[field->first];
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<float>>
    RifElementPropertyReader::readAllElementPropertiesInFileContainingField(const std::string& fieldName)
{
    RifElementPropertyTable table;
    RifElementPropertyTableReader::readData(&m_fields[fieldName], &table);

    CVF_ASSERT(m_fields[fieldName].dataColumns.size() == table.data.size());

    std::map<std::string, std::vector<float>> fieldAndData;

    for (size_t i = 0; i < table.data.size(); i++)
    {
        fieldAndData[m_fields[fieldName].dataColumns[i].toStdString()].swap(table.data[i]);
    }

    return fieldAndData;
}

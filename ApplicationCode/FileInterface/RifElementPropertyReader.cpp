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

#include <cmath>

#include <QMessageBox>
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifElementPropertyReader::RifElementPropertyReader(const std::vector<int>& elementIdxToId) : m_elementIdxToId(elementIdxToId) {}

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
        m_fieldsMetaData[field.toStdString()] = metaData;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElementPropertyReader::removeFile(const std::string& fileName)
{
    std::map<std::string, RifElementPropertyMetadata> tempMetaData;

    for (std::pair<std::string, RifElementPropertyMetadata> metaData : m_fieldsMetaData)
    {
        if (metaData.second.fileName.toStdString() != fileName)
        {
            tempMetaData[metaData.first] = metaData.second;
        }
    }

    m_fieldsMetaData.swap(tempMetaData);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifElementPropertyReader::scalarElementFields() const
{
    std::vector<std::string> fields;

    for (std::map<std::string, RifElementPropertyMetadata>::const_iterator field = m_fieldsMetaData.begin();
         field != m_fieldsMetaData.end(); field++)
    {
        fields.push_back(field->first);
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<float>>
    RifElementPropertyReader::readAllElementPropertiesInFileContainingField(const std::string& fieldName)
{
    std::map<std::string, std::vector<float>> fieldAndData;

    if (m_fieldsMetaData.find(fieldName) == m_fieldsMetaData.end())
    {
        return fieldAndData;
    }
    
    RifElementPropertyTable table;
    RifElementPropertyTableReader::readData(&m_fieldsMetaData[fieldName], &table);

    CVF_ASSERT(m_fieldsMetaData[fieldName].dataColumns.size() == table.data.size());

    for (size_t i = 0; i < table.data.size(); i++)
    {
        CVF_ASSERT(table.data[i].size() == table.elementIds.size());
    }

    const std::vector<int>& elementIdsFromFile = table.elementIds;

    if (elementIdsFromFile == m_elementIdxToId)
    {
        for (size_t i = 0; i < table.data.size(); i++)
        {
            const std::string& currentFieldFromFile = m_fieldsMetaData[fieldName].dataColumns[i].toStdString();
            fieldAndData[currentFieldFromFile]      = table.data[i];
        }
    }
    else if (elementIdsFromFile.size() > m_elementIdxToId.size() && elementIdsFromFile.size() > m_elementIdToIdx.size())
    {
        RifElementPropertyReader::outputWarningAboutWrongFileData();
        return fieldAndData;
    }
    else
    {
        if (m_elementIdToIdx.size() == 0)
        {
            makeElementIdToIdxMap();
        }

        std::vector<int> fileIdxToElementIdx;
        fileIdxToElementIdx.reserve(elementIdsFromFile.size());

        for (size_t i = 0; i < elementIdsFromFile.size(); i++)
        {
            std::unordered_map<int /*elm ID*/, int /*elm idx*/>::const_iterator it = m_elementIdToIdx.find(elementIdsFromFile[i]);
            if (it == m_elementIdToIdx.end())
            {
                RifElementPropertyReader::outputWarningAboutWrongFileData();
                return fieldAndData;
            }

            fileIdxToElementIdx.push_back(it->second);
        }

        for (size_t i = 0; i < table.data.size(); i++)
        {
            std::string currentFieldFromFile = m_fieldsMetaData[fieldName].dataColumns[i].toStdString();

            const std::vector<float>& currentColumn = table.data[i];

            std::vector<float> tempResult(m_elementIdToIdx.size(), HUGE_VAL);

            for (size_t j = 0; j < currentColumn.size(); j++)
            {
                tempResult[fileIdxToElementIdx[j]] = currentColumn[j];
            }

            fieldAndData[currentFieldFromFile].swap(tempResult);
        }
    }

    return fieldAndData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifElementPropertyReader::fieldsInFile(const std::string& fileName) const
{
    std::vector<std::string> fields;

    for (std::pair<std::string, RifElementPropertyMetadata> metaData : m_fieldsMetaData)
    {
        if (metaData.second.fileName.toStdString() == fileName)
        {
            for (const QString& column : metaData.second.dataColumns)
            {
                fields.push_back(column.toStdString());
            }
        }
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElementPropertyReader::makeElementIdToIdxMap()
{
    m_elementIdToIdx.reserve(m_elementIdxToId.size());

    for (size_t i = 0; i < m_elementIdxToId.size(); i++)
    {
        m_elementIdToIdx[m_elementIdxToId[i]] = (int)i;
    }

    std::vector<int> tempVectorForDeletion;
    m_elementIdxToId.swap(tempVectorForDeletion);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifElementPropertyReader::outputWarningAboutWrongFileData()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    QString warningText;
    warningText = QString("The chosen result property does not fit the model");
    msgBox.setText(warningText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

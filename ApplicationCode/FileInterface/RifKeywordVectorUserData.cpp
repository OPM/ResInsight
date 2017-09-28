/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RifKeywordVectorUserData.h"

#include "RiaDateTimeTools.h"
#include "RiaLogging.h"

#include "RifEclipseSummaryAddress.h"
#include "RifKeywordVectorParser.h"
#include "RifRsmspecParserTools.h"

#include "cafUtils.h"

#include <QDateTime>
#include <QFile>
#include <QStringList>
#include <QTextStream>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifKeywordVectorUserData::RifKeywordVectorUserData()
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifKeywordVectorUserData::~RifKeywordVectorUserData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifKeywordVectorUserData::parse(const QString& data)
{
    m_allResultAddresses.clear();
    m_timeSteps.clear();

    m_parser = std::unique_ptr<RifKeywordVectorParser>(new RifKeywordVectorParser(data));
    if (!m_parser)
    {
        RiaLogging::error(QString("Failed to parse file"));

        return false;
    }

    std::vector<std::map<QString, QString>> keyValuePairVector;
    for (const KeywordBasedVector& keywordVector : m_parser->keywordBasedVectors())
    {
        std::map<QString, QString> keyValuePairs;

        for (auto s : keywordVector.header)
        {
            QString ss = QString::fromStdString(s);

            QStringList entries = ss.split(" ");
            if (entries.size() == 2)
            {
                keyValuePairs[entries[0]] = entries[1];
            }
        }

        keyValuePairVector.push_back(keyValuePairs);
    }

    // Find all time vectors

    for (size_t i = 0; i < keyValuePairVector.size(); i++)
    {
        const std::map<QString, QString>& keyValuePairs = keyValuePairVector[i];

        if (isTimeHeader(keyValuePairs))
        {
            QString unitText = valueForKey(keyValuePairs, "UNITS");
            quint64 scaleFactor = RiaDateTimeTools::secondsFromUnit(unitText.toStdString());

            std::vector<time_t> ts;
            for (const auto& year : m_parser->keywordBasedVectors()[i].values)
            {
                ts.push_back(scaleFactor * year);
            }

            m_timeSteps.push_back(ts);

            QString originText = valueForKey(keyValuePairs, "ORIGIN");

            m_mapFromOriginToTimeStepIndex[originText] = m_timeSteps.size() - 1;
        }
    }

    // Find all data vectors having a reference to a time step vector

    for (size_t i = 0; i < keyValuePairVector.size(); i++)
    {
        const std::map<QString, QString>& keyValuePairs = keyValuePairVector[i];

        if (!isTimeHeader(keyValuePairs))
        {
            if (isVectorHeader(keyValuePairs))
            {
                QString originText = valueForKey(keyValuePairs, "ORIGIN");
                auto timeStepIndexIterator = m_mapFromOriginToTimeStepIndex.find(originText);
                if (timeStepIndexIterator != m_mapFromOriginToTimeStepIndex.end())
                {
                    QString vectorText = valueForKey(keyValuePairs, "VECTOR");

                    RifEclipseSummaryAddress addr(RifEclipseSummaryAddress::SUMMARY_WELL,
                                                  vectorText.toStdString(),
                                                  -1,
                                                  -1,
                                                  "",
                                                  originText.toStdString(),
                                                  -1,
                                                  "",
                                                  -1,
                                                  -1,
                                                  -1
                                                  );

                    m_allResultAddresses.push_back(addr);

                    m_mapFromAddressToTimeIndex[addr] = timeStepIndexIterator->second;
                    m_mapFromAddressToVectorIndex[addr] = i;
                }
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifKeywordVectorUserData::values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const
{
    auto search = m_mapFromAddressToVectorIndex.find(resultAddress);
    if (search == m_mapFromAddressToVectorIndex.end()) return false;

    for (const auto& v : m_parser->keywordBasedVectors()[search->second].values)
    {
        values->push_back(v);
    }
        
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifKeywordVectorUserData::timeSteps(const RifEclipseSummaryAddress& resultAddress) const
{
    auto timeIndexIterator = m_mapFromAddressToTimeIndex.find(resultAddress);
    if (timeIndexIterator != m_mapFromAddressToTimeIndex.end())
    {
        return m_timeSteps[timeIndexIterator->second];
    }

    static std::vector<time_t> emptyVector;
    
    return emptyVector;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RifKeywordVectorUserData::unitName(const RifEclipseSummaryAddress& resultAddress) const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifKeywordVectorUserData::isTimeHeader(const std::map<QString, QString>& header)
{
    for (const auto& keyValue : header)
    {
        if (keyValue.first == "VECTOR" && keyValue.second == "YEARX")
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifKeywordVectorUserData::isVectorHeader(const std::map<QString, QString>& header)
{
    for (const auto& keyValue : header)
    {
        if (keyValue.first == "VECTOR")
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifKeywordVectorUserData::valueForKey(const std::map<QString, QString>& header, const QString& key)
{
    auto it = header.find(key);
    if (it != header.end())
    {
        return it->second;
    }

    return "";
}


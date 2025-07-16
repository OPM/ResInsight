/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RiaDefines.h"

#include <map>
#include <string>

class RifEclipseKeywordValueCount
{
public:
    enum class KeywordDataType
    {
        UNKNOWN,
        FLOAT,
        DOUBLE,
        INTEGER,
    };

public:
    RifEclipseKeywordValueCount( const std::string& keyword, size_t itemCount, KeywordDataType dataType )
        : m_keyword( keyword )
        , m_valueCount( itemCount )
        , m_dataType( dataType )
    {
    }

    RifEclipseKeywordValueCount()
        : m_valueCount( 0 )
        , m_dataType( KeywordDataType::UNKNOWN )
    {
    }

    void addValueCount( size_t valueCount ) { m_valueCount += valueCount; }

    std::string     keyword() const { return m_keyword; }
    size_t          valueCount() const { return m_valueCount; }
    KeywordDataType dataType() const { return m_dataType; }

    static RiaDefines::ResultDataType mapType( RifEclipseKeywordValueCount::KeywordDataType dataType )
    {
        switch ( dataType )
        {
            case RifEclipseKeywordValueCount::KeywordDataType::FLOAT:
                return RiaDefines::ResultDataType::FLOAT;
            case RifEclipseKeywordValueCount::KeywordDataType::DOUBLE:
                return RiaDefines::ResultDataType::DOUBLE;
            case RifEclipseKeywordValueCount::KeywordDataType::INTEGER:
                return RiaDefines::ResultDataType::INTEGER;
            case RifEclipseKeywordValueCount::KeywordDataType::UNKNOWN:
                return RiaDefines::ResultDataType::UNKNOWN;
        }

        return RiaDefines::ResultDataType::UNKNOWN;
    }

private:
    std::string     m_keyword;
    size_t          m_valueCount;
    KeywordDataType m_dataType;
};

//==================================================================================================
//
//==================================================================================================
class RifEclipseReportKeywords
{
public:
    void appendKeywordCount( const RifEclipseReportKeywords& other );
    void appendKeywordCount( const std::string& keyword, size_t valueCount, RifEclipseKeywordValueCount::KeywordDataType dataType );

    std::vector<RifEclipseKeywordValueCount> keywordValueCounts() const;

private:
    std::map<std::string, RifEclipseKeywordValueCount> m_keywordValueCounts;
};

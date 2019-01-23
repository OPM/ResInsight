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

#include "RiaDefines.h"
#include <QString>

class RigEclipseResultAddress
{
public:
    RigEclipseResultAddress()
        : scalarResultIndex(-1)
        , m_resultCatType(RiaDefines::UNDEFINED)
    {}

    explicit RigEclipseResultAddress(size_t ascalarResultIndex)
        : scalarResultIndex(ascalarResultIndex)
        ,  m_resultCatType(RiaDefines::UNDEFINED)
    {}

    explicit RigEclipseResultAddress(RiaDefines::ResultCatType type, const QString& resultName)
        : scalarResultIndex(-1)
        , m_resultCatType(type)
        , m_resultName(resultName)
    {}

    bool isValid() const
    {
        if (scalarResultIndex != -1) return true;
        if (!m_resultName.isEmpty()) return true;

        return false;
    }

    bool operator< (const RigEclipseResultAddress& other ) const
    {
        if (scalarResultIndex !=  other.scalarResultIndex)
        {
            return (scalarResultIndex <  other.scalarResultIndex);
        }

        if (m_resultCatType != other.m_resultCatType)
        {
            return (m_resultCatType <  other.m_resultCatType);
        }

        return (m_resultName <  other.m_resultName);
    }

    size_t scalarResultIndex; // Temporary. Must be removed 

    RiaDefines::ResultCatType m_resultCatType;
    QString m_resultName;
};



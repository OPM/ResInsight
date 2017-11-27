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

#include "RigWellPathFormations.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellPathFormations::RigWellPathFormations(std::vector<std::pair<double, QString>> measuredDepthAndFormationNames, const QString& filePath, const QString& key)
{
    m_measuredDepthAndFormationNames = measuredDepthAndFormationNames;
    m_filePath = filePath;
    m_keyInFile = key;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<std::pair<double, QString>>& RigWellPathFormations::measuredDepthAndFormationNames() const
{
    return m_measuredDepthAndFormationNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::measuredDepthAndFormationNames(std::vector<QString>& names, std::vector<double>& measuredDepths) const
{
    for (std::pair<double, QString> mdAndFormName : m_measuredDepthAndFormationNames)
    {
        measuredDepths.push_back(mdAndFormName.first);
        names.push_back(mdAndFormName.second);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::measuredDepthAndFormationNamesWithoutDuplicates(std::vector<QString>& names, std::vector<double>& measuredDepths) const
{
    names.clear();
    measuredDepths.clear();

    std::map<double, QString> tempMakeVectorUniqueOnMeasuredDepth;

    for (const std::pair<double, QString>& mdAndFormName : m_measuredDepthAndFormationNames)
    {
        if (tempMakeVectorUniqueOnMeasuredDepth.find(mdAndFormName.first) == tempMakeVectorUniqueOnMeasuredDepth.end())
        {
            measuredDepths.push_back(mdAndFormName.first);
            names.push_back(mdAndFormName.second);
            tempMakeVectorUniqueOnMeasuredDepth[mdAndFormName.first] = mdAndFormName.second;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellPathFormations::filePath() const
{
    return m_filePath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellPathFormations::keyInFile() const
{
    return m_keyInFile;
}

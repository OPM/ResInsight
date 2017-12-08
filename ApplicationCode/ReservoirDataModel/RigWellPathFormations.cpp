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

#include "QStringList"

#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellPathFormations::RigWellPathFormations(std::vector<RigWellPathFormation> formations, const QString& filePath, const QString& key)
{
    m_filePath = filePath;
    m_keyInFile = key;
    m_formations = formations;
}

struct MeasuredDepthComp
{
    bool operator()(const double& md1, const double& md2) const
    {
        if (cvf::Math::abs(md1 - md2) < 1.0)
        {
            return false;
        }
        return md1 < md2;
    }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::measuredDepthAndFormationNamesWithoutDuplicatesOnDepth(std::vector<QString>* names, std::vector<double>* measuredDepths) const
{
    names->clear();
    measuredDepths->clear();

    std::map<double, bool, MeasuredDepthComp> tempMakeVectorUniqueOnMeasuredDepth;

    for (RigWellPathFormation formation : m_formations)
    {
        if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.mdTop))
        {
            measuredDepths->push_back(formation.mdTop);
            names->push_back(formation.formationName + " Top");
            tempMakeVectorUniqueOnMeasuredDepth[formation.mdTop] = true;
        }
    }

    for (RigWellPathFormation formation : m_formations)
    {
        if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.mdBase))
        {
            measuredDepths->push_back(formation.mdBase);
            names->push_back(formation.formationName + " Base");
            tempMakeVectorUniqueOnMeasuredDepth[formation.mdBase] = true;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigWellPathFormations::formationNamesCount() const
{
    return m_formations.size();
}

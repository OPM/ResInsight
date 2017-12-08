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
RigWellPathFormations::RigWellPathFormations(std::vector<RigWellPathFormation> formations, const QString& filePath,
                                             const QString& key)
{
    m_filePath   = filePath;
    m_keyInFile  = key;
    m_formations = formations;
    m_maxLevelDetected = RigWellPathFormation::ALL;

    for (RigWellPathFormation& formation : m_formations)
    {
        RigWellPathFormation::FormationLevel level = detectLevel(formation.formationName);
        formation.level = level;
        if (level > m_maxLevelDetected)
        {
            m_maxLevelDetected = level;
        }
    }
}

struct MeasuredDepthComp
{
    bool operator()(const double& md1, const double& md2) const
    {
        if (cvf::Math::abs(md1 - md2) < 0.1)
        {
            return false;
        }
        return md1 < md2;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::measuredDepthAndFormationNamesWithoutDuplicatesOnDepth(std::vector<QString>* names,
                                                                                   std::vector<double>*  measuredDepths) const
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

struct NameAndMD
{
    NameAndMD(RigWellPathFormation::FormationLevel level, QString name, double md) : m_level(level), m_name(name), m_md(md) {}
    NameAndMD() {}
    RigWellPathFormation::FormationLevel m_level;
    QString                              m_name;
    double                               m_md;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::measuredDepthAndFormationNamesUpToLevel(RigWellPathFormation::FormationLevel level,
                                                                    std::vector<QString>*                names,
                                                                    std::vector<double>* measuredDepths, bool includeFluids) const
{
    names->clear();
    measuredDepths->clear();

    if (level == RigWellPathFormation::ALL)
    {
        measuredDepthAndFormationNamesWithoutDuplicatesOnDepth(names, measuredDepths);
        return;
    }

    std::map<double, NameAndMD, MeasuredDepthComp> tempMakeVectorUniqueOnMeasuredDepth;

    for (RigWellPathFormation formation : m_formations)
    {
        if (formation.level == RigWellPathFormation::FLUIDS)
        {
            if (includeFluids)
            {
                tempMakeVectorUniqueOnMeasuredDepth[formation.mdTop] =
                    NameAndMD(formation.level, formation.formationName + " Top", formation.mdTop);
            }
            else continue;
        }

        if (level < formation.level) continue;

        if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.mdTop) ||
            tempMakeVectorUniqueOnMeasuredDepth.at(formation.mdTop).m_level < formation.level)
        {
            tempMakeVectorUniqueOnMeasuredDepth[formation.mdTop] =
                NameAndMD(formation.level, formation.formationName + " Top", formation.mdTop);
        }
    }

    for (RigWellPathFormation formation : m_formations)
    {
        if (formation.level == RigWellPathFormation::FLUIDS)
        {
            if (includeFluids)
            {
                tempMakeVectorUniqueOnMeasuredDepth[formation.mdTop] =
                    NameAndMD(formation.level, formation.formationName + " Base", formation.mdTop);
            }
            else continue;
        }

        if (level < formation.level) continue;

        if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.mdBase) ||
            tempMakeVectorUniqueOnMeasuredDepth.at(formation.mdBase).m_level < formation.level)
        {
            tempMakeVectorUniqueOnMeasuredDepth[formation.mdBase] =
                NameAndMD(formation.level, formation.formationName + " Base", formation.mdBase);
        }
    }

    for (auto it = tempMakeVectorUniqueOnMeasuredDepth.begin(); it != tempMakeVectorUniqueOnMeasuredDepth.end(); it++)
    {
        names->push_back(it->second.m_name);
        measuredDepths->push_back(it->second.m_md);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellPathFormation::FormationLevel RigWellPathFormations::maxFormationLevel() const
{
    return m_maxLevelDetected;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPathFormation::FormationLevel RigWellPathFormations::detectLevel(QString formationName)
{
    formationName = formationName.trimmed();

    if (formationName == "OIL" || formationName == "GAS")
    {
        return RigWellPathFormation::FLUIDS;
    }

    bool isGroupName = true;
    for (QChar c : formationName)
    {
        if (c.isLower())
        {
            isGroupName = false;
            break;
        }
    }
    if (isGroupName)
    {
        return RigWellPathFormation::GROUP;
    }

    QStringList formationNameSplitted = formationName.split(" ");

    std::vector<QString> levelDesctiptorCandidates;

    for (QString word : formationNameSplitted)
    {
        for (const QChar& c : word)
        {
            if (c.isDigit())
            {
                levelDesctiptorCandidates.push_back(word);
                break;
            }
        }
    }
    if (levelDesctiptorCandidates.empty())
    {
        return RigWellPathFormation::LEVEL0;
    }

    if (levelDesctiptorCandidates.size() > 1)
    {
        for (auto it = levelDesctiptorCandidates.begin(); it != levelDesctiptorCandidates.end(); it++)
        {
            for (const QChar& c : *it)
            {
                if (c.isLetter())
                {
                    levelDesctiptorCandidates.erase(it);
                }
            }
        }
    }
    if (levelDesctiptorCandidates.size() != 1) return RigWellPathFormation::ALL;

    QString levelDescriptor = levelDesctiptorCandidates[0];

    int dotCount = levelDescriptor.count('.');

    size_t level = dotCount + 1;

    switch (dotCount)
    {
        case 0: return RigWellPathFormation::LEVEL1;
        case 1: return RigWellPathFormation::LEVEL2;
        case 2: return RigWellPathFormation::LEVEL3;
        default: break;
    }
    return RigWellPathFormation::ALL;
}

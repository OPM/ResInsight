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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPathFormations::RigWellPathFormations(const std::vector<RigWellPathFormation>& formations, const QString& filePath,
                                             const QString& key)
{
    m_filePath  = filePath;
    m_keyInFile = key;

    for (const RigWellPathFormation& formation : formations)
    {
        if (isFluid(formation.formationName))
        {
            m_fluids.push_back(formation);
        }
        else
        {
            FormationLevel level             = detectLevel(formation.formationName);
            m_formationsLevelsPresent[level] = true;

            m_formations.push_back(std::pair<RigWellPathFormation, FormationLevel>(formation, level));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::depthAndFormationNamesWithoutDuplicatesOnDepth(std::vector<QString>* names,
                                                                           std::vector<double>*  measuredDepths,
                                                                           RimWellLogPlot::DepthTypeEnum depthType) const
{
    std::map<double, bool, DepthComp> tempMakeVectorUniqueOnMeasuredDepth;

    if (depthType == RimWellLogPlot::MEASURED_DEPTH)
    {
        for (const std::pair<RigWellPathFormation, FormationLevel>& formation : m_formations)
        {
            if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.first.mdTop))
            {
                measuredDepths->push_back(formation.first.mdTop);
                names->push_back(formation.first.formationName + " Top");
                tempMakeVectorUniqueOnMeasuredDepth[formation.first.mdTop] = true;
            }
        }

        for (const std::pair<RigWellPathFormation, FormationLevel>& formation : m_formations)
        {
            if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.first.mdBase))
            {
                measuredDepths->push_back(formation.first.mdBase);
                names->push_back(formation.first.formationName + " Base");
                tempMakeVectorUniqueOnMeasuredDepth[formation.first.mdBase] = true;
            }
        }
    }
    else if (depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
    {
        for (const std::pair<RigWellPathFormation, FormationLevel>& formation : m_formations)
        {
            if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.first.tvdTop))
            {
                measuredDepths->push_back(formation.first.tvdTop);
                names->push_back(formation.first.formationName + " Top");
                tempMakeVectorUniqueOnMeasuredDepth[formation.first.tvdTop] = true;
            }
        }

        for (const std::pair<RigWellPathFormation, FormationLevel>& formation : m_formations)
        {
            if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.first.tvdBase))
            {
                measuredDepths->push_back(formation.first.tvdBase);
                names->push_back(formation.first.formationName + " Base");
                tempMakeVectorUniqueOnMeasuredDepth[formation.first.tvdBase] = true;
            }
        }
    }




    /*
    for (const std::pair<RigWellPathFormation, FormationLevel>& formation : m_formations)
    {
        if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.first.mdTop))
        {
            double depth;
            if (depthType == RimWellLogPlot::MEASURED_DEPTH)
            {
                depth = formation.first.mdTop;
            }
            else if (depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
            {
                depth = formation.first.tvdTop;
            }
            else return;

            measuredDepths->push_back(depth);
            names->push_back(formation.first.formationName + " Top");
            tempMakeVectorUniqueOnMeasuredDepth[depth] = true;
        }
    }

    for (const std::pair<RigWellPathFormation, FormationLevel>& formation : m_formations)
    {
        double depth;
        if (depthType == RimWellLogPlot::MEASURED_DEPTH)
        {
            depth = formation.first.mdBase;
        }
        else if (depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            depth = formation.first.tvdBase;
        }
        else return;

        if (!tempMakeVectorUniqueOnMeasuredDepth.count(formation.first.mdBase))
        {
            measuredDepths->push_back(depth);
            names->push_back(formation.first.formationName + " Base");
            tempMakeVectorUniqueOnMeasuredDepth[depth] = true;
        }
    }*/
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::evaluateFormationsForOnePosition(
    const std::vector<std::pair<RigWellPathFormation, FormationLevel>>& formations,
    const FormationLevel& maxLevel, const PickPosition& position,
    std::map<double, LevelAndName, DepthComp>* uniqueListMaker, RimWellLogPlot::DepthTypeEnum depthType) const
{
    QString postFix;

    if (position == TOP)
    {
        postFix = " Top";
    }
    else
    {
        postFix = " Base";
    }

    for (const std::pair<RigWellPathFormation, FormationLevel>& formation : formations)
    {
        double depth;

        if (depthType == RimWellLogPlot::MEASURED_DEPTH)
        {
            if (position == TOP)
            {
                depth = formation.first.mdTop;
            }
            else
            {
                depth = formation.first.mdBase;
            }
        }
        else if (depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            if (position == TOP)
            {
                depth = formation.first.tvdTop;
            }
            else
            {
                depth = formation.first.tvdBase;
            }
        }
        else return;

        if (formation.second > maxLevel) continue;

        if (!uniqueListMaker->count(depth) || uniqueListMaker->at(depth).level < formation.second)
        {
            (*uniqueListMaker)[depth] = LevelAndName(formation.second, formation.first.formationName + postFix);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::evaluateFormations(const std::vector<std::pair<RigWellPathFormation, FormationLevel>>& formations,
                                               const FormationLevel& maxLevel, std::vector<QString>* names,
                                               std::vector<double>* depths, RimWellLogPlot::DepthTypeEnum depthType) const
{
    std::map<double, LevelAndName, DepthComp> tempMakeVectorUniqueOnDepth;

    evaluateFormationsForOnePosition(formations, maxLevel, PickPosition::TOP, &tempMakeVectorUniqueOnDepth, depthType);
    evaluateFormationsForOnePosition(formations, maxLevel, PickPosition::BASE, &tempMakeVectorUniqueOnDepth, depthType);

    for (const std::pair<double, LevelAndName>& uniqueDepth : tempMakeVectorUniqueOnDepth)
    {
        depths->push_back(uniqueDepth.first);
        names->push_back(uniqueDepth.second.name);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::evaluateFluids(const std::vector<RigWellPathFormation>& fluidFormations, std::vector<QString>* names,
                                           std::vector<double>* depths, RimWellLogPlot::DepthTypeEnum depthType) const
{
    std::map<double, QString, DepthComp> uniqueListMaker;

    for (const RigWellPathFormation& formation : fluidFormations)
    {
        double depthBase;
        if (depthType == RimWellLogPlot::MEASURED_DEPTH)
        {
            depthBase = formation.mdBase;
        }
        else if (depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            depthBase = formation.tvdBase;
        }
        else return;

        uniqueListMaker[depthBase] = formation.formationName + " Base";
    }

    for (const RigWellPathFormation& formation : fluidFormations)
    {
        double depthTop;
        if (depthType == RimWellLogPlot::MEASURED_DEPTH)
        {
            depthTop = formation.mdTop;
        }
        else if (depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
        {
            depthTop = formation.tvdTop;
        }
        else return;

        uniqueListMaker[depthTop] = formation.formationName + " Top";
    }

    for (const std::pair<double, QString>& depthAndFormation : uniqueListMaker)
    {
        depths->push_back(depthAndFormation.first);
        names->push_back(depthAndFormation.second);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPathFormations::depthAndFormationNamesUpToLevel(FormationLevel level, std::vector<QString>* names, std::vector<double>* depths,
                                                            bool includeFluids, RimWellLogPlot::DepthTypeEnum depthType) const
{
    names->clear();
    depths->clear();

    if (includeFluids)
    {
        evaluateFluids(m_fluids, names, depths, depthType);
    }

    if (level == RigWellPathFormations::NONE)
    {
        return;
    }
    else if (level == RigWellPathFormations::ALL)
    {
        depthAndFormationNamesWithoutDuplicatesOnDepth(names, depths, depthType);
    }
    else
    {
        evaluateFormations(m_formations, level, names, depths, depthType);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellPathFormations::FormationLevel> RigWellPathFormations::formationsLevelsPresent() const
{
    std::vector<RigWellPathFormations::FormationLevel> formationLevels;

    for (const std::pair<RigWellPathFormations::FormationLevel, bool>& formationLevel : m_formationsLevelsPresent)
    {
        formationLevels.push_back(formationLevel.first);
    }
    return formationLevels;
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
    return m_formations.size() + m_fluids.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellPathFormations::isFluid(QString formationName)
{
    formationName = formationName.trimmed();

    if (formationName == "OIL" || formationName == "GAS" || formationName == "WATER")
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPathFormations::FormationLevel RigWellPathFormations::detectLevel(QString formationName)
{
    formationName = formationName.trimmed();

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
        return RigWellPathFormations::GROUP;
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
        return RigWellPathFormations::LEVEL0;
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
    if (levelDesctiptorCandidates.size() != 1) return RigWellPathFormations::UNKNOWN;

    QString levelDescriptor = levelDesctiptorCandidates[0];

    QStringList joinedLevel = levelDescriptor.split('+');
    if (joinedLevel.size() > 1)
    {
        levelDescriptor = joinedLevel[0];
    }

    int dotCount = levelDescriptor.count('.');

    size_t level = dotCount + 1;

    switch (dotCount)
    {
        case 0: return RigWellPathFormations::LEVEL1;
        case 1: return RigWellPathFormations::LEVEL2;
        case 2: return RigWellPathFormations::LEVEL3;
        case 3: return RigWellPathFormations::LEVEL4;
        case 4: return RigWellPathFormations::LEVEL5;
        case 5: return RigWellPathFormations::LEVEL6;
        case 6: return RigWellPathFormations::LEVEL7;
        case 7: return RigWellPathFormations::LEVEL8;
        case 8: return RigWellPathFormations::LEVEL9;
        case 9: return RigWellPathFormations::LEVEL10;
        default: break;
    }
    return RigWellPathFormations::UNKNOWN;
}

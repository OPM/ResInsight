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

#include "cvfBase.h"
#include "cvfObject.h"

#include <map>
#include <utility>
#include <vector>

#include <QString>

struct RigWellPathFormation
{
    double  mdTop;
    double  mdBase;
    QString formationName;
};

class RigWellPathFormations : public cvf::Object
{
public:
    RigWellPathFormations(const std::vector<RigWellPathFormation>& formations, const QString& filePath, const QString& key);

    void measuredDepthAndFormationNamesWithoutDuplicatesOnDepth(std::vector<QString>* names,
                                                                std::vector<double>*  measuredDepths) const;

    enum FormationLevel
    {
        GROUP, LEVEL0, LEVEL1, LEVEL2, LEVEL3, LEVEL4, LEVEL5, LEVEL6, LEVEL7, LEVEL8, LEVEL9, LEVEL10, ALL, FLUIDS, UNKNOWN
    };

    void measuredDepthAndFormationNamesUpToLevel(FormationLevel level, std::vector<QString>* names,
                                                 std::vector<double>* measuredDepths, bool includeFluids) const;

    std::vector<FormationLevel> formationsLevelsPresent() const;

    QString filePath() const;
    QString keyInFile() const;

    size_t  formationNamesCount() const;

private:
    FormationLevel detectLevel(QString formationName);

private:
    QString m_filePath;
    QString m_keyInFile;

    std::map<FormationLevel, bool> m_formationsLevelsPresent;

    std::vector<std::pair<RigWellPathFormation, FormationLevel>> m_formations;
};

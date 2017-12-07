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
#include <vector>

#include <QString>
#include <utility>

class RigWellPathFormations : public cvf::Object
{
public:
    RigWellPathFormations(std::vector<std::pair<double, QString>> measuredDepthAndFormationNames, const QString& filePath, const QString& key);

    void measuredDepthAndFormationNamesWithoutDuplicates(std::vector<QString>& names, std::vector<double>& measuredDepths) const;

    QString filePath() const;
    QString keyInFile() const;

    size_t formationNamesCount() const;

private:
    QString m_filePath;
    QString m_keyInFile;
    
    std::vector<std::pair<double, QString>> m_measuredDepthAndFormationNames;
};

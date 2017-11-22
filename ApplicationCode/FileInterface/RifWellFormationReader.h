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

#include <map>
#include <vector>

#include <QStringList>

struct RifWellFormation
{
    RifWellFormation(QString wellName, QString surfaceName, double measuredDepth)
        :wellName(wellName), surfaceName(surfaceName), measuredDepth(measuredDepth) {};

    QString wellName;
    QString surfaceName;
    double measuredDepth;
};

//==================================================================================================
///
//==================================================================================================
class RifWellFormationReader
{
public:
    static std::map<QString, std::vector<RifWellFormation> > readWellFormations(const QStringList& filePaths);
    static std::map<QString, std::vector<RifWellFormation> > readWellFormations(const QString& filePath);

private:
    static void readFileIntoMap(const QString& filePath, std::map<QString, std::vector<RifWellFormation> >* formations);
};

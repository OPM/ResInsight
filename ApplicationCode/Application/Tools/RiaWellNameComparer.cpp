/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "../RiaApplication.h"
#include "RiaWellNameComparer.h"
#include "../../ProjectDataModel/RimProject.h"
#include "../../ProjectDataModel/RimWellPath.h"
#include <regex>

//==================================================================================================
//
//==================================================================================================
QString  RiaWellNameComparer::tryFindMatchingSimWellName(QString searchName)
{
    RimProject* proj = RiaApplication::instance()->project();
    const std::vector<QString>& simWellNames = proj->simulationWellNames();

    if (searchName.isEmpty() || simWellNames.empty()) return QString();

    searchName = removeWellNamePrefix(searchName);
    return tryMatchNameInList(searchName, simWellNames);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::tryFindMatchingWellPath(QString wellName)
{
    RimProject* proj = RiaApplication::instance()->project();
    const std::vector<RimWellPath*>& wellPaths = proj->allWellPaths();

    if (wellName.isEmpty() || wellPaths.empty()) return QString();

    std::vector<QString> wellPathNames;
    for (const RimWellPath* wellPath : wellPaths)
    {
        wellPathNames.push_back(wellPath->name());
    }

    wellName = removeWellNamePrefix(wellName);
    return tryMatchNameInList(wellName, wellPathNames);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::tryMatchNameInList(QString searchName, const std::vector<QString>& nameList)
{
    // Try exact name match
    QString matchedName = tryMatchName(searchName, nameList);
    if (!matchedName.isEmpty())
    {
        return matchedName;
    }

    // Try matching ignoring spaces, dashes and underscores
    return tryMatchName(searchName, 
                        nameList,
                        [](const QString& str)
                        {
                            QString s = str;
                            s = removeWellNamePrefix(s);
                            return s.remove(' ').remove('-').remove('_');
                        }
                        );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::tryMatchName(QString searchName, 
                                       const std::vector<QString>& simWellNames, 
                                       std::function<QString(QString)> stringFormatter)
{
    if (searchName.isEmpty()) return QString();

    if (stringFormatter != nullptr)
    {
        searchName = stringFormatter(searchName);
    }

    for (const auto& simWellName : simWellNames)
    {
        QString simWn = simWellName;
        if (stringFormatter != nullptr)
        {
            simWn = stringFormatter(simWn);
        }
        if (QString::compare(simWn, searchName, Qt::CaseInsensitive) == 0)
        {
            return simWellName;
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaWellNameComparer::removeWellNamePrefix(const QString& name)
{
    // Try to remove prefix on the format 'xx xxxx/xx-'
    std::regex pattern("^.*\\d*[/]\\d*[-_]");

    return QString::fromStdString(std::regex_replace(name.toStdString(), pattern, ""));
}

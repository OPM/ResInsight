/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfApplicationTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include <QStringList>

#include <chrono>
#include <thread>

//--------------------------------------------------------------------------------------------------
/// Empty wellPathNames returns all well paths
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicfApplicationTools::wellPathsFromNames(const QStringList& wellPathNames, QStringList* wellsNotFound)
{
    std::vector<RimWellPath*> wellPaths;
    auto                      allWellPaths = RiaApplication::instance()->project()->allWellPaths();

    if (!wellPathNames.empty())
    {
        std::set<QString> wellPathNameSet(wellPathNames.begin(), wellPathNames.end());

        for (auto wellPath : allWellPaths)
        {
            auto matchedName = RiaWellNameComparer::tryMatchNameInList(wellPath->name(), toStringVector(wellPathNames));
            if (!matchedName.isEmpty())
            {
                wellPaths.push_back(wellPath);
                wellPathNameSet.erase(matchedName);
            }
        }

        if (wellsNotFound)
        {
            wellsNotFound->clear();
            if (!wellPathNameSet.empty())
            {
                for (const auto& wpn : wellPathNameSet)
                    wellsNotFound->push_back(wpn);
            }
        }
    }
    else
    {
        wellPaths = allWellPaths;
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicfApplicationTools::toStringVector(const QStringList& stringList)
{
    return std::vector<QString>(stringList.begin(), stringList.end());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RicfApplicationTools::toQStringList(const std::vector<QString>& v)
{
    QStringList stringList;
    for (const auto& s : v)
    {
        stringList.push_back(s);
    }
    return stringList;
}

//--------------------------------------------------------------------------------------------------
/// If caseId is -1, return first case found
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicfApplicationTools::caseFromId(int caseId)
{
    auto eclipseCases = RiaApplication::instance()->project()->eclipseCases();
    if (caseId < 0)
    {
        if (!eclipseCases.empty()) return eclipseCases.front();
    }
    else
    {
        for (RimEclipseCase* c : eclipseCases)
        {
            if (c->caseId() == caseId) return c;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicfApplicationTools::viewFromCaseIdAndViewName(int caseId, const QString& viewName)
{
    for (RimEclipseCase* c : RiaApplication::instance()->project()->eclipseCases())
    {
        if (c->caseId() == caseId)
        {
            for (auto v : c->views())
            {
                auto eclipseView = dynamic_cast<RimEclipseView*>(v);
                if (eclipseView && eclipseView->name().compare(viewName, Qt::CaseInsensitive) == 0)
                {
                    return eclipseView;
                }
            }
        }
    }
    return nullptr;
}

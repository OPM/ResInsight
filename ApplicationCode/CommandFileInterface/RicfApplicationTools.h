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

#pragma once

#include <QString>
#include <vector>

class RimEclipseCase;
class RimEclipseView;
class RimWellPath;
class QStringList;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfApplicationTools
{
public:
    static std::vector<RimWellPath*> wellPathsFromNames(const QStringList& wellPathNames,
                                                        QStringList*       wellsNotFound);
    static RimEclipseCase*           caseFromId(int caseId);
    static RimEclipseView*           viewFromCaseIdAndViewName(int caseId, const QString& viewName);

    static std::vector<QString> toStringVector(const QStringList& stringList);
    static QStringList          toQStringList(const std::vector<QString>& v);
};

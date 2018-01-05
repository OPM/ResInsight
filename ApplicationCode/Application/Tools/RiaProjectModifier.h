/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include <QString>

#include <vector>
#include <map>

class RimProject;



//==================================================================================================
//
// 
//
//==================================================================================================
class RiaProjectModifier : public cvf::Object
{
public:
    RiaProjectModifier();

    void            setReplaceCaseFirstOccurrence(QString newGridFileName);
    void            setReplaceCase(int caseIdToReplace, QString newGridFileName);

    void            setReplaceSourceCasesFirstOccurrence(std::vector<QString> newGridFileNames);
    void            setReplaceSourceCasesById(int caseGroupIdToReplace, std::vector<QString> newGridFileNames);

    void            setReplacePropertiesFolderFirstOccurrence(QString newPropertiesFolder);
    void            setReplacePropertiesFolder(int caseIdToReplace, QString newPropertiesFolder);

    bool            applyModificationsToProject(RimProject* project);

private:
    void            replaceSourceCases(RimProject* project);
    void            replaceCase(RimProject* project);
    void            replacePropertiesFolder(RimProject* project);

    static QString  makeFilePathAbsolute(QString relOrAbsolutePath);
    static QString  caseNameFromGridFileName(QString fullGridFilePathName);

    static int      firstCaseId(RimProject* project);
    static int      firstGroupId(RimProject* project);
    static int      firstInputCaseId(RimProject* project);

    static int      firstOccurrenceId();

private:
    std::map<int, QString>               m_caseIdToGridFileNameMap;
    std::map<int, std::vector<QString> > m_groupIdToGridFileNamesMap;
    std::map<int, QString>               m_caseIdToPropertiesFolderMap;
};


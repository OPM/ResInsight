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

    void            setReplaceCaseFirstOccurence(QString newGridFileName);
    void            setReplaceCase(int caseIdToReplace, QString newGridFileName);

    void            setReplaceSourceCasesFirstOccurence(std::vector<QString> newGridFileNames);
    void            setReplaceSourceCasesById(int caseGroupIdToReplace, std::vector<QString> newGridFileNames);

    bool            applyModificationsToProject(RimProject* project);

private:
    bool            replaceSourceCases(RimProject* project);
    bool            replaceCase(RimProject* project);
    static QString  makeFilePathAbsolute(QString relOrAbsolutePath);
    static QString  caseNameFromGridFileName(QString fullGridFilePathName);

private:
    int                     m_replaceCase_caseId;
    QString                 m_replaceCase_gridFileName;

    int                     m_replaceSourceCases_caseGroupId;
    std::vector<QString>    m_replaceSourceCases_gridFileNames;

    static const int UNDEFINED = -1;
    static const int FIRST_OCCURENCE = -999;
};






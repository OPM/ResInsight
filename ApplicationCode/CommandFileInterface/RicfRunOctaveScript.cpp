#include "RicfRunOctaveScript.h"
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

#include "RicfRunOctaveScript.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RicfRunOctaveScript, "runOctaveScript");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfRunOctaveScript::RicfRunOctaveScript()
{
    RICF_InitField(&m_path,     "path",    QString(),          "Path", "", "", "");
    RICF_InitField(&m_caseIds,  "caseIds", std::vector<int>(), "Case IDs", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfRunOctaveScript::execute()
{
    QString octavePath = RiaApplication::instance()->octavePath();
    QFileInfo scriptFileInfo(m_path());
    QStringList processArguments;

    processArguments << "--path" << scriptFileInfo.absolutePath();
    processArguments << scriptFileInfo.absoluteFilePath();

    bool ok;
    if (m_caseIds().empty())
    {
        ok = RiaApplication::instance()->launchProcess(octavePath, processArguments);
    }
    else
    {
        ok = RiaApplication::instance()->launchProcessForMultipleCases(octavePath, processArguments, m_caseIds());
    }
    if (!ok)
    {
        RiaLogging::error(QString("runOctaveScript: Could not execute script %1").arg(m_path()));
    }
    else
    {
        RiaApplication::instance()->waitForProcess();
    }
}

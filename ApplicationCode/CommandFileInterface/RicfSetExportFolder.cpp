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

#include "RicfSetExportFolder.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include <QDir>

CAF_PDM_SOURCE_INIT(RicfSetExportFolder, "setExportFolder");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetExportFolder::RicfSetExportFolder()
{
    // clang-format off
    RICF_InitField(&m_type,  "type",  RicfCommandFileExecutor::ExportTypeEnum(RicfCommandFileExecutor::COMPLETIONS), "Type",  "", "", "");
    RICF_InitField(&m_path,  "path",  QString(),                                                                     "Path",  "", "", "");
    RICF_InitField(&m_createFolder, "createFolder", false, "Create Folder", "", "", "");
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfSetExportFolder::execute()
{
    if (m_createFolder)
    {
        QDir dir;

        if (!dir.exists(m_path))
        {
            dir.mkdir(m_path);

            if (!dir.exists(m_path))
            {
                RiaLogging::error("Could not create folder : " + m_path);
            }
        }
    }

    RicfCommandFileExecutor* executor = RicfCommandFileExecutor::instance();
    executor->setExportPath(m_type(), m_path);
}

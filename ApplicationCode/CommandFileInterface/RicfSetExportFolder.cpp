#include "RicfSetExportFolder.h"
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

CAF_PDM_SOURCE_INIT(RicfSetExportFolder, "setExportFolder");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfSetExportFolder::RicfSetExportFolder()
{
    RICF_InitField(&m_type,  "type",  RicfCommandFileExecutor::ExportTypeEnum(RicfCommandFileExecutor::COMPLETIONS), "Type",  "", "", "");
    RICF_InitField(&m_path,  "path",  QString(),                                                                     "Path",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfSetExportFolder::execute()
{
    RicfCommandFileExecutor* executor = RicfCommandFileExecutor::instance();
    executor->setExportPath(m_type(), m_path);
}

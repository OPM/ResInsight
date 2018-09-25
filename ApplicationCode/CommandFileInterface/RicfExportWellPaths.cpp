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

#include "RicfExportWellPaths.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCreateMultipleFractures.h"

#include "ExportCommands/RicExportSelectedWellPathsFeature.h"

#include "RimProject.h"
#include "RimDialogData.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimWellPath.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "cafCmdFeatureManager.h"


CAF_PDM_SOURCE_INIT(RicfExportWellPaths, "exportWellPaths");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportWellPaths::RicfExportWellPaths()
{
    RICF_InitField(&m_wellPathNames,        "wellPathNames",        std::vector<QString>(), "Well Path Names", "", "", "");
    RICF_InitField(&m_mdStepSize,           "mdStepSize",           5.0,                    "MD Step Size",    "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportWellPaths::execute()
{
    const auto wellPaths = RicfCreateMultipleFractures::wellPaths(m_wellPathNames);
    if (!wellPaths.empty())
    {
        QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::WELLPATHS);
        if (exportFolder.isNull())
        {
            exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath("wellpaths");
        }

        caf::CmdFeatureManager*                 commandManager = caf::CmdFeatureManager::instance();
        auto feature = dynamic_cast<RicExportSelectedWellPathsFeature*>(commandManager->getCommandFeature("RicExportSelectedWellPathsFeature"));

        for (const auto wellPath : wellPaths)
        {
            if (wellPath)
            {
                feature->exportWellPath(wellPath, m_mdStepSize, exportFolder);
            }
        }
    }
}

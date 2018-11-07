/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicfExportProperty.h"

#include "../Commands/ExportCommands/RicEclipseCellResultToFileImpl.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"
#include "RicfExportPropertyInViews.h"

#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimProject.h"

#include "cafUtils.h"

#include <limits>

#include <QDir>

CAF_PDM_SOURCE_INIT(RicfExportPropertyInViews, "exportPropertyInViews");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportPropertyInViews::RicfExportPropertyInViews()
{
    RICF_InitField(&m_caseId, "caseId", -1, "Case ID", "", "", "");
    RICF_InitField(&m_viewNames, "viewNames", std::vector<QString>(), "View Names", "", "", "");
    RICF_InitField(&m_undefinedValue, "undefinedValue", 0.0, "Undefined Value", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfExportPropertyInViews::execute()
{
    using TOOLS = RicfApplicationTools;

    RimEclipseCase* eclipseCase = TOOLS::caseFromId(m_caseId());
    if (!eclipseCase)
    {
        RiaLogging::error(QString("exportProperty: Could not find case with ID %1").arg(m_caseId()));
        return;
    }

    std::vector<RimEclipseView*> viewsForExport;

    for (Rim3dView* v : eclipseCase->views())
    {
        RimEclipseView* view = dynamic_cast<RimEclipseView*>(v);
        if (!view) continue;

        if (m_viewNames().empty())
        {
            viewsForExport.push_back(view);
        }
        else
        {
            bool matchingName = false;
            for (const auto& viewName : m_viewNames())
            {
                if (view->name().compare(viewName, Qt::CaseInsensitive) == 0)
                {
                    matchingName = true;
                }
            }

            if (matchingName)
            {
                viewsForExport.push_back(view);
            }
        }
    }

    for (const auto& view : viewsForExport)
    {
        cvf::ref<RigResultAccessor> resultAccessor = nullptr;
        {
            const int mainGridIndex = 0;

            resultAccessor = RigResultAccessorFactory::createFromResultDefinition(
                eclipseCase->eclipseCaseData(), mainGridIndex, view->currentTimeStep(), view->cellResult());
        }

        const QString propertyName = view->cellResult()->resultVariableUiShortName();

        if (resultAccessor.isNull())
        {
            RiaLogging::error(QString("exportProperty: Could not find property. Case ID %1, time step %2, property '%3'")
                                  .arg(m_caseId)
                                  .arg(view->currentTimeStep())
                                  .arg(propertyName));

            continue;
        }

        QDir propertiesDir(RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::PROPERTIES));

        QString fileName = QString("%1-%2-T%3-%4")
                               .arg(eclipseCase->caseUserDescription())
                               .arg(view->name())
                               .arg(view->currentTimeStep())
                               .arg(propertyName);

        fileName               = caf::Utils::makeValidFileBasename(fileName);
        const QString filePath = propertiesDir.filePath(fileName);

        RicEclipseCellResultToFileImpl::writeResultToTextFile(filePath,
                                                              eclipseCase->eclipseCaseData(),
                                                              resultAccessor.p(),
                                                              propertyName,
                                                              m_undefinedValue,
                                                              "exportPropertiesInViews");
    }
}

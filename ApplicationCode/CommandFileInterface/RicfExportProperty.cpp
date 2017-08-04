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

#include "RicfExportProperty.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"

#include "RifEclipseInputFileTools.h"

#include <QDir>

CAF_PDM_SOURCE_INIT(RicfExportProperty, "exportProperty");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportProperty::RicfExportProperty()
{
    RICF_InitField(&m_caseId,         "caseId",         -1,        "Case ID", "", "", "");
    RICF_InitField(&m_timeStepIndex,  "timeStep",       -1,        "Time Step Index", "", "", "");
    RICF_InitField(&m_propertyName,   "property",       QString(), "Property Name", "", "", "");
    RICF_InitField(&m_eclipseKeyword, "eclipseKeyword", QString(), "Eclipse Keyword", "", "", "");
    RICF_InitField(&m_undefinedValue, "undefinedValue", 0.0,       "Undefined Value", "", "", "");
    RICF_InitField(&m_path,           "exportFile",     QString(), "Export File", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportProperty::execute()
{
    QString fileName = m_path;
    if (fileName.isNull())
    {
        QDir propertiesDir(RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::PROPERTIES));
        fileName = propertiesDir.filePath(m_propertyName);
    }

    RimEclipseCase* eclipseCase;

    {
        bool foundCase = false;
        for (RimEclipseCase* c : RiaApplication::instance()->project()->activeOilField()->analysisModels()->cases)
        {
            if (c->caseId == m_caseId)
            {
                eclipseCase = c;
                foundCase = true;
                break;
            }
        }
        if (!foundCase)
        {
            RiaLogging::error(QString("exportProperty: Could not find case with ID %1").arg(m_caseId()));
            return;
        }
    }

    // FIXME : Select correct view?
    RimEclipseView* view;
    for (RimView* v : eclipseCase->views())
    {
        view = dynamic_cast<RimEclipseView*>(v);
        if (view) break;
    }
    if (!view)
    {
        RiaLogging::error(QString("exportProperty: Could not find a view for case with ID %1").arg(m_caseId()));
        return;
    }

    if (m_eclipseKeyword().isNull())
    {
        m_eclipseKeyword = m_propertyName;
    }

    view->cellResult->setResultVariable(m_propertyName);
    view->loadDataAndUpdate();

    RifEclipseInputFileTools::writeBinaryResultToTextFile(fileName, eclipseCase->eclipseCaseData(), m_timeStepIndex, view->cellResult, m_eclipseKeyword, m_undefinedValue);
}

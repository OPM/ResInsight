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

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimProject.h"
#include "RimOilField.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"

#include "RifEclipseInputFileTools.h"

#include "cafUtils.h"

#include <QDir>

CAF_PDM_SOURCE_INIT(RicfExportProperty, "exportProperty");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfExportProperty::RicfExportProperty()
{
    RICF_InitField(&m_caseId,         "caseId",         -1,                                                                  "Case ID", "", "", "");
    RICF_InitField(&m_timeStepIndex,  "timeStep",       -1,                                                                  "Time Step Index", "", "", "");
    RICF_InitField(&m_propertyName,   "property",       QString(),                                                           "Property Name", "", "", "");
    RICF_InitField(&m_type,           "type",           caf::AppEnum<RiaDefines::ResultCatType>(RiaDefines::UNDEFINED),      "Property type", "", "", "");
    RICF_InitField(&m_eclipseKeyword, "eclipseKeyword", QString(),                                                           "Eclipse Keyword", "", "", "");
    RICF_InitField(&m_undefinedValue, "undefinedValue", 0.0,                                                                 "Undefined Value", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfExportProperty::execute()
{

    RimEclipseCase* eclipseCase = nullptr;
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

    bool fullySpecified = m_caseId >= 0 && m_timeStepIndex >= 0 && !m_propertyName().isEmpty();
    bool anyViewsFound = false;
    for (Rim3dView* v : eclipseCase->views())
    {
        RimEclipseView* view = dynamic_cast<RimEclipseView*>(v);
        if (!view) continue;
        anyViewsFound = true;

        auto timeStepIndex = m_timeStepIndex >= 0 ? m_timeStepIndex : view->currentTimeStep();
        auto propertyName = !m_propertyName().isEmpty() ? m_propertyName : view->cellResult()->resultVariable();
        auto propertyType = !m_propertyName().isEmpty() ? m_type() : view->cellResult()->resultType();

        QString filePath = m_path;
        if (filePath.isNull())
        {
            QDir propertiesDir(RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::PROPERTIES));
            QString fileName;

            if (fullySpecified) fileName = QString("%1-T%2-%3").arg(eclipseCase->caseUserDescription()).arg(timeStepIndex).arg(propertyName);
            else fileName = QString("%1-%2-T%3-%4").arg(eclipseCase->caseUserDescription()).arg(view->name()).arg(timeStepIndex).arg(propertyName);

            fileName = caf::Utils::makeValidFileBasename(fileName);
            filePath = propertiesDir.filePath(fileName);
        }

        auto eclipseKeyword = m_eclipseKeyword();
        if (m_eclipseKeyword().isNull())
        {
            eclipseKeyword = propertyName;
        }

        auto resultAccessor = findResult(view, timeStepIndex, propertyType, propertyName);
        if (!resultAccessor.isNull())
        {
            RifEclipseInputFileTools::writeResultToTextFile(filePath, eclipseCase->eclipseCaseData(), resultAccessor, eclipseKeyword, m_undefinedValue, "exportProperty");
        }
        else
        {
            RiaLogging::error(QString("exportProperty: Could not find property. Case ID %1, time step %2, property '%3'")
                .arg(m_caseId).arg(timeStepIndex).arg(propertyName));
        }

        if (fullySpecified) break;
    }

    if (!anyViewsFound)
    {
        RiaLogging::error(QString("exportProperty: Could not find any views for case ID %1").arg(m_caseId()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RicfExportProperty::findResult(RimEclipseView* view,
                                                           size_t timeStep,
                                                           RiaDefines::ResultCatType resultType,
                                                           const QString& property)
{
    auto eclipseCase = view->eclipseCase();
    size_t resultIndex = cvf::UNDEFINED_SIZE_T;

    if (resultType == RiaDefines::UNDEFINED)
    {
        resultIndex = eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(property);
    }
    else
    {
        resultIndex = eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(resultType, property);
    }

    cvf::ref<RigResultAccessor> resultAccessor = nullptr;
    if (resultIndex != cvf::UNDEFINED_SIZE_T)
    {
        resultAccessor = RigResultAccessorFactory::createFromResultIdx(eclipseCase->eclipseCaseData(),
                                                                       0,
                                                                       RiaDefines::MATRIX_MODEL,
                                                                       timeStep,
                                                                       resultIndex);
    }

    return resultAccessor;
}

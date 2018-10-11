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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "../Commands/ExportCommands/RicEclipseCellResultToFileImpl.h"
#include "RicfCommandFileExecutor.h"

#include "RifEclipseInputFileTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimProject.h"

#include "cafUtils.h"

#include <QDir>

CAF_PDM_SOURCE_INIT(RicfExportProperty, "exportProperty");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportProperty::RicfExportProperty()
{
    // clang-format off
    RICF_InitField(&m_caseId,           "caseId",           -1, "Case ID", "", "", "");
    RICF_InitField(&m_timeStepIndex,    "timeStep",         -1, "Time Step Index", "", "", "");
    RICF_InitField(&m_propertyName,     "property",         QString(), "Property Name", "", "", "");
    RICF_InitField(&m_type,             "type",             caf::AppEnum<RiaDefines::ResultCatType>(RiaDefines::DYNAMIC_NATIVE), "Property type", "", "", "");
    RICF_InitField(&m_eclipseKeyword,   "eclipseKeyword",   QString(), "Eclipse Keyword", "", "", "");
    RICF_InitField(&m_undefinedValue,   "undefinedValue",   0.0, "Undefined Value", "", "", "");
    RICF_InitField(&m_exportFileName,   "exportFile",       QString(), "Export FileName", "", "", "");
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfExportProperty::execute()
{
    RimEclipseCase* eclipseCase = nullptr;
    {
        std::vector<RimCase*> cases;
        RiaApplication::instance()->project()->allCases(cases);

        for (auto* c : cases)
        {
            RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*>(c);
            if (eclCase->caseId == m_caseId)
            {
                eclipseCase = eclCase;
                break;
            }
        }

        if (!eclipseCase)
        {
            RiaLogging::error(QString("exportProperty: Could not find case with ID %1").arg(m_caseId()));
            return;
        }

        if (!eclipseCase->eclipseCaseData())
        {
            if (!eclipseCase->openReserviorCase())
            {
                RiaLogging::error(QString("exportProperty: Could not find eclipseCaseData with ID %1").arg(m_caseId()));
                return;
            }
        }
    }

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    RigCaseCellResultsData* cellResultsData = eclipseCaseData->results(RiaDefines::MATRIX_MODEL);

    size_t resultIdx = cellResultsData->findOrLoadScalarResult(m_propertyName);
    if (resultIdx == cvf::UNDEFINED_SIZE_T)
    {
        RiaLogging::error(QString("exportProperty: Could not find result property : %1").arg(m_propertyName()));
        return;
    }

    QString filePath = m_exportFileName;
    if (filePath.isNull())
    {
        QDir    propertiesDir(RicfCommandFileExecutor::instance()->getExportPath(RicfCommandFileExecutor::PROPERTIES));
        QString fileName = QString("%1-%2").arg(eclipseCase->caseUserDescription()).arg(m_propertyName);
        fileName         = caf::Utils::makeValidFileBasename(fileName);
        filePath         = propertiesDir.filePath(fileName);
    }

    QString eclipseKeyword = m_eclipseKeyword;
    if (eclipseKeyword.isNull())
    {
        eclipseKeyword = m_propertyName;
    }

    RicEclipseCellResultToFileImpl::writePropertyToTextFile(
        filePath, eclipseCase->eclipseCaseData(), m_timeStepIndex, m_propertyName, eclipseKeyword, m_undefinedValue);
}

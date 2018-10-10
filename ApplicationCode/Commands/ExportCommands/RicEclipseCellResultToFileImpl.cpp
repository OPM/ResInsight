/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicEclipseCellResultToFileImpl.h"

#include "RiaLogging.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseResultDefinition.h"

#include "cafProgressInfo.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseCellResultToFileImpl::writePropertyToTextFile(const QString&      fileName,
                                                             RigEclipseCaseData* eclipseCase,
                                                             size_t              timeStep,
                                                             const QString&      resultName,
                                                             const QString&      eclipseKeyword)
{
    CVF_TIGHT_ASSERT(eclipseCase);
    if (!eclipseCase) return false;

    cvf::ref<RigResultAccessor> resultAccessor =
        RigResultAccessorFactory::createFromUiResultName(eclipseCase, 0, RiaDefines::MATRIX_MODEL, timeStep, resultName);
    if (resultAccessor.isNull())
    {
        return false;
    }

    const double undefinedValue = 0.0;

    return writeResultToTextFile(
        fileName, eclipseCase, resultAccessor.p(), eclipseKeyword, undefinedValue, "writePropertyToTextFile");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseCellResultToFileImpl::writeBinaryResultToTextFile(const QString&              fileName,
                                                                 RigEclipseCaseData*         eclipseCase,
                                                                 size_t                      timeStep,
                                                                 RimEclipseResultDefinition* resultDefinition,
                                                                 const QString&              eclipseKeyword,
                                                                 const double                undefinedValue,
                                                                 const QString&              logPrefix)
{
    CVF_TIGHT_ASSERT(eclipseCase);

    cvf::ref<RigResultAccessor> resultAccessor =
        RigResultAccessorFactory::createFromResultDefinition(eclipseCase, 0, timeStep, resultDefinition);
    if (resultAccessor.isNull())
    {
        return false;
    }

    return writeResultToTextFile(fileName, eclipseCase, resultAccessor.p(), eclipseKeyword, undefinedValue, logPrefix);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEclipseCellResultToFileImpl::writeResultToTextFile(const QString&      fileName,
                                                           RigEclipseCaseData* eclipseCase,
                                                           RigResultAccessor*  resultAccessor,
                                                           const QString&      eclipseKeyword,
                                                           const double        undefinedValue,
                                                           const QString&      logPrefix)
{
    if (!resultAccessor)
    {
        RiaLogging::error(logPrefix + QString(" : : Could not access result data for '%1'").arg(fileName));
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        RiaLogging::error(logPrefix + QString(" : Could not open file '%1'. Do the folder exist?").arg(fileName));
        return false;
    }

    std::vector<double> resultData;
    for (size_t i = 0; i < eclipseCase->mainGrid()->cellCount(); i++)
    {
        double resultValue = resultAccessor->cellScalar(i);
        if (resultValue == HUGE_VAL)
        {
            resultValue = undefinedValue;
        }

        resultData.push_back(resultValue);
    }

    writeDataToTextFile(&file, eclipseKeyword, resultData);

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEclipseCellResultToFileImpl::writeDataToTextFile(QFile*                     file,
                                                         const QString&             eclipseKeyword,
                                                         const std::vector<double>& resultData)
{
    QTextStream textstream(file);
    textstream << "\n";
    textstream << "-- Exported from ResInsight"
               << "\n";
    textstream << eclipseKeyword << "\n" << right << qSetFieldWidth(16);

    caf::ProgressInfo pi(resultData.size(), QString("Writing data to file %1").arg(file->fileName()));
    size_t            progressSteps = resultData.size() / 20;

    size_t i;
    for (i = 0; i < resultData.size(); i++)
    {
        textstream << resultData[i];

        if ((i + 1) % 5 == 0)
        {
            textstream << "\n";
        }

        if (i % progressSteps == 0)
        {
            pi.setProgress(i);
        }
    }

    textstream << "\n"
               << "/"
               << "\n";
}

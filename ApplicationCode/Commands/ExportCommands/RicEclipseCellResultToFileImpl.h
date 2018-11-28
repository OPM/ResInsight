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

#pragma once

#include <QString>

#include <vector>

class QFile;

class RigEclipseCaseData;
class RimEclipseResultDefinition;
class RigResultAccessor;

//==================================================================================================
///
//==================================================================================================
class RicEclipseCellResultToFileImpl
{
public:
    static bool writePropertyToTextFile(const QString&      fileName,
                                        RigEclipseCaseData* eclipseCase,
                                        size_t              timeStep,
                                        const QString&      resultName,
                                        const QString&      eclipseKeyword,
                                        const double        undefinedValue);

    static bool writeBinaryResultToTextFile(const QString&              fileName,
                                            RigEclipseCaseData*         eclipseCase,
                                            size_t                      timeStep,
                                            RimEclipseResultDefinition* resultDefinition,
                                            const QString&              eclipseKeyword,
                                            const double                undefinedValue,
                                            const QString&              logPrefix);

    static bool writeResultToTextFile(const QString&      fileName,
                                      RigEclipseCaseData* eclipseCase,
                                      RigResultAccessor*  resultAccessor,
                                      const QString&      eclipseKeyword,
                                      const double        undefinedValue,
                                      const QString&      logPrefix);

    static void writeDataToTextFile(QFile* file, const QString& eclipseKeyword, const std::vector<double>& resultData);
};

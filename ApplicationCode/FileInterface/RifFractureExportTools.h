/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfLibCore.h"

#include "ert/ecl/ecl_kw.h"

#include <map>
#include <QString>


class QFile;
class QTextStream;
class RigFracturedEclipseCellExportData;
class RigMainGrid;
class RimEclipseCase;
class RimEclipseWell;
class RimFracture;
class RimFracture;
class RimWellPath;

//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifFractureExportTools 
{
public:
    RifFractureExportTools();
    virtual ~RifFractureExportTools();

    static bool exportFracturesToEclipseDataInputFile(const QString& fileName, 
                                                      const std::vector<RimFracture*>& fractures, 
                                                      RimEclipseCase* caseToApply);

    static void exportWellPathFracturesToEclipseDataInputFile(const QString& fileName,
                                                              const RimWellPath* wellPath,
                                                              const RimEclipseCase* caseToApply);


private:

    static void printCOMPDATvalues(QTextStream & out, 
                                   double transmissibility,
                                   size_t i, size_t j, size_t k, 
                                   const QString& fractureName, 
                                   double skinFactor, 
                                   const QString& wellName);

    static void printStimPlanCellsMatrixTransContributions(const std::vector<RimFracture *>& fractures, 
                                                           RimEclipseCase* caseToApply, 
                                                           QTextStream &out, 
                                                           const QString& wellName,
                                                           const RigMainGrid* mainGrid);
    static void printStimPlanFractureTrans(const std::vector<RimFracture *>& fractures, 
                                           RimEclipseCase* caseToApply, 
                                           QTextStream &out);
    static void printTransmissibilityFractureToWell(const std::vector<RimFracture *>& fractures, 
                                                    QTextStream &out, 
                                                    RimEclipseCase* caseToApply);


    static void printBackgroundDataHeaderLine(QTextStream & out);

    static void printBackgroundData(QTextStream & out, 
                                    const QString& wellName, 
                                    RimFracture* fracture, 
                                    const RigMainGrid* mainGrid, 
                                    RigFracturedEclipseCellExportData &fracData);
};

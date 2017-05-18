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

    static bool writeFracturesToTextFile(const QString& fileName, const std::vector<RimFracture*>& fractures, RimEclipseCase* caseToApply);

private:
    static void performStimPlanUpscalingAndPrintResults(const std::vector<RimFracture *>& fractures, RimEclipseCase* caseToApply, QTextStream &out, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid);
    static void printStimPlanCellsMatrixTransContributions(const std::vector<RimFracture *>& fractures, RimEclipseCase* caseToApply, QTextStream &out, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid);
    static void printStimPlanFractureTrans(const std::vector<RimFracture *>& fractures, QTextStream &out);
    static void printTransmissibilityFractureToWell(const std::vector<RimFracture *>& fractures, QTextStream &out, RimEclipseCase* caseToApply);


    static void printCOMPDATvalues(QTextStream & out, RigFracturedEclipseCellExportData &fracData, RimFracture* fracture, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid);

    static void printBackgroundDataHeaderLine(QTextStream & out);

    static void printBackgroundData(QTextStream & out, RimWellPath* wellPath, RimEclipseWell* simWell, RimFracture* fracture, const RigMainGrid* mainGrid, RigFracturedEclipseCellExportData &fracData);
};

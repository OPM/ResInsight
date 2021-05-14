/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
#include <set>
#include <vector>

namespace RiaResultNames
{
bool isPerCellFaceResult( const QString& resultName );

QString undefinedResultName();
QString undefinedGridFaultName();
QString undefinedGridFaultWithInactiveName();
QString combinedTransmissibilityResultName();
QString combinedWaterFluxResultName();
QString combinedOilFluxResultName();
QString combinedGasFluxResultName();

QString ternarySaturationResultName();
QString combinedMultResultName();

QString eqlnumResultName();

QString riTranXResultName();
QString riTranYResultName();
QString riTranZResultName();
QString combinedRiTranResultName();

QString riMultXResultName();
QString riMultYResultName();
QString riMultZResultName();
QString combinedRiMultResultName();

QString riAreaNormTranXResultName();
QString riAreaNormTranYResultName();
QString riAreaNormTranZResultName();
QString combinedRiAreaNormTranResultName();

QString riCellVolumeResultName();
QString riOilVolumeResultName();
QString mobilePoreVolumeName();

QString faultReactAssessmentPrefix();

QString completionTypeResultName();

// Well path derived results
QString wbsAzimuthResult();
QString wbsInclinationResult();
QString wbsPPResult();
QString wbsSHResult();
QString wbsSHMkResult();
QString wbsOBGResult();
QString wbsFGResult();
QString wbsSFGResult();

// Fault results
QString           formationBinaryAllanResultName();
QString           formationAllanResultName();
std::set<QString> nncResultNames();

// List of well path derived results
std::vector<QString> wbsAngleResultNames();
std::vector<QString> wbsDerivedResultNames();

QString activeFormationNamesResultName();

}; // namespace RiaResultNames

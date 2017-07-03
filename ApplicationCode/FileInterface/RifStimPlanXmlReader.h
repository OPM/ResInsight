/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RiaEclipseUnitTools.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include <QString>

#include <vector>

class RigStimPlanFractureDefinition;
class QXmlStreamReader;

class RifStimPlanXmlReader
{
public:
    static cvf::ref<RigStimPlanFractureDefinition> readStimPlanXMLFile(const QString& stimPlanFileName, QString * errorMessage);

private:
    static size_t                           readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream, RigStimPlanFractureDefinition* stimPlanFileData, RiaEclipseUnitTools::UnitSystemType& unit);

    static double                           getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName);
    static QString                          getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName);
    static void                             getGriddingValues(QXmlStreamReader &xmlStream, std::vector<double>& gridValues, size_t& startNegValues);

    static std::vector<std::vector<double>> getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream, size_t startingNegValuesXs);

};



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
    enum MirrorMode { MIRROR_OFF = 0, MIRROR_ON = 1, MIRROR_AUTO = 2};

    static cvf::ref<RigStimPlanFractureDefinition> readStimPlanXMLFile(const QString& stimPlanFileName,
                                                                       double conductivityScalingFactor,
                                                                       MirrorMode mirrorMode,
                                                                       RiaEclipseUnitTools::UnitSystem requiredUnit,
                                                                       QString * errorMessage);

private:
    static void                             readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream,
                                                                         RigStimPlanFractureDefinition* stimPlanFileData,
                                                                         MirrorMode mirrorMode,
                                                                         RiaEclipseUnitTools::UnitSystem requiredUnit);

    static double                           getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName);
    static QString                          getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName);
    static void                             getGriddingValues(QXmlStreamReader &xmlStream, std::vector<double>& gridValues, size_t& startNegValues);

    static std::vector<std::vector<double>> getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream);

    static std::vector<double>              valuesInRequiredUnitSystem(RiaEclipseUnitTools::UnitSystem sourceUnit,
                                                                       RiaEclipseUnitTools::UnitSystem requiredUnit,
                                                                       const std::vector<double>&      values);

    static double                           valueInRequiredUnitSystem(RiaEclipseUnitTools::UnitSystem sourceUnit,
                                                                      RiaEclipseUnitTools::UnitSystem requiredUnit,
                                                                      double                          value);
};

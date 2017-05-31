/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemResultPosEnum.h"
#include <string>

//==================================================================================================
/// 
//==================================================================================================

class RigFemResultAddress
{
public:
    RigFemResultAddress(RigFemResultPosEnum resPosType,
                        const std::string& aFieldName,
                        const std::string& aComponentName)
        : resultPosType(resPosType),
        fieldName(aFieldName),
        componentName(aComponentName),
        timeLapseBaseFrameIdx(-1)
    {
    }

    RigFemResultAddress(RigFemResultPosEnum resPosType,
                        const std::string& aFieldName,
                        const std::string& aComponentName,
                        int aTimeLapseBaseFrame)
        : resultPosType(resPosType),
        fieldName(aFieldName),
        componentName(aComponentName),
        timeLapseBaseFrameIdx(aTimeLapseBaseFrame)
    {
    }

    RigFemResultPosEnum resultPosType;
    std::string         fieldName;
    std::string         componentName;
    int                 timeLapseBaseFrameIdx;

    static const int ALL_TIME_LAPSES = -2;

    bool isTimeLapse() const { return timeLapseBaseFrameIdx >= 0;}
    bool representsAllTimeLapses() const { return timeLapseBaseFrameIdx == ALL_TIME_LAPSES;}

    bool isValid() const
    {
        bool isTypeValid =     resultPosType == RIG_NODAL 
                            || resultPosType == RIG_ELEMENT_NODAL 
                            || resultPosType == RIG_INTEGRATION_POINT
                            || resultPosType == RIG_ELEMENT_NODAL_FACE
                            || resultPosType == RIG_FORMATION_NAMES;
        bool isFieldValid = fieldName != "";

        return isTypeValid && isFieldValid;
    }

    bool operator< (const RigFemResultAddress& other ) const
    {
        if (timeLapseBaseFrameIdx != other.timeLapseBaseFrameIdx)
        {
            return (timeLapseBaseFrameIdx < other.timeLapseBaseFrameIdx);
        }

        if (resultPosType != other.resultPosType)
        {
            return (resultPosType < other.resultPosType);
        }

        if (fieldName != other.fieldName)
        {
            return (fieldName <  other.fieldName);
        }
 
        return (componentName <  other.componentName);
    }
};



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
    RigFemResultAddress()
        : resultPosType( RIG_NODAL )
        , fieldName( "" )
        , componentName( "" )
        , timeLapseBaseStepIdx( NO_TIME_LAPSE )
        , refKLayerIndex( NO_COMPACTION )
        , normalizedByHydrostaticPressure( false )
    {
    }

    RigFemResultAddress( RigFemResultPosEnum resPosType,
                         const std::string&  aFieldName,
                         const std::string&  aComponentName,
                         int                 timeLapseBaseStepIdx            = NO_TIME_LAPSE,
                         int                 refKLayerIndex                  = NO_COMPACTION,
                         bool                normalizedByHydrostaticPressure = false )
        : resultPosType( resPosType )
        , fieldName( aFieldName )
        , componentName( aComponentName )
        , timeLapseBaseStepIdx( timeLapseBaseStepIdx )
        , refKLayerIndex( refKLayerIndex )
        , normalizedByHydrostaticPressure( normalizedByHydrostaticPressure )
    {
    }

    RigFemResultAddress( const RigFemResultAddress& rhs )
        : resultPosType( rhs.resultPosType )
        , fieldName( rhs.fieldName )
        , componentName( rhs.componentName )
        , timeLapseBaseStepIdx( rhs.timeLapseBaseStepIdx )
        , refKLayerIndex( rhs.refKLayerIndex )
        , normalizedByHydrostaticPressure( rhs.normalizedByHydrostaticPressure )
    {
    }

    RigFemResultAddress& operator=( const RigFemResultAddress& rhs )
    {
        resultPosType                   = rhs.resultPosType;
        fieldName                       = rhs.fieldName;
        componentName                   = rhs.componentName;
        timeLapseBaseStepIdx            = rhs.timeLapseBaseStepIdx;
        refKLayerIndex                  = rhs.refKLayerIndex;
        normalizedByHydrostaticPressure = rhs.normalizedByHydrostaticPressure;
        return *this;
    }

    RigFemResultPosEnum resultPosType;
    std::string         fieldName;
    std::string         componentName;
    int                 timeLapseBaseStepIdx;
    int                 refKLayerIndex;
    bool                normalizedByHydrostaticPressure;

    RigFemResultAddress copyWithComponent( const std::string& componentName ) const
    {
        auto copy          = *this;
        copy.componentName = componentName;
        return copy;
    }

    static constexpr int allTimeLapsesValue() { return ALL_TIME_LAPSES; }
    static constexpr int noTimeLapseValue() { return NO_TIME_LAPSE; }
    static constexpr int noCompactionValue() { return NO_COMPACTION; }

    bool isTimeLapse() const { return timeLapseBaseStepIdx > NO_TIME_LAPSE; }
    bool representsAllTimeLapses() const { return timeLapseBaseStepIdx == ALL_TIME_LAPSES; }
    bool normalizeByHydrostaticPressure() const { return normalizedByHydrostaticPressure; }

    bool isValid() const
    {
        bool isTypeValid = resultPosType == RIG_NODAL || resultPosType == RIG_ELEMENT_NODAL ||
                           resultPosType == RIG_INTEGRATION_POINT || resultPosType == RIG_ELEMENT_NODAL_FACE ||
                           resultPosType == RIG_FORMATION_NAMES || resultPosType == RIG_ELEMENT ||
                           resultPosType == RIG_DIFFERENTIALS;
        bool isFieldValid = fieldName != "";

        return isTypeValid && isFieldValid;
    }

    bool operator<( const RigFemResultAddress& other ) const
    {
        if ( normalizedByHydrostaticPressure != other.normalizedByHydrostaticPressure )
        {
            return ( normalizedByHydrostaticPressure < other.normalizedByHydrostaticPressure );
        }

        if ( timeLapseBaseStepIdx != other.timeLapseBaseStepIdx )
        {
            return ( timeLapseBaseStepIdx < other.timeLapseBaseStepIdx );
        }

        if ( resultPosType != other.resultPosType )
        {
            return ( resultPosType < other.resultPosType );
        }

        if ( fieldName != other.fieldName )
        {
            return ( fieldName < other.fieldName );
        }

        if ( refKLayerIndex != other.refKLayerIndex )
        {
            return refKLayerIndex < other.refKLayerIndex;
        }

        return ( componentName < other.componentName );
    }

    bool operator==( const RigFemResultAddress& other ) const
    {
        if ( resultPosType != other.resultPosType || fieldName != other.fieldName ||
             componentName != other.componentName || timeLapseBaseStepIdx != other.timeLapseBaseStepIdx ||
             normalizedByHydrostaticPressure != other.normalizedByHydrostaticPressure )
        {
            return false;
        }

        return true;
    }

private:
    static const int ALL_TIME_LAPSES = -2;
    static const int NO_TIME_LAPSE   = -1;
    static const int NO_COMPACTION   = -1;
};

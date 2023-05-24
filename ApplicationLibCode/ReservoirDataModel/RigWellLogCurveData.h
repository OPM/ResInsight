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

#include "RiaDefines.h"
#include "RiaWellLogUnitTools.h"

#include "cvfObject.h"

#include <QString>

#include <map>
#include <set>
#include <vector>

class RigWellLogCurveDataTestInterface;

//==================================================================================================
///
//==================================================================================================
class RigWellLogCurveData : public cvf::Object
{
public:
    RigWellLogCurveData();
    ~RigWellLogCurveData() override;

    void clear();

    void setDepthUnit( RiaDefines::DepthUnitType depthUnit );

    void setValuesAndDepths( const std::vector<double>& propertyValues,
                             const std::vector<double>& depths,
                             RiaDefines::DepthTypeEnum  depthType,
                             double                     rkbDiff,
                             RiaDefines::DepthUnitType  depthUnit,
                             bool                       isExtractionCurve,
                             bool                       useLogarithmicScale );

    void setValuesAndDepths( const std::vector<double>&                                      propertyValues,
                             const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& depths,
                             double                                                          rkbDiff,
                             RiaDefines::DepthUnitType                                       depthUnit,
                             bool                                                            isExtractionCurve,
                             bool                                                            useLogarithmicScale );

    void setPropertyValueUnit( const QString& propertyValueUnitString );

    std::vector<double> propertyValues() const;
    std::vector<double> propertyValues( const QString& units ) const;
    QString             propertyValueUnit() const;

    std::vector<double> depths( RiaDefines::DepthTypeEnum depthType ) const;
    std::vector<double> depths( RiaDefines::DepthTypeEnum depthType, RiaDefines::DepthUnitType destinationDepthUnit ) const;

    std::set<RiaDefines::DepthTypeEnum> availableDepthTypes() const;

    bool calculateDepthRange( RiaDefines::DepthTypeEnum depthType, RiaDefines::DepthUnitType depthUnit, double* minMD, double* maxMD ) const;

    RiaDefines::DepthUnitType depthUnit() const;

    std::vector<double> propertyValuesByIntervals() const;
    std::vector<double> depthValuesByIntervals( RiaDefines::DepthTypeEnum depthType, RiaDefines::DepthUnitType destinationDepthUnit ) const;
    std::vector<std::pair<size_t, size_t>> polylineStartStopIndices() const;

    cvf::ref<RigWellLogCurveData> calculateResampledCurveData( double newMeasuredDepthStepSize ) const;
    cvf::ref<RigWellLogCurveData> calculateResampledCurveData( RiaDefines::DepthTypeEnum  resamplingDepthType,
                                                               const std::vector<double>& depths ) const;
    static void                   interpolateSegment( RiaDefines::DepthTypeEnum                                       resamplingDepthType,
                                                      std::vector<double>&                                            resampledValues,
                                                      std::map<RiaDefines::DepthTypeEnum, std::vector<double>>&       resampledDepths,
                                                      double                                                          targetDepthValue,
                                                      size_t                                                          firstIndex,
                                                      const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& originalDepths,
                                                      const std::vector<double>&                                      propertyValues,
                                                      double                                                          eps );

    static std::pair<std::vector<double>, std::map<RiaDefines::DepthTypeEnum, std::vector<double>>>
        createResampledValuesAndDepths( RiaDefines::DepthTypeEnum                                       resamplingDepthType,
                                        const std::vector<double>&                                      targetDepths,
                                        const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& originalDepths,
                                        const std::vector<double>&                                      propertyValues );

private:
    void calculateIntervalsOfContinousValidValues();

    static std::vector<double> depthsForDepthUnit( const std::vector<double>& depths,
                                                   RiaDefines::DepthUnitType  sourceDepthUnit,
                                                   RiaDefines::DepthUnitType  destinationDepthUnit );

    static void splitIntervalAtEmptySpace( const std::vector<double>&              depthValues,
                                           size_t                                  startIdx,
                                           size_t                                  stopIdx,
                                           std::vector<std::pair<size_t, size_t>>* intervals );

private:
    std::vector<double>                                      m_propertyValues;
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> m_depths;
    bool                                                     m_isExtractionCurve;
    double                                                   m_rkbDiff;
    bool                                                     m_useLogarithmicScale;

    std::vector<std::pair<size_t, size_t>> m_intervalsOfContinousValidValues;

    RiaDefines::DepthUnitType m_depthUnit;
    QString                   m_propertyValueUnitString;
};

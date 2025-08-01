/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RifReaderEclipseRft.h"

#include "RiaCurveMerger.h"
#include "RiaWeightedMeanCalculator.h"

class RimSummaryEnsemble;
class RimEclipseCase;
class RigWellPath;

class RifReaderEnsembleStatisticsRft : public RifReaderRftInterface
{
public:
    RifReaderEnsembleStatisticsRft( const RimSummaryEnsemble* summaryCaseCollection, RimEclipseCase* eclipseCase );

    std::set<RifEclipseRftAddress> eclipseRftAddresses() override;
    void                           values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) override;

    std::set<QDateTime>                                   availableTimeSteps( const QString& wellName ) override;
    std::set<QDateTime>                                   availableTimeSteps( const QString&                                     wellName,
                                                                              const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) override;
    std::set<QDateTime>                                   availableTimeSteps( const QString&                                               wellName,
                                                                              const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) override;
    std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) override;
    std::set<QString>                                     wellNames() override;

private:
    void calculateStatistics( const QString& wellName, const QDateTime& timeStep );

    void extractStatisticsFromCurveMerger( const QString&                     wellName,
                                           const QDateTime&                   timeStep,
                                           RifEclipseRftAddress               depthAddress,
                                           RiaCurveMerger<double>&            curveMerger,
                                           RiaWeightedMeanCalculator<size_t>& dataSetSizeCalc,
                                           const RigWellPath*                 wellPathGeometry );

    void clearCache( const QString& wellName, const QDateTime& timeStep );

private:
    const RimSummaryEnsemble* m_summaryCaseCollection;
    RimEclipseCase*           m_eclipseCase;

    std::map<RifEclipseRftAddress, std::vector<double>> m_cachedValues;
};

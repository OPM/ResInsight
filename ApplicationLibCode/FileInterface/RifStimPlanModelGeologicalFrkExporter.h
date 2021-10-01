/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-    Equinor ASA
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

#include <map>
#include <vector>

class RimStimPlanModel;
class QString;
class QTextStream;

//==================================================================================================
//
//==================================================================================================
class RifStimPlanModelGeologicalFrkExporter
{
public:
    static const int        MAX_STIMPLAN_LAYERS     = 100;
    static constexpr double MIN_STRESS_GRADIENT     = 0.3;
    static constexpr double MAX_STRESS_GRADIENT     = 0.8;
    static constexpr double DEFAULT_STRESS_GRADIENT = 0.7;

    static bool writeToFile( RimStimPlanModel* plot, bool useDetailedFluidLoss, const QString& filepath );

private:
    static bool writeToFrkFile( const QString&                                filepath,
                                const std::vector<QString>&                   labels,
                                const std::map<QString, std::vector<double>>& values );
    static bool writeToCsvFile( const QString&                                filepath,
                                const std::vector<QString>&                   labels,
                                const std::map<QString, std::vector<double>>& values,
                                const std::vector<QString>&                   faciesNames,
                                const std::vector<QString>&                   formationNames );

    static void appendHeaderToStream( QTextStream& stream );
    static void appendToStream( QTextStream& stream, const QString& label, const std::vector<double>& values );
    static void appendFooterToStream( QTextStream& stream );

    static void fixupStressGradients( std::vector<double>& stressGradients,
                                      double               minStressGradient,
                                      double               maxStressGradient,
                                      double               defaultStressGradient );
    static void fixupLowerBoundary( std::vector<double>& values, double minValue, const QString& property );
    static std::pair<std::vector<double>, std::vector<double>> createDepthRanges( const std::vector<double>& tvd );

    static bool warnOnInvalidData( const QString& label, const std::vector<double>& values );
    static bool hasInvalidData( const std::vector<double>& values );

    static std::vector<double> createPerforationValues( const std::vector<double>& depthStart,
                                                        const std::vector<double>& depthEnd,
                                                        double                     perforationTop,
                                                        double                     perforationBottom );
};

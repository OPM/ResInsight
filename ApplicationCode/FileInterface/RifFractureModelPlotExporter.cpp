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

#include "RifFractureModelPlotExporter.h"

#include "RimFractureModelPlot.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureModelPlotExporter::writeToFile( RimFractureModelPlot* plot, bool useDetailedLoss, const QString& filepath )
{
    std::vector<QString> labels;
    // TVD depth of top of zone (ft)
    labels.push_back( "dpthlyr" );

    // Stress at top of zone (psi)
    labels.push_back( "strs" );

    // Stress gradient (psi/ft)
    labels.push_back( "strsg" );

    // Young's modulus (MMpsi)
    labels.push_back( "elyr" );

    // Poisson's Ratio
    labels.push_back( "poissonr" );

    // K-Ic (psi*sqrt(in)
    labels.push_back( "tuflyr" );

    // Fluid Loss Coefficient
    labels.push_back( "clyrc" );

    // Spurt loss (gal/100f^2)
    labels.push_back( "clyrs" );

    // Proppand Embedmeent (lb/ft^2)
    labels.push_back( "pembed" );

    if ( useDetailedLoss )
    {
        // B2 Detailed Loss
        // Reservoir Pressure (psi)
        labels.push_back( "zoneResPres" );

        // Immobile Fluid Saturation (fraction)
        labels.push_back( "zoneWaterSat" );

        // Porosity (fraction)
        labels.push_back( "zonePorosity" );

        // Horizontal Perm (md)
        labels.push_back( "zoneHorizPerm" );

        // Vertical Perm (md)
        labels.push_back( "zoneVertPerm" );

        // Temperature (F)
        labels.push_back( "zoneTemp" );

        // Relative permeability
        labels.push_back( "zoneRelPerm" );

        // Poro-Elastic constant
        labels.push_back( "zonePoroElas" );

        // Thermal Epansion Coefficient (1/F)
        labels.push_back( "zoneThermalExp" );
    }

    std::map<QString, std::vector<double>> values;
    values["dpthlyr"]        = plot->calculateTrueVerticalDepth();
    values["strs"]           = plot->calculateStress();
    values["strsg"]          = plot->calculateStressGradient();
    values["elyr"]           = plot->calculateYoungsModulus();
    values["poissonr"]       = plot->calculatePoissonsRatio();
    values["tuflyr"]         = plot->calculateKIc();
    values["clyrc"]          = plot->calculateFluidLossCoefficient();
    values["clyrs"]          = plot->calculateSpurtLoss();
    values["pembed"]         = plot->calculateProppandEmbedment();
    values["zoneResPres"]    = plot->calculateReservoirPressure();
    values["zoneWaterSat"]   = plot->calculateImmobileFluidSaturation();
    values["zonePorosity"]   = plot->calculatePorosity();
    values["zoneHorizPerm"]  = plot->calculateHorizontalPermeability();
    values["zoneVertPerm"]   = plot->calculateVerticalPermeability();
    values["zoneTemp"]       = plot->calculateTemperature();
    values["zoneRelPerm"]    = plot->calculateRelativePermeabilityFactor();
    values["zonePoroElas"]   = plot->calculatePoroElasticConstant();
    values["zoneThermalExp"] = plot->calculateThermalExpansionCoefficient();

    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );

    for ( QString label : labels )
    {
        appendToStream( stream, label, values[label] );
    }

    return true;
}

void RifFractureModelPlotExporter::appendToStream( QTextStream& stream, const QString& label, const std::vector<double>& values )
{
    stream << "<cNamedSet>"
           << "\n"
           << label << "\n"
           << "<dimCount>"
           << "\n"
           << 1 << "\n"
           << "<sizes>"
           << "\n"
           << values.size() << "\n"
           << "<data>"
           << "\n";
    for ( auto val : values )
    {
        stream << val << "\n";
    }

    stream << "</cNamedSet>"
           << "\n";
}

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

#include "RifFractureModelGeologicalFrkExporter.h"

#include "RimFractureModel.h"
#include "RimFractureModelCalculator.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureModelGeologicalFrkExporter::writeToFile( RimFractureModel* fractureModel,
                                                         bool              useDetailedLoss,
                                                         const QString&    filepath )
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
    values["dpthlyr"]        = fractureModel->calculator()->calculateTrueVerticalDepth();
    values["strs"]           = fractureModel->calculator()->calculateStress();
    values["strsg"]          = fractureModel->calculator()->calculateStressGradient();
    values["elyr"]           = fractureModel->calculator()->calculateYoungsModulus();
    values["poissonr"]       = fractureModel->calculator()->calculatePoissonsRatio();
    values["tuflyr"]         = fractureModel->calculator()->calculateKIc();
    values["clyrc"]          = fractureModel->calculator()->calculateFluidLossCoefficient();
    values["clyrs"]          = fractureModel->calculator()->calculateSpurtLoss();
    values["pembed"]         = fractureModel->calculator()->calculateProppandEmbedment();
    values["zoneResPres"]    = fractureModel->calculator()->calculateReservoirPressure();
    values["zoneWaterSat"]   = fractureModel->calculator()->calculateImmobileFluidSaturation();
    values["zonePorosity"]   = fractureModel->calculator()->calculatePorosity();
    values["zoneHorizPerm"]  = fractureModel->calculator()->calculateHorizontalPermeability();
    values["zoneVertPerm"]   = fractureModel->calculator()->calculateVerticalPermeability();
    values["zoneTemp"]       = fractureModel->calculator()->calculateTemperature();
    values["zoneRelPerm"]    = fractureModel->calculator()->calculateRelativePermeabilityFactor();
    values["zonePoroElas"]   = fractureModel->calculator()->calculatePoroElasticConstant();
    values["zoneThermalExp"] = fractureModel->calculator()->calculateThermalExpansionCoefficient();

    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );

    for ( QString label : labels )
    {
        appendToStream( stream, label, values[label] );
    }

    appendFooterToStream( stream );

    return true;
}

void RifFractureModelGeologicalFrkExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl << "<geologic>" << endl;
}

void RifFractureModelGeologicalFrkExporter::appendToStream( QTextStream&               stream,
                                                            const QString&             label,
                                                            const std::vector<double>& values )
{
    stream << "<cNamedSet>" << endl
           << "<name>" << endl
           << label << endl
           << "</name>" << endl
           << "<dimCount>" << endl
           << 1 << endl
           << "</dimCount>" << endl
           << "<sizes>" << endl
           << values.size() << endl
           << "</sizes>" << endl
           << "<data>" << endl;
    for ( auto val : values )
    {
        stream << val << endl;
    }

    stream << "</data>" << endl << "</cNamedSet>" << endl;
}

void RifFractureModelGeologicalFrkExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</geologic>" << endl;
}

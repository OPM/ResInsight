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

#include "RifStimPlanModelGeologicalFrkExporter.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifCsvDataTableFormatter.h"

#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelGeologicalFrkExporter::writeToFile( RimStimPlanModel* stimPlanModel,
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

    std::vector<double> tvd = stimPlanModel->calculator()->calculateTrueVerticalDepth();
    // Warn if the generated model has too many layers for StimPlan
    if ( tvd.size() > MAX_STIMPLAN_LAYERS )
    {
        RiaLogging::warning(
            QString( "Exporting model with too many layers: %1. Maximum supported number of layers is %2." )
                .arg( tvd.size() )
                .arg( MAX_STIMPLAN_LAYERS ) );
    }

    // Make sure stress gradients are in the valid interval
    std::vector<double> stressGradients = stimPlanModel->calculator()->calculateStressGradient();
    fixupStressGradients( stressGradients, MIN_STRESS_GRADIENT, MAX_STRESS_GRADIENT, DEFAULT_STRESS_GRADIENT );

    std::map<QString, std::vector<double>> values;
    values["dpthlyr"]        = tvd;
    values["strs"]           = stimPlanModel->calculator()->calculateStress();
    values["strsg"]          = stressGradients;
    values["elyr"]           = stimPlanModel->calculator()->calculateYoungsModulus();
    values["poissonr"]       = stimPlanModel->calculator()->calculatePoissonsRatio();
    values["tuflyr"]         = stimPlanModel->calculator()->calculateKIc();
    values["clyrc"]          = stimPlanModel->calculator()->calculateFluidLossCoefficient();
    values["clyrs"]          = stimPlanModel->calculator()->calculateSpurtLoss();
    values["pembed"]         = stimPlanModel->calculator()->calculateProppandEmbedment();
    values["zoneResPres"]    = stimPlanModel->calculator()->calculateReservoirPressure();
    values["zoneWaterSat"]   = stimPlanModel->calculator()->calculateImmobileFluidSaturation();
    values["zonePorosity"]   = stimPlanModel->calculator()->calculatePorosity();
    values["zoneHorizPerm"]  = stimPlanModel->calculator()->calculateHorizontalPermeability();
    values["zoneVertPerm"]   = stimPlanModel->calculator()->calculateVerticalPermeability();
    values["zoneTemp"]       = stimPlanModel->calculator()->calculateTemperature();
    values["zoneRelPerm"]    = stimPlanModel->calculator()->calculateRelativePermeabilityFactor();
    values["zonePoroElas"]   = stimPlanModel->calculator()->calculatePoroElasticConstant();
    values["zoneThermalExp"] = stimPlanModel->calculator()->calculateThermalExpansionCoefficient();

    // Special values for csv export
    auto [depthStart, depthEnd]    = createDepthRanges( tvd );
    values["dpthstart"]            = depthStart;
    values["dpthend"]              = depthEnd;
    std::vector<QString> csvLabels = { "dpthstart", "dpthend" };
    for ( const QString& label : labels )
        csvLabels.push_back( label );

    return writeToFrkFile( filepath, labels, values ) && writeToCsvFile( filepath, csvLabels, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelGeologicalFrkExporter::writeToFrkFile( const QString&                                filepath,
                                                            const std::vector<QString>&                   labels,
                                                            const std::map<QString, std::vector<double>>& values )

{
    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );

    for ( QString label : labels )
    {
        auto vals = values.find( label );
        if ( vals == values.end() ) return false;

        warnOnInvalidData( label, vals->second );
        appendToStream( stream, label, vals->second );
    }

    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelGeologicalFrkExporter::writeToCsvFile( const QString&                                filepath,
                                                            const std::vector<QString>&                   labels,
                                                            const std::map<QString, std::vector<double>>& values )

{
    // Create the csv in the same directory as the frk file
    QFileInfo fi( filepath );
    QString   csvFilepath = fi.absolutePath() + "/Geological.csv";

    QFile data( csvFilepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream              stream( &data );
    QString                  fieldSeparator = RiaPreferences::current()->csvTextExportFieldSeparator;
    RifCsvDataTableFormatter formatter( stream, fieldSeparator );

    // Construct header
    std::vector<RifTextDataTableColumn> header;
    for ( auto label : labels )
    {
        header.push_back( RifTextDataTableColumn( label, RifTextDataTableDoubleFormat::RIF_FLOAT ) );
    }
    formatter.header( header );

    // The length of the vectors are assumed to be equal
    size_t idx    = 0;
    bool   isDone = false;
    while ( !isDone )
    {
        // Construct one row
        for ( auto label : labels )
        {
            auto vals = values.find( label );
            if ( vals == values.end() ) return false;

            if ( idx >= vals->second.size() )
                isDone = true;
            else
            {
                formatter.add( vals->second[idx] );
            }
        }
        formatter.rowCompleted();
        idx++;
    }

    formatter.tableCompleted();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelGeologicalFrkExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << '\n' << "<geologic>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelGeologicalFrkExporter::appendToStream( QTextStream&               stream,
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

    stream << "</data>" << '\n' << "</cNamedSet>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelGeologicalFrkExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</geologic>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanModelGeologicalFrkExporter::fixupStressGradients( std::vector<double>& stressGradients,
                                                                  double               minStressGradient,
                                                                  double               maxStressGradient,
                                                                  double               defaultStressGradient )
{
    for ( size_t i = 0; i < stressGradients.size(); i++ )
    {
        if ( stressGradients[i] < minStressGradient || stressGradients[i] > maxStressGradient )
        {
            RiaLogging::warning(
                QString( "Found stress gradient outside valid range [%1, %2]. Replacing %3 with default value: %4." )
                    .arg( minStressGradient )
                    .arg( maxStressGradient )
                    .arg( stressGradients[i] )
                    .arg( defaultStressGradient ) );

            stressGradients[i] = defaultStressGradient;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelGeologicalFrkExporter::warnOnInvalidData( const QString& label, const std::vector<double>& values )
{
    bool isInvalid = hasInvalidData( values );
    if ( isInvalid )
    {
        RiaLogging::warning( QString( "Found invalid data in Geological.FRK export of property '%1'." ).arg( label ) );
    }

    return isInvalid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelGeologicalFrkExporter::hasInvalidData( const std::vector<double>& values )
{
    for ( auto v : values )
    {
        if ( std::isinf( v ) || std::isnan( v ) ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<double>>
    RifStimPlanModelGeologicalFrkExporter::createDepthRanges( const std::vector<double>& tvd )
{
    std::vector<double> startTvd;
    std::vector<double> endTvd;

    for ( size_t i = 0; i < tvd.size(); i++ )
    {
        startTvd.push_back( tvd[i] );
        // Special handling for last range
        if ( i == tvd.size() - 1 )
            endTvd.push_back( startTvd[i] );
        else
            endTvd.push_back( tvd[i + 1] );
    }

    return std::make_pair( startTvd, endTvd );
}

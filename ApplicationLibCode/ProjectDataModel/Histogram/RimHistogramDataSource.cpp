/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimHistogramDataSource.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimHistogramDataSource, "HistogramDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramDataSource::RimHistogramDataSource()
    : dataSourceChanged( this )
{
    CAF_PDM_InitObject( "Histogram Data Source", );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramDataSource::~RimHistogramDataSource()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramDataSource::computeHistogramBins( double min, double max, int numBins, RimHistogramPlot::GraphType graphType )
{
    const double binSize = ( max - min ) / numBins;

    std::vector<double> values;
    for ( int i = 0; i < numBins; i++ )
    {
        if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
        {
            const double binMin = min + binSize * i;
            const double binMax = min + binSize * ( i + 1 );

            // Close first on left side
            if ( i == 0 ) values.push_back( binMin );

            values.push_back( binMin );
            values.push_back( binMax );

            // Close last bar on right side
            if ( i == numBins - 1 ) values.push_back( binMax );
        }
        else if ( graphType == RimHistogramPlot::GraphType::LINE_GRAPH )
        {
            double centerOfBin = min + binSize * i + binSize / 2.0;
            values.push_back( centerOfBin );
        }
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramDataSource::computeHistogramFrequencies( const std::vector<size_t>&      values,
                                                                         RimHistogramPlot::GraphType     graphType,
                                                                         RimHistogramPlot::FrequencyType frequencyType )
{
    std::vector<double> valuesAsDouble( values.begin(), values.end() );
    return computeHistogramFrequencies( valuesAsDouble, graphType, frequencyType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramDataSource::computeHistogramFrequencies( const std::vector<double>&      values,
                                                                         RimHistogramPlot::GraphType     graphType,
                                                                         RimHistogramPlot::FrequencyType frequencyType )
{
    double sumElements = 0.0;
    for ( double value : values )
        sumElements += value;

    std::vector<double> frequencies;
    for ( size_t i = 0; i < values.size(); i++ )
    {
        double value = values[i];
        if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY ) value /= sumElements;
        if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT ) value = value / sumElements * 100.0;

        if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
        {
            // Close first bar on left side
            if ( i == 0 ) frequencies.push_back( 0.0 );

            frequencies.push_back( value );
            frequencies.push_back( value );

            // Close last bar on right side
            if ( i == values.size() - 1 ) frequencies.push_back( 0.0 );
        }
        else if ( graphType == RimHistogramPlot::GraphType::LINE_GRAPH )
        {
            frequencies.push_back( value );
        }
    }
    return frequencies;
}

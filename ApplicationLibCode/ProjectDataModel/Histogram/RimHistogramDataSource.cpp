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

#include "cafPdmUiOrdering.h"

#include <cmath>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimHistogramDataSource, "HistogramDataSource" );

namespace caf
{
template <>
void caf::AppEnum<RigHistogramCalculator::BinningMode>::setUp()
{
    addItem( RigHistogramCalculator::BinningMode::LINEAR, "LINEAR", "Linear" );
    addItem( RigHistogramCalculator::BinningMode::LOGARITHMIC, "LOGARITHMIC", "Logarithmic" );
    setDefault( RigHistogramCalculator::BinningMode::LINEAR );
}

template <>
void caf::AppEnum<RimHistogramDataSource::BinRangeMode>::setUp()
{
    addItem( RimHistogramDataSource::BinRangeMode::AUTOMATIC, "AUTOMATIC", "Automatic" );
    addItem( RimHistogramDataSource::BinRangeMode::USER_DEFINED, "USER_DEFINED", "User Defined" );
    setDefault( RimHistogramDataSource::BinRangeMode::AUTOMATIC );
}

template <>
void caf::AppEnum<RigHistogramCalculator::OutOfRangeHandling>::setUp()
{
    addItem( RigHistogramCalculator::OutOfRangeHandling::EXCLUDE, "EXCLUDE", "Exclude" );
    addItem( RigHistogramCalculator::OutOfRangeHandling::INCLUDE_IN_BOUNDARY_BINS, "INCLUDE_IN_BOUNDARY_BINS", "Include in Boundary Bins" );
    setDefault( RigHistogramCalculator::OutOfRangeHandling::EXCLUDE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramDataSource::RimHistogramDataSource()
    : dataSourceChanged( this )
    , cumulativeChanged( this )
    , logarithmicBinningEnabled( this )
{
    CAF_PDM_InitObject( "Histogram Data Source", );

    CAF_PDM_InitFieldNoDefault( &m_binningMode, "BinningMode", "Binning Mode" );
    CAF_PDM_InitFieldNoDefault( &m_binRangeMode, "BinRangeMode", "Bin Range" );
    CAF_PDM_InitField( &m_binRangeMin, "BinRangeMin", 0.0, "Minimum" );
    CAF_PDM_InitField( &m_binRangeMax, "BinRangeMax", 1.0, "Maximum" );
    CAF_PDM_InitFieldNoDefault( &m_outOfRangeHandling, "OutOfRangeHandling", "Out of Range Values" );
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
bool RimHistogramDataSource::showCumulativeCurve() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramDataSource::setShowCumulativeCurve( bool showCumulativeCurve )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_binningMode && m_binningMode() == RigHistogramCalculator::BinningMode::LOGARITHMIC )
    {
        logarithmicBinningEnabled.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramDataSource::appendBinningUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_binningMode );
    uiOrdering.add( &m_binRangeMode );
    if ( useUserDefinedBinRange() )
    {
        uiOrdering.add( &m_binRangeMin );
        uiOrdering.add( &m_binRangeMax );
        uiOrdering.add( &m_outOfRangeHandling );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimHistogramDataSource::computeBinRange( BinRangeMode                        binRangeMode,
                                                                   double                              userMin,
                                                                   double                              userMax,
                                                                   double                              dataMin,
                                                                   double                              dataMax,
                                                                   RigHistogramCalculator::BinningMode binningMode,
                                                                   double                              smallestPositiveValue )
{
    double min = dataMin;
    double max = dataMax;
    if ( binRangeMode == BinRangeMode::USER_DEFINED )
    {
        min = userMin;
        max = userMax;
    }

    if ( binningMode == RigHistogramCalculator::BinningMode::LOGARITHMIC && min <= 0.0 )
    {
        min = smallestPositiveValue;
    }

    return { min, max };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimHistogramDataSource::binRange( double dataMin, double dataMax, double smallestPositiveValue ) const
{
    return computeBinRange( m_binRangeMode(), m_binRangeMin(), m_binRangeMax(), dataMin, dataMax, m_binningMode(), smallestPositiveValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramCalculator::BinningMode RimHistogramDataSource::binningMode() const
{
    return m_binningMode();
}

//--------------------------------------------------------------------------------------------------
/// Out of range values only exist for a user-defined bin range: an automatic range covers the data
//--------------------------------------------------------------------------------------------------
RigHistogramCalculator::OutOfRangeHandling RimHistogramDataSource::outOfRangeHandling() const
{
    if ( !useUserDefinedBinRange() ) return RigHistogramCalculator::OutOfRangeHandling::EXCLUDE;

    return m_outOfRangeHandling();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramDataSource::useUserDefinedBinRange() const
{
    return m_binRangeMode() == BinRangeMode::USER_DEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramDataSource::computeHistogramBins( double                              min,
                                                                  double                              max,
                                                                  int                                 numBins,
                                                                  RimHistogramPlot::GraphType         graphType,
                                                                  bool                                cumulative,
                                                                  RigHistogramCalculator::BinningMode binningMode )
{
    const bool isLogarithmic = ( binningMode == RigHistogramCalculator::BinningMode::LOGARITHMIC && min > 0.0 );

    const double logMin  = isLogarithmic ? std::log10( min ) : 0.0;
    const double logStep = isLogarithmic ? ( std::log10( max ) - logMin ) / numBins : 0.0;
    const double binSize = ( max - min ) / numBins;

    auto binEdge = [=]( int i ) { return isLogarithmic ? std::pow( 10.0, logMin + logStep * i ) : min + binSize * i; };

    std::vector<double> values;
    for ( int i = 0; i < numBins; i++ )
    {
        if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
        {
            const double binMin = binEdge( i );
            const double binMax = binEdge( i + 1 );

            // Close first on left side
            if ( i == 0 ) values.push_back( binMin );

            values.push_back( binMin );
            values.push_back( binMax );

            // Close last bar on right side. A cumulative curve is not closed: it should end at its
            // maximum instead of dropping back to zero.
            if ( i == numBins - 1 && !cumulative ) values.push_back( binMax );
        }
        else if ( graphType == RimHistogramPlot::GraphType::LINE_GRAPH )
        {
            // For logarithmic bins the center is the geometric mean of the bin edges
            double centerOfBin = isLogarithmic ? std::pow( 10.0, logMin + logStep * ( i + 0.5 ) ) : min + binSize * i + binSize / 2.0;
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
                                                                         RimHistogramPlot::FrequencyType frequencyType,
                                                                         bool                            cumulative )
{
    std::vector<double> valuesAsDouble( values.begin(), values.end() );
    return computeHistogramFrequencies( valuesAsDouble, graphType, frequencyType, cumulative );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramDataSource::computeHistogramFrequencies( const std::vector<double>&      values,
                                                                         RimHistogramPlot::GraphType     graphType,
                                                                         RimHistogramPlot::FrequencyType frequencyType,
                                                                         bool                            cumulative )
{
    double sumElements = 0.0;
    for ( double value : values )
        sumElements += value;

    double runningSum = 0.0;

    std::vector<double> frequencies;
    for ( size_t i = 0; i < values.size(); i++ )
    {
        double value = values[i];
        if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY ) value /= sumElements;
        if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT ) value = value / sumElements * 100.0;

        if ( cumulative )
        {
            runningSum += value;
            value = runningSum;
        }

        if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
        {
            // Close first bar on left side
            if ( i == 0 ) frequencies.push_back( 0.0 );

            frequencies.push_back( value );
            frequencies.push_back( value );

            // Close last bar on right side. A cumulative curve is not closed: it should end at its
            // maximum instead of dropping back to zero.
            if ( i == values.size() - 1 && !cumulative ) frequencies.push_back( 0.0 );
        }
        else if ( graphType == RimHistogramPlot::GraphType::LINE_GRAPH )
        {
            frequencies.push_back( value );
        }
    }
    return frequencies;
}

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

#include "RimEnsembleFractureHistogramDataSource.h"

#include "Histogram/RimHistogramPlot.h"
#include "RimEnsembleFractureStatistics.h"
#include "RimProject.h"

CAF_PDM_XML_SOURCE_INIT( RimEnsembleFractureHistogramDataSource, "EnsembleFractureHistogramDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureHistogramDataSource::RimEnsembleFractureHistogramDataSource()
{
    CAF_PDM_InitObject( "Ensemble Fracture Histogram Data Source" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFractureStatistics, "EnsembleFractureStatistics", "Ensemble Fracture Statistics" );
    m_ensembleFractureStatistics.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_property, "Property", "Property" );

    CAF_PDM_InitField( &m_numBins, "NumBins", 15, "Number of Bins" );

    setDefaults();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFractureHistogramDataSource::~RimEnsembleFractureHistogramDataSource()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleFractureHistogramDataSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleFractureStatistics )
    {
        std::vector<RimEnsembleFractureStatistics*> esfItems =
            RimProject::current()->descendantsIncludingThisOfType<RimEnsembleFractureStatistics>();
        for ( auto item : esfItems )
        {
            options.push_back( caf::PdmOptionItemInfo( item->name(), item ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureHistogramDataSource ::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_ensembleFractureStatistics );
    uiOrdering.add( &m_property );
    uiOrdering.add( &m_numBins );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureHistogramDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                               const QVariant&            oldValue,
                                                               const QVariant&            newValue )
{
    dataSourceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleFractureHistogramDataSource::unitNameY() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleFractureHistogramDataSource::unitNameX() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramDataSource::HistogramResult RimEnsembleFractureHistogramDataSource::compute( RimHistogramPlot::GraphType graphType,
                                                                                         RimHistogramPlot::FrequencyType frequencyType ) const
{
    RimHistogramDataSource::HistogramResult result;

    if ( !m_ensembleFractureStatistics() ) return result;

    RigHistogramData histogramData =
        RigEnsembleFractureStatisticsCalculator::createStatisticsData( m_ensembleFractureStatistics(), m_property(), m_numBins() );

    if ( !histogramData.isHistogramVectorValid() ) return result;

    double min = histogramData.min;
    double max = histogramData.max;

    result.valuesX = computeHistogramBins( min, max, m_numBins, graphType );
    result.valuesY = computeHistogramFrequencies( histogramData.histogram, graphType, frequencyType );

    result.p10  = histogramData.p10;
    result.mean = histogramData.mean;
    result.p90  = histogramData.p90;

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleFractureHistogramDataSource::name() const
{
    if ( m_ensembleFractureStatistics() == nullptr )
    {
        return "Undefined";
    }

    QStringList nameTags;
    nameTags += m_ensembleFractureStatistics()->name();
    nameTags += caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::uiText( m_property() );

    return nameTags.join( ", " ).toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureHistogramDataSource::setEnsembleFractureStatistics( RimEnsembleFractureStatistics* statistics )
{
    m_ensembleFractureStatistics = statistics;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureHistogramDataSource::setProperty( RigEnsembleFractureStatisticsCalculator::PropertyType property )
{
    m_property = property;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureHistogramDataSource::setDefaults()
{
    auto esfItems = RimProject::current()->descendantsIncludingThisOfType<RimEnsembleFractureStatistics>();
    if ( !esfItems.empty() ) m_ensembleFractureStatistics = esfItems.front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFractureHistogramDataSource::loadDataAndUpdate()
{
    dataSourceChanged.send();
}

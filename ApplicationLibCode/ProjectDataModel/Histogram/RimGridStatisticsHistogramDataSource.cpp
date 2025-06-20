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

#include "RimGridStatisticsHistogramDataSource.h"

#include "RiaLogging.h"

#include "Histogram/RimHistogramPlot.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimHistogramCalculator.h"
#include "RimProject.h"
#include "RimTools.h"

#include "RigStatisticsMath.h"

#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_XML_SOURCE_INIT( RimGridStatisticsHistogramDataSource, "GridStatisticsHistogramDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsHistogramDataSource::RimGridStatisticsHistogramDataSource()
{
    CAF_PDM_InitObject( "Ensemble Parameter Histogram Data Source", );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", -1, "Time Step" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility" );

    CAF_PDM_InitFieldNoDefault( &m_property, "Property", "Property" );
    m_property = new RimEclipseResultDefinition( caf::PdmUiItemInfo::TOP );
    m_property.uiCapability()->setUiTreeChildrenHidden( true );
    m_property->setTernaryEnabled( false );

    CAF_PDM_InitField( &m_numBins, "NumBins", 15, "Number of Bins" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsHistogramDataSource::~RimGridStatisticsHistogramDataSource()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridStatisticsHistogramDataSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options; // = RimStatisticsPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_case )
    {
        RimTools::eclipseCaseOptionItems( &options );
        if ( options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        options.push_back( caf::PdmOptionItemInfo( "All Time Steps", -1 ) );

        RimTools::timeStepsForCase( m_case, &options );
    }
    else if ( fieldNeedingOptions == &m_cellFilterView )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclipseCase )
        {
            options.push_back( caf::PdmOptionItemInfo( "Disabled", nullptr ) );
            for ( RimEclipseView* view : eclipseCase->reservoirViews() )
            {
                CVF_ASSERT( view && "Really always should have a valid view pointer in ReservoirViews" );
                options.push_back( caf::PdmOptionItemInfo( view->name(), view, false, view->uiIconProvider() ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource ::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    if ( m_case )
    {
        uiOrdering.add( &m_timeStep );
        uiOrdering.add( &m_cellFilterView );
        caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup( "Property" );
        m_property->uiOrdering( uiConfigName, *propertyGroup );
    }

    uiOrdering.add( &m_numBins );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                             const QVariant&            oldValue,
                                                             const QVariant&            newValue )
{
    if ( changedField == &m_case )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
        if ( eclipseCase )
        {
            m_property->setEclipseCase( eclipseCase );
            m_property->updateConnectedEditors();
            dataSourceChanged.send();
        }
    }
    else
    {
        dataSourceChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimGridStatisticsHistogramDataSource::unitNameY() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimGridStatisticsHistogramDataSource::unitNameX() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridStatisticsHistogramDataSource::valuesX( RimHistogramPlot::GraphType graphType ) const
{
    std::vector<double> values;
    RigHistogramData    histogramData = createStatisticsData();
    if ( histogramData.isHistogramVectorValid() )
    {
        std::vector<size_t> histogram = histogramData.histogram;
        double              min       = histogramData.min;
        double              max       = histogramData.max;
        double              binSize   = ( max - min ) / m_numBins;
        for ( int i = 0; i < m_numBins; i++ )
        {
            if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
            {
                values.push_back( min + binSize * i );
                values.push_back( min + binSize * ( i + 1 ) );
            }
            else if ( graphType == RimHistogramPlot::GraphType::LINE_GRAPH )
            {
                double centerOfBin = min + binSize * i + binSize / 2.0;
                values.push_back( centerOfBin );
            }
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimGridStatisticsHistogramDataSource::valuesY( RimHistogramPlot::GraphType     graphType,
                                                                   RimHistogramPlot::FrequencyType frequencyType ) const
{
    RigHistogramData histogramData = createStatisticsData();
    if ( histogramData.isHistogramVectorValid() )
    {
        std::vector<size_t> histogram = histogramData.histogram;

        double sumElements = 0.0;
        for ( double value : histogram )
            sumElements += value;

        std::vector<double> frequencies;
        for ( size_t frequency : histogram )
        {
            double value = frequency;
            if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY ) value /= sumElements;
            if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT ) value = value / sumElements * 100.0;

            frequencies.push_back( value );
            if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
            {
                frequencies.push_back( value );
            }
        }
        return frequencies;
    }
    // if ( m_ensemble )
    // {
    //     auto parameter = m_ensemble->ensembleParameter( m_parameter );
    //     if ( parameter.isNumeric() && parameter.isValid() )
    //     {
    //         std::vector<double> values;
    //         for ( const QVariant& v : parameter.values )
    //         {
    //             values.push_back( v.toDouble() );
    //         }

    //         double min = parameter.minValue;
    //         double max = parameter.maxValue;

    //         std::vector<size_t>    histogram;
    //         RigHistogramCalculator histCalc( min, max, m_numBins, &histogram );
    //         histCalc.addData( values );

    //         double p10 = histCalc.calculatePercentil( 0.1, RigStatisticsMath::PercentileStyle::REGULAR );
    //         double p50 = histCalc.calculatePercentil( 0.5, RigStatisticsMath::PercentileStyle::REGULAR );
    //         double p90 = histCalc.calculatePercentil( 0.9, RigStatisticsMath::PercentileStyle::REGULAR );

    //         RiaLogging::info( QString( "%1: P10=%2 Mean=%3 P90=%4" ).arg( QString::fromStdString( name() ) ).arg( p10 ).arg( p50 ).arg(
    //         p90 ) );

    //         double sumElements = 0.0;
    //         for ( double value : histogram )
    //             sumElements += value;

    //         std::vector<double> frequencies;
    //         for ( size_t frequency : histogram )
    //         {
    //             double value = frequency;
    //             if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY ) value /= sumElements;
    //             if ( frequencyType == RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT ) value = value / sumElements * 100.0;

    //             frequencies.push_back( value );
    //             if ( graphType == RimHistogramPlot::GraphType::BAR_GRAPH )
    //             {
    //                 frequencies.push_back( value );
    //             }
    //         }
    //         return frequencies;
    //     }
    // }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData RimGridStatisticsHistogramDataSource::createStatisticsData() const
{
    std::unique_ptr<RimHistogramCalculator> histogramCalculator;
    histogramCalculator = std::make_unique<RimHistogramCalculator>();
    histogramCalculator->setNumBins( static_cast<size_t>( m_numBins() ) );

    RigHistogramData histogramData;

    RimHistogramCalculator::StatisticsCellRangeType cellRange = RimHistogramCalculator::StatisticsCellRangeType::ALL_CELLS;

    RimHistogramCalculator::StatisticsTimeRangeType timeRange = RimHistogramCalculator::StatisticsTimeRangeType::ALL_TIMESTEPS;
    int                                             timeStep  = 0;
    if ( m_timeStep() != -1 && !m_property()->hasStaticResult() )
    {
        timeStep  = m_timeStep();
        timeRange = RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP;
    }

    if ( m_cellFilterView.value() )
    {
        // Filter by visible cells of the view
        cellRange                   = RimHistogramCalculator::StatisticsCellRangeType::VISIBLE_CELLS;
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( m_cellFilterView.value() );
        histogramData               = histogramCalculator->histogramData( eclipseView, m_property.value(), cellRange, timeRange, timeStep );
    }
    else
    {
        RimEclipseView* eclipseView = nullptr;
        histogramData               = histogramCalculator->histogramData( eclipseView, m_property.value(), cellRange, timeRange, timeStep );
    }

    return histogramData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimGridStatisticsHistogramDataSource::name() const
{
    if ( m_case() == nullptr )
    {
        return "Undefined";
    }

    QStringList nameTags;
    nameTags += m_property()->resultVariable();
    nameTags += m_case()->caseUserDescription();

    auto createTimeStepString = [this]() -> QString
    {
        QString timeStepStr = "";
        if ( m_case() && m_property->hasDynamicResult() )
        {
            if ( m_timeStep == -1 )
            {
                return "All Time Steps";
            }
            return m_case->timeStepStrings()[m_timeStep];
        }

        return "";
    };

    QString timeStepStr = createTimeStepString();
    if ( !timeStepStr.isEmpty() )
    {
        nameTags += timeStepStr;
    }

    return nameTags.join( ", " ).toStdString();
}

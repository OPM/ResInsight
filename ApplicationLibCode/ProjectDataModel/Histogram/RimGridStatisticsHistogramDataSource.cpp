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

#include "Histogram/RimHistogramPlot.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimHistogramCalculator.h"
#include "RimProject.h"
#include "RimTools.h"

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
    QList<caf::PdmOptionItemInfo> options;

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
        if ( RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() ) )
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
        if ( RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() ) )
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
RimHistogramDataSource::HistogramResult RimGridStatisticsHistogramDataSource::compute( RimHistogramPlot::GraphType     graphType,
                                                                                       RimHistogramPlot::FrequencyType frequencyType ) const
{
    RimHistogramDataSource::HistogramResult result;

    RigHistogramData histogramData = createStatisticsData();
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
RigHistogramData RimGridStatisticsHistogramDataSource::createStatisticsData() const
{
    std::unique_ptr<RimHistogramCalculator> histogramCalculator = std::make_unique<RimHistogramCalculator>();
    histogramCalculator->setNumBins( static_cast<size_t>( m_numBins() ) );

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
        return histogramCalculator->histogramData( eclipseView, m_property.value(), cellRange, timeRange, timeStep );
    }
    else
    {
        RimEclipseView* eclipseView = nullptr;
        return histogramCalculator->histogramData( eclipseView, m_property.value(), cellRange, timeRange, timeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimGridStatisticsHistogramDataSource::name() const
{
    if ( m_case() == nullptr ) return "Undefined";

    QStringList nameTags;
    nameTags += m_property()->resultVariable();
    nameTags += m_case()->caseUserDescription();

    auto createTimeStepString = [this]() -> QString
    {
        if ( m_case() && m_property->hasDynamicResult() )
        {
            if ( m_timeStep == -1 ) return "All Time Steps";
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource ::initAfterRead()
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
    if ( eclipseCase )
    {
        m_property->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource::setDefaults()
{
    RimProject* project = RimProject::current();
    if ( project )
    {
        if ( !project->eclipseCases().empty() )
        {
            RimEclipseCase* eclipseCase = project->eclipseCases().front();
            m_case                      = eclipseCase;
            m_property->setEclipseCase( eclipseCase );

            m_property->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
            m_property->setResultVariable( "PORO" );

            if ( eclipseCase && !eclipseCase->reservoirViews().empty() )
            {
                m_cellFilterView.setValue( eclipseCase->reservoirViews().front() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource::cellFilterViewUpdated()
{
    dataSourceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource::loadDataAndUpdate()
{
    dataSourceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsHistogramDataSource::setPropertiesFromView( RimEclipseView* view )
{
    CAF_ASSERT( view );

    m_case = view->ownerCase();

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase ) m_property->setEclipseCase( eclipseCase );

    const RimEclipseResultDefinition* resDef = dynamic_cast<const RimEclipseResultDefinition*>( view->cellResult() );
    if ( resDef ) m_property->simpleCopy( resDef );

    dataSourceChanged.send();
}

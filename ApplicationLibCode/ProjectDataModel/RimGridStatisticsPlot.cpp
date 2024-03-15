/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimGridStatisticsPlot.h"

#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimHistogramCalculator.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimTools.h"

#include "RigHistogramData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimGridStatisticsPlot, "GridStatisticsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlot::RimGridStatisticsPlot()
{
    CAF_PDM_InitObject( "Grid Statistics Plot", ":/statistics.png", "", "A Plot of Grid Statistics" );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", -1, "Time Step" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility" );

    CAF_PDM_InitFieldNoDefault( &m_property, "Property", "Property" );
    m_property = new RimEclipseResultDefinition( caf::PdmUiItemInfo::TOP );
    m_property.uiCapability()->setUiTreeChildrenHidden( true );
    m_property->setTernaryEnabled( false );

    m_plotLegendsHorizontal.uiCapability()->setUiHidden( true );

    setDefaults();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridStatisticsPlot::~RimGridStatisticsPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::setDefaults()
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

            m_numHistogramBins = 15;
            m_tickNumberFormat = RiaNumberFormat::NumberFormatType::FIXED;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::setPropertiesFromView( RimEclipseView* view )
{
    CAF_ASSERT( view );

    m_case = view->ownerCase();

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase ) m_property->setEclipseCase( eclipseCase );

    const RimEclipseResultDefinition* resDef = dynamic_cast<const RimEclipseResultDefinition*>( view->cellResult() );
    if ( resDef ) m_property->simpleCopy( resDef );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimStatisticsPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
        if ( eclipseCase )
        {
            m_property->setEclipseCase( eclipseCase );
            m_property->updateConnectedEditors();
            loadDataAndUpdate();
        }
    }
    else
    {
        loadDataAndUpdate();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    if ( m_case )
    {
        uiOrdering.add( &m_timeStep );
        uiOrdering.add( &m_cellFilterView );
        caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup( "Property" );
        m_property->uiOrdering( uiConfigName, *propertyGroup );
    }

    const bool showNumHistogramBins = true;
    RimStatisticsPlot::uiOrderingForHistogram( uiConfigName, uiOrdering, showNumHistogramBins );
    RimStatisticsPlot::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridStatisticsPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimStatisticsPlot::calculateValueOptions( fieldNeedingOptions );

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
void RimGridStatisticsPlot::initAfterRead()
{
    RimStatisticsPlot::initAfterRead();

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
    if ( eclipseCase )
    {
        m_property->setEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridStatisticsPlot::cellFilterViewUpdated()
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridStatisticsPlot::hasStatisticsData() const
{
    return ( m_viewer && m_case() && m_property() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData RimGridStatisticsPlot::createStatisticsData() const
{
    std::unique_ptr<RimHistogramCalculator> histogramCalculator;
    histogramCalculator.reset( new RimHistogramCalculator );
    histogramCalculator->setNumBins( static_cast<size_t>( m_numHistogramBins() ) );

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
QString RimGridStatisticsPlot::createAutoName() const
{
    if ( m_case() == nullptr )
    {
        return "Undefined";
    }

    QStringList nameTags;
    nameTags += m_property()->resultVariable();
    nameTags += m_case()->caseUserDescription();

    QString timeStepStr = timeStepString();
    if ( !timeStepStr.isEmpty() )
    {
        nameTags += timeStepStr;
    }

    return nameTags.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridStatisticsPlot::timeStepString() const
{
    if ( m_case() && m_property->hasDynamicResult() )
    {
        if ( m_timeStep == -1 )
        {
            return "All Time Steps";
        }
        return m_case->timeStepStrings()[m_timeStep];
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridStatisticsPlot::createXAxisTitle() const
{
    if ( m_case() == nullptr ) return "";

    QStringList nameTags;
    nameTags += m_property()->resultVariable();

    QString timeStepStr = timeStepString();
    if ( !timeStepStr.isEmpty() )
    {
        nameTags += timeStepStr;
    }

    return nameTags.join( ", " );
}

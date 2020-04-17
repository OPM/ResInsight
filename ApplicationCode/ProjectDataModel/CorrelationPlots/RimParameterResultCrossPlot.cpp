/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimParameterResultCrossPlot.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"
#include "RiaStatisticsTools.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "RifSummaryReaderInterface.h"

#include "RimDerivedSummaryCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"

#include <limits>
#include <map>
#include <set>

CAF_PDM_SOURCE_INIT( RimParameterResultCrossPlot, "ParameterResultCrossPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot::RimParameterResultCrossPlot()
    : RimAbstractCorrelationPlot()
{
    CAF_PDM_InitObject( "ParameterResultCross Plot", ":/ParameterResultCrossPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Ensemble Parameter", "", "", "" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot::~RimParameterResultCrossPlot()
{
    removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue )
{
    RimAbstractCorrelationPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_ensembleParameter )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );
    curveDataGroup->add( &m_ensemble );
    curveDataGroup->add( &m_summaryAddressUiField );
    curveDataGroup->add( &m_pushButtonSelectSummaryAddress, { false, 1, 0 } );
    curveDataGroup->add( &m_timeStep );

    caf::PdmUiGroup* crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot Parameters" );
    crossPlotGroup->add( &m_ensembleParameter );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_showPlotTitle );
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimParameterResultCrossPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options =
        RimAbstractCorrelationPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        if ( m_ensemble )
        {
            for ( const auto& param : m_ensemble->variationSortedEnsembleParameters() )
            {
                options.push_back( caf::PdmOptionItemInfo( param.uiName(), param.name ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_summaryAddressUiField = m_summaryAddress->address();

    if ( m_plotWidget )
    {
        m_plotWidget->insertLegend( nullptr );
        m_plotWidget->updateLegend();

        createPoints();

        this->updateAxes();
        this->updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisTitleText( QwtPlot::yLeft, QString::fromStdString( m_summaryAddressUiField().uiText() ) );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
    m_plotWidget->setAxisAutoScale( QwtPlot::yLeft, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::yLeft, 11, 11, false, Qt::AlignCenter );

    m_plotWidget->setAxisTitleText( QwtPlot::xBottom, m_ensembleParameter );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xBottom, 11, 11, false, Qt::AlignCenter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::createPoints()
{
    time_t selectedTimestep = m_timeStep().toTime_t();

    if ( !m_ensemble ) return;

    if ( !m_summaryAddress ) return;

    EnsembleParameter ensembleParameter = m_ensemble->ensembleParameter( m_ensembleParameter );
    if ( !( ensembleParameter.isNumeric() && ensembleParameter.isValid() ) ) return;

    for ( size_t caseIdx = 0u; caseIdx < m_ensemble->allSummaryCases().size(); ++caseIdx )
    {
        std::vector<double> caseValuesAtTimestep;
        std::vector<double> parameterValues;

        auto summaryCase = m_ensemble->allSummaryCases()[caseIdx];

        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        if ( !reader ) continue;

        if ( !summaryCase->caseRealizationParameters() ) continue;

        std::vector<double> values;

        double closestValue    = std::numeric_limits<double>::infinity();
        time_t closestTimeStep = 0;
        if ( reader->values( m_summaryAddress->address(), &values ) )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( m_summaryAddress->address() );
            for ( size_t i = 0; i < timeSteps.size(); ++i )
            {
                if ( timeDiff( timeSteps[i], selectedTimestep ) < timeDiff( selectedTimestep, closestTimeStep ) )
                {
                    closestValue    = values[i];
                    closestTimeStep = timeSteps[i];
                }
            }
        }
        if ( closestValue != std::numeric_limits<double>::infinity() )
        {
            caseValuesAtTimestep.push_back( closestValue );
            double paramValue = ensembleParameter.values[caseIdx].toDouble();
            parameterValues.push_back( paramValue );

            RiuQwtPlotCurve* plotCurve = new RiuQwtPlotCurve;
            plotCurve->setSamplesFromXValuesAndYValues( parameterValues, caseValuesAtTimestep, false );
            plotCurve->setAppearance( RiuQwtPlotCurve::STYLE_NONE,
                                      RiuQwtPlotCurve::INTERPOLATION_POINT_TO_POINT,
                                      0,
                                      QColor( Qt::red ) );
            RiuQwtSymbol* symbol = new RiuQwtSymbol( RiuQwtSymbol::SYMBOL_ELLIPSE, "" );
            symbol->setSize( 12, 12 );
            symbol->setColor( QColor( Qt::red ) );
            plotCurve->setSymbol( symbol );
            plotCurve->attach( m_plotWidget );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle )
    {
        m_description = QString( "Cross Plot %1 x %2" )
                            .arg( m_ensembleParameter )
                            .arg( QString::fromStdString( m_summaryAddressUiField().uiText() ) );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && isMdiWindow() );
}

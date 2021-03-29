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

#include "RiaColorTables.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaStatisticsTools.h"
#include "RiaTextStringTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimDerivedSummaryCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimMultiPlot.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_engine.h"

#include <QStringList>

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
    CAF_PDM_InitObject( "ParameterResultCross Plot", ":/CorrelationCrossPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Ensemble Parameter", "", "", "" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    m_selectMultipleVectors = true;

    m_legendFontSize = caf::FontTools::RelativeSize::XSmall;

    m_xRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
    m_yRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot::~RimParameterResultCrossPlot()
{
    if ( isMdiWindow() ) removeMdiWindowFromMdiArea();
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::setEnsembleParameter( const QString& ensembleParameter )
{
    m_ensembleParameter = ensembleParameter;
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
    m_selectedVarsUiField = selectedQuantitiesText();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );
    curveDataGroup->add( &m_selectedVarsUiField );
    curveDataGroup->add( &m_pushButtonSelectSummaryAddress, { false, 1, 0 } );
    curveDataGroup->add( &m_timeStepFilter );
    curveDataGroup->add( &m_timeStep );
    curveDataGroup->add( &m_useCaseFilter );
    curveDataGroup->add( &m_curveSetForFiltering );
    m_curveSetForFiltering.uiCapability()->setUiHidden( !m_useCaseFilter() );

    caf::PdmUiGroup* crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot Parameters" );
    crossPlotGroup->add( &m_ensembleParameter );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_showPlotTitle );
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    RimPlot::defineUiOrdering( uiConfigName, *plotGroup );
    plotGroup->add( &m_titleFontSize );
    plotGroup->add( &m_legendFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );

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
        for ( const auto& param : ensembleParameters() )
        {
            options.push_back( caf::PdmOptionItemInfo( param.uiName(), param.name ) );
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

    m_selectedVarsUiField = selectedQuantitiesText();

    if ( m_plotWidget && m_analyserOfSelectedCurveDefs )
    {
        createPoints();
        if ( m_showPlotLegends && !isSubPlot<RimMultiPlot>() )
        {
            QwtLegend* legend = new QwtLegend( m_plotWidget );
            m_plotWidget->insertLegend( legend, QwtPlot::RightLegend );
            m_plotWidget->setLegendFontSize( legendFontSize() );
            m_plotWidget->updateLegend();
        }

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

    m_plotWidget->setAxisTitleText( QwtPlot::yLeft, completeAddressText() );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::yLeft, axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );

    double yRangeWidth = m_yRange.second - m_yRange.first;
    m_plotWidget->setAxisRange( QwtPlot::yLeft, m_yRange.first - yRangeWidth * 0.1, m_yRange.second + yRangeWidth * 0.1 );

    m_plotWidget->setAxisTitleText( QwtPlot::xBottom, m_ensembleParameter );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xBottom, axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );

    double xRangeWidth = m_xRange.second - m_xRange.first;
    m_plotWidget->setAxisRange( QwtPlot::xBottom, m_xRange.first - xRangeWidth * 0.1, m_xRange.second + xRangeWidth * 0.1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList caseNamesOfValidEnsembleCases( const RimSummaryCaseCollection* ensemble )
{
    QStringList caseNames;
    for ( auto summaryCase : ensemble->allSummaryCases() )
    {
        RifSummaryReaderInterface* reader = summaryCase->summaryReader();

        if ( !reader ) continue;
        if ( !summaryCase->caseRealizationParameters() ) continue;

        caseNames.push_back( summaryCase->displayCaseName() );
    }
    return caseNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::createPoints()
{
    detachAllCurves();

    time_t selectedTimestep = m_timeStep().toSecsSinceEpoch();

    caf::ColorTable colorTable = RiaColorTables::categoryPaletteColors();

    if ( ensembles().empty() ) return;
    if ( addresses().empty() ) return;

    bool showEnsembleName = ensembles().size() > 1u;
    bool showAddressName  = addresses().size() > 1u;

    m_xRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
    m_yRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );

    int ensembleIdx = 0;
    for ( auto ensemble : ensembles() )
    {
        int addressIdx = 0;
        for ( auto address : addresses() )
        {
            EnsembleParameter parameter = ensembleParameter( m_ensembleParameter );
            if ( !( parameter.isNumeric() && parameter.isValid() ) ) return;

            QStringList caseNames      = caseNamesOfValidEnsembleCases( ensemble );
            QString     commonCaseRoot = RiaTextStringTools::commonRoot( caseNames );

            std::set<RimSummaryCase*> activeCases = filterEnsembleCases( ensemble );

            for ( size_t caseIdx = 0u; caseIdx < ensemble->allSummaryCases().size(); ++caseIdx )
            {
                auto summaryCase = ensemble->allSummaryCases()[caseIdx];
                if ( activeCases.count( summaryCase ) == 0 ) continue;

                RifSummaryReaderInterface* reader = summaryCase->summaryReader();
                if ( !reader ) continue;

                if ( !summaryCase->caseRealizationParameters() ) continue;

                std::vector<double> values;

                double closestValue    = std::numeric_limits<double>::infinity();
                time_t closestTimeStep = 0;
                if ( reader->values( address, &values ) )
                {
                    const std::vector<time_t>& timeSteps = reader->timeSteps( address );
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
                    std::vector<double> caseValuesAtTimestep;
                    std::vector<double> parameterValues;

                    caseValuesAtTimestep.push_back( closestValue );
                    double paramValue = parameter.values[caseIdx].toDouble();
                    parameterValues.push_back( paramValue );

                    m_xRange.first  = std::min( m_xRange.first, paramValue );
                    m_xRange.second = std::max( m_xRange.second, paramValue );

                    m_yRange.first  = std::min( m_yRange.first, closestValue );
                    m_yRange.second = std::max( m_yRange.second, closestValue );

                    RiuQwtPlotCurve* plotCurve = new RiuQwtPlotCurve;
                    plotCurve->setSamples( parameterValues.data(), caseValuesAtTimestep.data(), (int)parameterValues.size() );
                    plotCurve->setStyle( QwtPlotCurve::NoCurve );
                    RiuQwtSymbol* symbol =
                        new RiuQwtSymbol( RiuQwtSymbol::cycledSymbolStyle( ensembleIdx, addressIdx ), "" );
                    symbol->setSize( legendFontSize(), legendFontSize() );
                    symbol->setColor( colorTable.cycledQColor( caseIdx ) );
                    plotCurve->setSymbol( symbol );
                    QStringList curveName;
                    if ( showEnsembleName ) curveName += ensemble->name();
                    curveName += summaryCase->displayCaseName().replace( commonCaseRoot, "" );
                    if ( showAddressName ) curveName += QString::fromStdString( address.uiText() );

                    plotCurve->setTitle( curveName.join( " - " ) );

                    plotCurve->attach( m_plotWidget );
                }
            }
            addressIdx++;
        }
        ensembleIdx++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle && !ensembles().empty() )
    {
        auto ensemble = *ensembles().begin();

        QString vectorName = completeAddressText();

        m_description = QString( "Cross Plot %1, %2 x %3 at %4" )
                            .arg( ensemble->name() )
                            .arg( m_ensembleParameter )
                            .arg( vectorName )
                            .arg( timeStepString() );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
    m_plotWidget->setPlotTitleFontSize( titleFontSize() );
}

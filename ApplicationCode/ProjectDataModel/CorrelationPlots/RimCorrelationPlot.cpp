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

#include "RimCorrelationPlot.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"
#include "RiaStatisticsTools.h"
#include "RiuGroupedBarChartBuilder.h"
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

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_scale_draw.h"

#include <limits>
#include <map>
#include <set>

namespace caf
{
template <>
void caf::AppEnum<RimCorrelationPlot::CorrelationFactor>::setUp()
{
    addItem( RimCorrelationPlot::CorrelationFactor::PEARSON, "PEARSON", "Pearson Correlation Coefficient" );
    addItem( RimCorrelationPlot::CorrelationFactor::SPEARMAN, "SPEARMAN", "Spearman's Rank Correlation Coefficient" );
    setDefault( RimCorrelationPlot::CorrelationFactor::PEARSON );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCorrelationPlot, "CorrelationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot::RimCorrelationPlot()
    : RimPlot()
{
    CAF_PDM_InitObject( "Correlation Plot", ":/CorrelationPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddressUiField, "SelectedVariableDisplayVar", "Vector", "", "", "" );
    m_summaryAddressUiField.xmlCapability()->disableIO();
    m_summaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "SummaryAddress", "Summary Address", "", "", "" );
    m_summaryAddress.uiCapability()->setUiHidden( true );
    m_summaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_pushButtonSelectSummaryAddress );
    m_pushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_pushButtonSelectSummaryAddress = false;

    m_summaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step", "", "", "" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_correlationFactor, "CorrelationFactor", "Correlation Factor", "", "", "" );
    m_correlationFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_showAbsoluteValues, "CorrelationAbsValues", true, "Show Absolute Values", "", "", "" );
    CAF_PDM_InitField( &m_sortByAbsoluteValues, "CorrelationAbsSorting", true, "Sort by Absolute Values", "", "", "" );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_useAutoPlotTitle, "AutoTitle", true, "Automatic Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_description, "PlotTitle", QString( "Correlation Plot" ), "Custom Plot Title", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot::~RimCorrelationPlot()
{
    removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_pushButtonSelectSummaryAddress )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimSummaryCaseCollection*       candidateEnsemble = m_ensemble;
        RifEclipseSummaryAddress        candicateAddress  = m_summaryAddress->address();

        dlg.hideSummaryCases();
        dlg.setEnsembleAndAddress( candidateEnsemble, candicateAddress );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_summaryAddress->setAddress( curveSelection[0].summaryAddress() );

                this->loadDataAndUpdate();
                this->updateConnectedEditors();
            }
        }

        m_pushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_timeStep )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_ensemble )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_correlationFactor || changedField == &m_showAbsoluteValues ||
              changedField == &m_sortByAbsoluteValues )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
    else if ( changedField == &m_showPlotTitle || changedField == &m_useAutoPlotTitle || changedField == &m_description )
    {
        this->updatePlotTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );
    curveDataGroup->add( &m_ensemble );
    curveDataGroup->add( &m_summaryAddressUiField );
    curveDataGroup->add( &m_pushButtonSelectSummaryAddress, { false, 1, 0 } );
    curveDataGroup->add( &m_timeStep );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_correlationFactor );
    plotGroup->add( &m_showAbsoluteValues );
    if ( !m_showAbsoluteValues() )
    {
        plotGroup->add( &m_sortByAbsoluteValues );
    }
    plotGroup->add( &m_showPlotTitle );
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->m_buttonText = "...";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCorrelationPlot::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCorrelationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    if ( fieldNeedingOptions == &m_ensemble )
    {
        RimProject*                            project = RiaApplication::instance()->project();
        std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
        project->descendantsIncludingThisOfType( summaryCaseCollections );
        for ( auto summaryCaseCollection : summaryCaseCollections )
        {
            if ( summaryCaseCollection->isEnsemble() )
            {
                options.push_back( caf::PdmOptionItemInfo( summaryCaseCollection->name(), summaryCaseCollection ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_summaryAddressUiField )
    {
        if ( m_ensemble )
        {
            RimEnsembleCurveSet::appendOptionItemsForSummaryAddresses( &options, m_ensemble );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        QString dateTimeFormat = RiaApplication::instance()->preferences()->dateTimeFormat();
        if ( m_ensemble )
        {
            std::set<time_t> allTimeSteps = allAvailableTimeSteps();
            for ( time_t timeStep : allTimeSteps )
            {
                QDateTime dateTime = QDateTime::fromTime_t( timeStep );
                options.push_back( caf::PdmOptionItemInfo( dateTime.toString( dateTimeFormat ), dateTime ) );
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimCorrelationPlot::allAvailableTimeSteps()
{
    std::set<time_t> timeStepUnion;

    for ( RimSummaryCase* sumCase : m_ensemble->allSummaryCases() )
    {
        const std::vector<time_t>& timeSteps = sumCase->summaryReader()->timeSteps( m_summaryAddress->address() );

        for ( time_t t : timeSteps )
        {
            timeStepUnion.insert( t );
        }
    }

    return timeStepUnion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCorrelationPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_summaryAddressUiField = m_summaryAddress->address();

    if ( m_plotWidget )
    {
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotScale );

        RiuGroupedBarChartBuilder chartBuilder;

        // buildTestPlot( chartBuilder );
        addDataToChartBuilder( chartBuilder );

        chartBuilder.addBarChartToPlot( m_plotWidget, Qt::Horizontal );

        m_plotWidget->insertLegend( nullptr );
        m_plotWidget->updateLegend();

        this->updateAxes();
        this->updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimCorrelationPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCorrelationPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                        int                         oldFontSize,
                                        int                         fontSize,
                                        bool                        forceChange /*= false */ )
{
    bool anyChange = false;

    return anyChange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimCorrelationPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
        // updatePlotTitle();
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimCorrelationPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisTitle( QwtPlot::yLeft, "Parameter" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
    m_plotWidget->setAxisTitle( QwtPlot::xBottom, "Pearson Correlation Coefficient" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisRange( QwtPlot::xBottom, -1.0, 1.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::onAxisSelected( int axis, bool toggle )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::cleanupBeforeClose()
{
    detachAllCurves();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder )
{
    time_t selectedTimestep = m_timeStep().toTime_t();

    if ( !m_ensemble ) return;

    if ( !m_summaryAddress ) return;

    std::vector<EnsembleParameter> ensembleParameters = m_ensemble->variationSortedEnsembleParameters();

    std::vector<double>                    caseValuesAtTimestep;
    std::map<QString, std::vector<double>> parameterValues;

    for ( size_t caseIdx = 0u; caseIdx < m_ensemble->allSummaryCases().size(); ++caseIdx )
    {
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
                if ( timeSteps[i] >= selectedTimestep && timeSteps[i] - selectedTimestep < selectedTimestep - closestTimeStep )
                {
                    closestValue    = values[i];
                    closestTimeStep = timeSteps[i];
                }
            }
        }
        if ( closestValue != std::numeric_limits<double>::infinity() )
        {
            caseValuesAtTimestep.push_back( closestValue );

            for ( auto parameter : ensembleParameters )
            {
                if ( parameter.isNumeric() && parameter.isValid() )
                {
                    double paramValue = parameter.values[caseIdx].toDouble();
                    parameterValues[parameter.name].push_back( paramValue );
                }
            }
        }
    }

    std::vector<std::pair<QString, double>> correlationResults;
    for ( auto parameterValuesPair : parameterValues )
    {
        double correlation = 0.0;
        if ( m_correlationFactor == CorrelationFactor::PEARSON )
        {
            correlation = RiaStatisticsTools::pearsonCorrelation( parameterValuesPair.second, caseValuesAtTimestep );
        }
        else
        {
            correlation = RiaStatisticsTools::spearmanCorrelation( parameterValuesPair.second, caseValuesAtTimestep );
        }
        correlationResults.push_back( std::make_pair( parameterValuesPair.first, correlation ) );
    }

    QString timestepString = m_timeStep().toString( RiaApplication::instance()->preferences()->dateTimeFormat() );

    for ( auto parameterCorrPair : correlationResults )
    {
        double  value     = m_showAbsoluteValues() ? std::abs( parameterCorrPair.second ) : parameterCorrPair.second;
        double  sortValue = m_sortByAbsoluteValues() ? std::abs( value ) : value;
        QString barText   = parameterCorrPair.first;
        QString majorText = "", medText = "", minText = "", legendText = barText;
        chartBuilder.addBarEntry( majorText, medText, minText, sortValue, legendText, barText, value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle )
    {
        m_description = QString( "%1 for %2" )
                            .arg( m_correlationFactor().uiText() )
                            .arg( QString::fromStdString( m_summaryAddressUiField().uiText() ) );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle );
}

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

#include "RifSummaryReaderInterface.h"

#include "RigEnsembleParameter.h"

#include "RimPlotDataFilterCollection.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "RiuContextMenuLauncher.h"
#include "RiuDockWidgetTools.h"
#include "RiuPlotCurve.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotRectAnnotation.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtSymbol.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiValueRangeEditor.h"

#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_text.h"

#include <QColor>
#include <QStringList>

#include <limits>
#include <set>

CAF_PDM_SOURCE_INIT( RimParameterResultCrossPlot, "ParameterResultCrossPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot::RimParameterResultCrossPlot()
    : RimAbstractCorrelationPlot()
    , m_xValueRange( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() )
    , m_yValueRange( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() )
{
    CAF_PDM_InitObject( "ParameterResultCross Plot", ":/CorrelationCrossPlot16x16.png" );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Ensemble Parameter" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_useParameterFilter, "UseParameterFilter", false, "Use Parameter Filter" );
    CAF_PDM_InitField( &m_summaryFilterRange, "SummaryFilterRange", std::make_pair( 0.0, 0.0 ), "Summary Value Range", "", "", "" );
    CAF_PDM_InitField( &m_parameterFilterRange, "ParameterFilterRange", std::make_pair( 0.0, 0.0 ), "Parameter Value Range", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_excludedCasesText, "ExcludedCasesText", "Excluded Cases" );
    m_excludedCasesText.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_excludedCasesText.registerGetMethod( this, &RimParameterResultCrossPlot::excludedCasesText );

    m_selectMultipleVectors = true;

    m_legendFontSize = caf::FontTools::RelativeSize::Small;

    m_xValueRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
    m_yValueRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );

    m_showPlotLegends = false;
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
    writeDataToCache();

    m_ensembleParameter = ensembleParameter;

    updateValueRanges();
    updateFilterRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterResultCrossPlot::ensembleParameter() const
{
    return m_ensembleParameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimParameterResultCrossPlot::summaryCasesExcludedByFilter() const
{
    if ( ensembles().size() != 1 ) return {};

    if ( !m_useParameterFilter() ) return {};

    std::vector<RimSummaryCase*> cases;

    auto plotValues = createCaseData();
    for ( const auto& caseData : plotValues )
    {
        if ( caseData.summaryValue < m_summaryFilterRange().first || caseData.summaryValue > m_summaryFilterRange().second ||
             caseData.parameterValue < m_parameterFilterRange().first || caseData.parameterValue > m_parameterFilterRange().second )
        {
            cases.push_back( caseData.summaryCase );
        }
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::appendFilterFields( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_useParameterFilter );
    if ( m_useParameterFilter )
    {
        uiOrdering.add( &m_summaryFilterRange );
        uiOrdering.add( &m_parameterFilterRange );
        uiOrdering.add( &m_excludedCasesText );
    }
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimAbstractCorrelationPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_ensembleParameter )
    {
        loadDataAndUpdate();
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    appendDataSourceFields( uiConfigName, uiOrdering );

    caf::PdmUiGroup* crossPlotGroup = uiOrdering.addNewGroup( "Cross Plot Parameters" );
    crossPlotGroup->add( &m_ensembleParameter );

    auto filterGroup = uiOrdering.addNewGroup( "Filter" );
    appendFilterFields( *filterGroup );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->setCollapsedByDefault();
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
QList<caf::PdmOptionItemInfo> RimParameterResultCrossPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimAbstractCorrelationPlot::calculateValueOptions( fieldNeedingOptions );
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
void RimParameterResultCrossPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_summaryFilterRange )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_decimals        = 4;
            attr->m_sliderTickCount = 20;

            attr->m_minimum = m_yValueRange.first;
            attr->m_maximum = m_yValueRange.second;
        }
    }
    else if ( field == &m_parameterFilterRange )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            attr->m_decimals        = 4;
            attr->m_sliderTickCount = 20;

            attr->m_minimum = m_xValueRange.first;
            attr->m_maximum = m_xValueRange.second;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_selectedVarsUiField = selectedVectorNamesText();

    if ( m_plotWidget && m_analyserOfSelectedCurveDefs )
    {
        createPoints();

        if ( m_useParameterFilter )
        {
            if ( !m_rectAnnotation )
            {
                m_rectAnnotation = std::make_unique<RiuQwtPlotRectAnnotation>();
            }

            m_rectAnnotation->setInterval( m_parameterFilterRange().first,
                                           m_parameterFilterRange().second,
                                           m_summaryFilterRange().first,
                                           m_summaryFilterRange().second );

            QColor penColor( "orange" );
            QPen   pen( penColor );
            m_rectAnnotation->setPen( pen );

            QColor brushColor   = penColor;
            double transparency = 0.3;
            brushColor.setAlphaF( transparency );
            QBrush brush( brushColor );
            m_rectAnnotation->setBrush( brush );

            m_rectAnnotation->attach( m_plotWidget->qwtPlot() );
        }

        updateValueRanges();
        if ( m_xValueRange.first != std::numeric_limits<double>::infinity() )
        {
            writeDataToCache();
        }

        updateAxes();

        updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), completeAddressText() );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );
    m_plotWidget->setAxisRange( RiuPlotAxis::defaultLeft(), m_yValueRange.first, m_yValueRange.second );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), m_ensembleParameter );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );
    m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), m_xValueRange.first, m_xValueRange.second );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList caseNamesOfValidEnsembleCases( const RimSummaryEnsemble* ensemble )
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

    caf::ColorTable colorTable = RiaColorTables::categoryPaletteColors();

    if ( ensembles().empty() ) return;
    if ( addresses().empty() ) return;

    bool showEnsembleName = ensembles().size() > 1u;

    std::set<RimSummaryCase*> selectedSummaryCases;
    auto selectedTreeViewItems = RiuDockWidgetTools::selectedItemsInTreeView( RiuDockWidgetTools::plotMainWindowDataSourceTreeName() );
    for ( auto item : selectedTreeViewItems )
    {
        if ( auto summaryCase = dynamic_cast<RimSummaryCase*>( item ) )
        {
            selectedSummaryCases.insert( summaryCase );
        }
    }

    int  idx      = 0;
    auto caseData = createCaseData();
    for ( const auto& [paramValue, closestValue, summaryCase] : caseData )
    {
        RiuQwtPlotCurve* plotCurve = new RiuQwtPlotCurve;
        plotCurve->setSamplesValues( { paramValue }, { closestValue } );
        plotCurve->setStyle( QwtPlotCurve::NoCurve );

        if ( selectedSummaryCases.contains( summaryCase ) )
        {
            RiuQwtSymbol* symbol   = new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );
            auto          fontSize = legendFontSize();
            symbol->setSize( fontSize, fontSize );
            symbol->setColor( colorTable.cycledQColor( idx++ ) );
            plotCurve->setSymbol( symbol );
        }
        else
        {
            RiuQwtSymbol* symbol = new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_ELLIPSE );

            auto fontSize = legendFontSize();
            symbol->setSize( fontSize, fontSize );
            symbol->setColor( colorTable.cycledQColor( idx++ ) );
            plotCurve->setSymbol( symbol );
        }

        QStringList curveName;
        if ( showEnsembleName && summaryCase->ensemble() )
        {
            curveName += summaryCase->ensemble()->name();
        }
        curveName += summaryCase->displayCaseName();

        plotCurve->setTitle( curveName.join( " - " ) );

        plotCurve->attach( m_plotWidget->qwtPlot() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::updateValueRanges()
{
    auto parameterMin = std::numeric_limits<double>::infinity();
    auto parameterMax = -std::numeric_limits<double>::infinity();
    auto summaryMin   = std::numeric_limits<double>::infinity();
    auto summaryMax   = -std::numeric_limits<double>::infinity();

    auto caseData = createCaseData();
    for ( const auto& [paramValue, summaryValue, summaryCase] : caseData )
    {
        parameterMin = std::min( parameterMin, paramValue );
        parameterMax = std::max( parameterMax, paramValue );
        summaryMin   = std::min( summaryMin, summaryValue );
        summaryMax   = std::max( summaryMax, summaryValue );
    }

    double parameterRange = parameterMax - parameterMin;
    double summaryRange   = summaryMax - summaryMin;

    m_xValueRange = { parameterMin - parameterRange * 0.1, parameterMax + parameterRange * 0.1 };
    m_yValueRange = { summaryMin - summaryRange * 0.1, summaryMax + summaryRange * 0.1 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::updateFilterRanges()
{
    const auto hashValue = hashFromCurrentData();
    if ( m_filterValueRangeCache.contains( hashValue ) )
    {
        const auto& cachedRanges = m_filterValueRangeCache[hashValue];
        m_parameterFilterRange   = { cachedRanges[0], cachedRanges[1] };
        m_summaryFilterRange     = { cachedRanges[2], cachedRanges[3] };
    }
    else
    {
        m_summaryFilterRange   = m_yValueRange;
        m_parameterFilterRange = m_xValueRange;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterResultCrossPlot::excludedCasesText() const
{
    auto        cases = summaryCasesExcludedByFilter();
    QStringList caseNames;
    for ( auto summaryCase : cases )
    {
        caseNames.push_back( summaryCase->displayCaseName() );
    }
    return caseNames.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::detachAllCurves()
{
    if ( m_plotWidget && m_rectAnnotation )
    {
        m_rectAnnotation->detach();
    }

    RimAbstractCorrelationPlot::detachAllCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimParameterResultCrossPlot::CaseData> RimParameterResultCrossPlot::createCaseData() const
{
    RigEnsembleParameter parameter = ensembleParameterByName( m_ensembleParameter );
    if ( !( parameter.isNumeric() && parameter.isValid() ) ) return {};

    std::vector<RimParameterResultCrossPlot::CaseData> caseData;

    time_t selectedTimestep = m_timeStep().toSecsSinceEpoch();

    for ( auto ensemble : ensembles() )
    {
        for ( auto address : addresses() )
        {
            std::set<RimSummaryCase*> activeCases = filterEnsembleCases( ensemble );

            for ( size_t caseIdx = 0u; caseIdx < ensemble->allSummaryCases().size(); ++caseIdx )
            {
                auto summaryCase = ensemble->allSummaryCases()[caseIdx];
                if ( activeCases.count( summaryCase ) == 0 ) continue;

                RifSummaryReaderInterface* reader = summaryCase->summaryReader();
                if ( !reader ) continue;

                if ( !summaryCase->caseRealizationParameters() ) continue;

                double summaryValue        = std::numeric_limits<double>::infinity();
                time_t closestTimeStep     = 0;
                auto [isOk, summaryValues] = reader->values( address );
                if ( isOk )
                {
                    const std::vector<time_t>& timeSteps = reader->timeSteps( address );
                    for ( size_t i = 0; i < timeSteps.size(); ++i )
                    {
                        if ( timeDiff( timeSteps[i], selectedTimestep ) < timeDiff( selectedTimestep, closestTimeStep ) )
                        {
                            summaryValue    = summaryValues[i];
                            closestTimeStep = timeSteps[i];
                        }
                    }
                }

                if ( summaryValue != std::numeric_limits<double>::infinity() )
                {
                    caseData.push_back(
                        { .parameterValue = parameter.values[caseIdx].toDouble(), .summaryValue = summaryValue, .summaryCase = summaryCase } );
                }
            }
        }
    }
    return caseData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimParameterResultCrossPlot::hashFromCurrentData() const
{
    return RiaHashTools::hash( m_ensembleParameter().toStdString(),
                               m_xValueRange.first,
                               m_xValueRange.second,
                               m_yValueRange.first,
                               m_yValueRange.second );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterResultCrossPlot::writeDataToCache()
{
    const auto hashValue               = hashFromCurrentData();
    m_filterValueRangeCache[hashValue] = { m_parameterFilterRange().first,
                                           m_parameterFilterRange().second,
                                           m_summaryFilterRange().first,
                                           m_summaryFilterRange().second };
}

namespace internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class CurveTracker : public QwtPlotPicker
{
public:
    CurveTracker( QwtPlot* plot )
        : QwtPlotPicker( plot->canvas() )
    {
        setStateMachine( new QwtPickerTrackerMachine() );
        setRubberBand( QwtPicker::NoRubberBand );
        setTrackerMode( QwtPicker::AlwaysOn );
    }

protected:
    QwtText trackerText( const QPoint& pos ) const override
    {
        double  minDistance = std::numeric_limits<double>::max();
        QString closestCurveLabel;

        // Ok to access reference to list, no add/remove of item during iteration
        for ( QwtPlotItem* item : plot()->itemList() )
        {
            if ( item->rtti() == QwtPlotItem::Rtti_PlotCurve )
            {
                auto   curve    = static_cast<QwtPlotCurve*>( item );
                double distance = std::numeric_limits<double>::max();
                curve->closestPoint( pos, &distance );

                if ( distance < minDistance )
                {
                    minDistance       = distance;
                    closestCurveLabel = curve->title().text();
                }
            }
        }

        if ( minDistance < 20.0 )
        {
            QwtText text( closestCurveLabel );
            text.setBackgroundBrush( QBrush( Qt::white ) );
            text.setColor( Qt::black );
            return text;
        }

        return QwtText();
    }
};
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimParameterResultCrossPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
        updatePlotTitle();

        new RiuContextMenuLauncher( m_plotWidget,
                                    { "RicShowPlotDataFeature",
                                      "RicCreateEnsembleFromFilteredCasesFeature",
                                      "RicCreateHistogramForEnsembleParameterFeature" } );
    }

    if ( m_plotWidget )
    {
        // Add a curve tracker to display the name of the realization when mouse is close to a sample in the cross plot
        new internal::CurveTracker( m_plotWidget->qwtPlot() );
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterResultCrossPlot::asciiDataForPlotExport() const
{
    QString asciiData;

    auto data = createCaseData();

    asciiData += "Realization\tParameter\tResult\n";
    for ( const auto& [parameterValue, summaryValue, summaryCase] : data )
    {
        auto caseName = summaryCase->displayCaseName();
        asciiData += QString( "%1\t%2\t%3\n" ).arg( caseName ).arg( parameterValue ).arg( summaryValue );
    }

    return asciiData;
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

        m_description =
            QString( "%1 x %2, %3 at %4" ).arg( vectorName ).arg( m_ensembleParameter ).arg( ensemble->name() ).arg( timeStepString() );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
    m_plotWidget->setPlotTitleFontSize( titleFontSize() );
}

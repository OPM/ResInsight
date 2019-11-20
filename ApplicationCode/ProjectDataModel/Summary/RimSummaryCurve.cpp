/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryCurve.h"

#include "RiaCurveMerger.h"
#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiaStatisticsTools.h"

#include "RifReaderEclipseSummary.h"

#include "RimEclipseResultCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimTools.h"

#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotCurve.h"
#include "RiuSummaryCurveDefSelectionDialog.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_date.h"
#include "qwt_plot.h"

#include "cafPdmUiLineEditor.h"
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimSummaryCurve, "SummaryCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject( "Summary Curve", ":/SummaryCurve16x16.png", "", "" );

    // Y Values

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryCase, "SummaryCase", "Case", "", "", "" );
    m_yValuesSummaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddressUiField, "SelectedVariableDisplayVar", "Vector", "", "", "" );
    m_yValuesSummaryAddressUiField.xmlCapability()->disableIO();
    m_yValuesSummaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddress, "SummaryAddress", "Summary Address", "", "", "" );
    m_yValuesSummaryAddress.uiCapability()->setUiHidden( true );
    m_yValuesSummaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_yPushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_yPushButtonSelectSummaryAddress );
    m_yPushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_yPushButtonSelectSummaryAddress = false;

    m_yValuesSummaryAddress = new RimSummaryAddress;

    // X Values

    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryCase, "SummaryCaseX", "Case", "", "", "" );
    m_xValuesSummaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_xValuesSummaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryAddressUiField, "SelectedVariableDisplayVarX", "Vector", "", "", "" );
    m_xValuesSummaryAddressUiField.xmlCapability()->disableIO();
    m_xValuesSummaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryAddress, "SummaryAddressX", "Summary Address", "", "", "" );
    m_xValuesSummaryAddress.uiCapability()->setUiHidden( true );
    m_xValuesSummaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_xPushButtonSelectSummaryAddress, "SelectAddressX", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_xPushButtonSelectSummaryAddress );
    m_xPushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_xPushButtonSelectSummaryAddress = false;

    m_xValuesSummaryAddress = new RimSummaryAddress;

    // Other members

    CAF_PDM_InitFieldNoDefault( &m_plotAxis, "PlotAxis", "Axis", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_curveNameConfig, "SummaryCurveNameConfig", "SummaryCurveNameConfig", "", "", "" );
    m_curveNameConfig.uiCapability()->setUiHidden( true );
    m_curveNameConfig.uiCapability()->setUiTreeChildrenHidden( true );

    m_curveNameConfig = new RimSummaryCurveAutoName;

    CAF_PDM_InitField( &m_isTopZWithinCategory, "isTopZWithinCategory", false, "", "", "", "" );
    m_isTopZWithinCategory.uiCapability()->setUiHidden( true );

    m_symbolSkipPixelDistance = 10.0f;
    m_curveThickness          = 2;

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryFilter_OBSOLETE, "VarListFilter", "Filter", "", "", "" );
    m_yValuesSummaryFilter_OBSOLETE.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryFilter_OBSOLETE.uiCapability()->setUiHidden( true );
    m_yValuesSummaryFilter_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_yValuesSummaryFilter_OBSOLETE = new RimSummaryFilter_OBSOLETE;

    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryFilter_OBSOLETE, "VarListFilterX", "Filter", "", "", "" );
    m_xValuesSummaryFilter_OBSOLETE.uiCapability()->setUiTreeChildrenHidden( true );
    m_xValuesSummaryFilter_OBSOLETE.uiCapability()->setUiHidden( true );
    m_xValuesSummaryFilter_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_xValuesSummaryFilter_OBSOLETE = new RimSummaryFilter_OBSOLETE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::~RimSummaryCurve() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryCaseY( RimSummaryCase* sumCase )
{
    if ( m_yValuesSummaryCase != sumCase )
    {
        m_qwtPlotCurve->clearErrorBars();
    }

    m_yValuesSummaryCase = sumCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCurve::summaryCaseY() const
{
    return m_yValuesSummaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::summaryAddressX() const
{
    return m_xValuesSummaryAddress->address();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddressX( const RifEclipseSummaryAddress& address )
{
    m_xValuesSummaryAddress->setAddress( address );

    // TODO: Should interpolation be computed similar to RimSummaryCurve::setSummaryAddressY
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::summaryAddressY() const
{
    return m_yValuesSummaryAddress->address();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddressYAndApplyInterpolation( const RifEclipseSummaryAddress& address )
{
    setSummaryAddressY( address );

    calculateCurveInterpolationFromAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryAddressY( const RifEclipseSummaryAddress& address )
{
    if ( m_yValuesSummaryAddress->address() != address )
    {
        m_qwtPlotCurve->clearErrorBars();
    }

    m_yValuesSummaryAddress->setAddress( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCurve::unitNameY() const
{
    RifSummaryReaderInterface* reader = valuesSummaryReaderY();
    if ( reader ) return reader->unitName( this->summaryAddressY() );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCurve::unitNameX() const
{
    RifSummaryReaderInterface* reader = valuesSummaryReaderX();
    if ( reader ) return reader->unitName( this->summaryAddressX() );

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::valuesY() const
{
    std::vector<double> values;

    RifSummaryReaderInterface* reader = valuesSummaryReaderY();

    if ( !reader ) return values;

    RifEclipseSummaryAddress addr = m_yValuesSummaryAddress()->address();
    reader->values( addr, &values );

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );
    bool isNormalized = plot->isNormalizationEnabled();
    if ( isNormalized )
    {
        auto   minMaxPair = std::minmax_element( values.begin(), values.end() );
        double min        = *minMaxPair.first;
        double max        = *minMaxPair.second;
        double range      = max - min;

        for ( double& v : values )
        {
            v = ( v - min ) / range;
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimSummaryCurve::errorSummaryAddressY() const
{
    auto addr = summaryAddressY();
    addr.setAsErrorResult();
    return addr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::errorValuesY() const
{
    std::vector<double> values;

    RifSummaryReaderInterface* reader = valuesSummaryReaderY();

    if ( !reader ) return values;

    RifEclipseSummaryAddress addr = errorSummaryAddressY();
    if ( reader->hasAddress( addr ) )
    {
        reader->values( addr, &values );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCurve::valuesX() const
{
    std::vector<double> values;

    if ( m_xValuesSummaryCase() && m_xValuesSummaryCase()->summaryReader() )
    {
        RifSummaryReaderInterface* reader = m_xValuesSummaryCase()->summaryReader();

        RifEclipseSummaryAddress addr = m_xValuesSummaryAddress()->address();
        reader->values( addr, &values );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCurve::timeStepsY() const
{
    static std::vector<time_t> emptyVector;
    RifSummaryReaderInterface* reader = valuesSummaryReaderY();

    if ( !reader ) return emptyVector;

    RifEclipseSummaryAddress addr = m_yValuesSummaryAddress()->address();

    return reader->timeSteps( addr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryCaseX( RimSummaryCase* sumCase )
{
    m_xValuesSummaryCase = sumCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCurve::summaryCaseX() const
{
    return m_xValuesSummaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setLeftOrRightAxisY( RiaDefines::PlotAxis plotAxis )
{
    m_plotAxis = plotAxis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimSummaryCurve::axisY() const
{
    return m_plotAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = this->RimPlotCurve::calculateValueOptions( fieldNeedingOptions,
                                                                                       useOptionsOnly );
    if ( !options.isEmpty() ) return options;

    if ( fieldNeedingOptions == &m_yValuesSummaryCase || fieldNeedingOptions == &m_xValuesSummaryCase )
    {
        RimProject* proj = RiaApplication::instance()->project();

        std::vector<RimSummaryCase*> cases = proj->allSummaryCases();

        cases.push_back( proj->calculationCollection->calculationSummaryCase() );

        for ( RimSummaryCase* rimCase : cases )
        {
            options.push_back( caf::PdmOptionItemInfo( rimCase->caseName(), rimCase ) );
        }

        if ( options.size() > 0 )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( &m_yValuesSummaryAddressUiField == fieldNeedingOptions )
    {
        appendOptionItemsForSummaryAddresses( &options, m_yValuesSummaryCase() );
    }
    else if ( &m_xValuesSummaryAddressUiField == fieldNeedingOptions )
    {
        appendOptionItemsForSummaryAddresses( &options, m_xValuesSummaryCase() );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurve::createCurveAutoName()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );

    const RimSummaryPlotNameHelper* nameHelper = plot->activePlotTitleHelperAllCurves();
    QString curveName = m_curveNameConfig->curveNameY( m_yValuesSummaryAddress->address(), nameHelper );
    if ( curveName.isEmpty() )
    {
        curveName = m_curveNameConfig->curveNameY( m_yValuesSummaryAddress->address(), nullptr );
    }

    if ( isCrossPlotCurve() )
    {
        QString curveNameX = m_curveNameConfig->curveNameX( m_xValuesSummaryAddress->address(), nameHelper );
        if ( curveNameX.isEmpty() )
        {
            curveNameX = m_curveNameConfig->curveNameX( m_xValuesSummaryAddress->address(), nullptr );
        }

        if ( !curveName.isEmpty() || !curveNameX.isEmpty() )
        {
            curveName += " | " + curveNameX;
        }
    }

    if ( curveName.isEmpty() )
    {
        curveName = "Curve Name Placeholder";
    }

    return curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateZoomInParentPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );

    plot->updateZoomInQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

    m_yValuesSummaryAddressUiField = m_yValuesSummaryAddress->address();
    m_xValuesSummaryAddressUiField = m_xValuesSummaryAddress->address();

    updateConnectedEditors();

    setZIndexFromCurveInfo();

    if ( isCurveVisible() )
    {
        std::vector<double> curveValuesY = this->valuesY();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted( plot );
        bool isLogCurve = plot->isLogarithmicScaleEnabled( this->axisY() );

        bool shouldPopulateViewWithEmptyData = false;

        if ( isCrossPlotCurve() )
        {
            auto curveValuesX    = this->valuesX();
            auto curveTimeStepsX = timeStepsX();

            auto curveTimeStepsY = timeStepsY();

            if ( curveValuesY.empty() || curveValuesX.empty() )
            {
                shouldPopulateViewWithEmptyData = true;
            }
            else
            {
                RiaTimeHistoryCurveMerger curveMerger;
                curveMerger.addCurveData( curveTimeStepsX, curveValuesX );
                curveMerger.addCurveData( curveTimeStepsY, curveValuesY );
                curveMerger.computeInterpolatedValues();

                if ( curveMerger.allXValues().size() > 0 )
                {
                    m_qwtPlotCurve->setSamplesFromXValuesAndYValues( curveMerger.interpolatedYValuesForAllXValues( 0 ),
                                                                     curveMerger.interpolatedYValuesForAllXValues( 1 ),
                                                                     isLogCurve );
                }
                else
                {
                    shouldPopulateViewWithEmptyData = true;
                }
            }
        }
        else
        {
            std::vector<time_t> curveTimeStepsY = this->timeStepsY();
            if ( curveTimeStepsY.size() > 0 && curveTimeStepsY.size() == curveValuesY.size() )
            {
                if ( plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE )
                {
                    auto reader = summaryCaseY()->summaryReader();
                    if ( reader )
                    {
                        auto errAddress = reader->errorAddress( summaryAddressY() );
                        if ( errAddress.isValid() )
                        {
                            std::vector<double> errValues;
                            reader->values( errAddress, &errValues );
                            m_qwtPlotCurve->setSamplesFromTimeTAndYValues( curveTimeStepsY,
                                                                           curveValuesY,
                                                                           errValues,
                                                                           isLogCurve );
                        }
                        else
                        {
                            m_qwtPlotCurve->setSamplesFromTimeTAndYValues( curveTimeStepsY, curveValuesY, isLogCurve );
                        }
                    }
                }
                else
                {
                    double timeScale = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

                    std::vector<double> timeFromSimulationStart;
                    if ( curveTimeStepsY.size() )
                    {
                        time_t startDate = curveTimeStepsY[0];
                        for ( const auto& date : curveTimeStepsY )
                        {
                            timeFromSimulationStart.push_back( timeScale * ( date - startDate ) );
                        }
                    }

                    m_qwtPlotCurve->setSamplesFromXValuesAndYValues( timeFromSimulationStart, curveValuesY, isLogCurve );
                }
            }
            else
            {
                shouldPopulateViewWithEmptyData = true;
            }
        }

        if ( shouldPopulateViewWithEmptyData )
        {
            m_qwtPlotCurve->setSamplesFromXValuesAndYValues( std::vector<double>(), std::vector<double>(), isLogCurve );
        }

        if ( updateParentPlot && m_parentQwtPlot )
        {
            updateZoomInParentPlot();
            m_parentQwtPlot->replot();
        }

        m_qwtPlotCurve->showErrorBars( m_showErrorBars );
    }

    if ( updateParentPlot ) updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateLegendsInPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );
    plot->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    RimPlotCurve::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    // Reset dynamic icon
    this->setUiIcon( caf::QIconProvider() );
    // Get static one
    caf::QIconProvider iconProvider = this->uiIconProvider();
    if ( iconProvider.isNull() ) return;

    QIcon icon = iconProvider.icon();

    RimSummaryCurveCollection* coll = nullptr;
    this->firstAncestorOrThisOfType( coll );
    if ( coll && coll->curveForSourceStepping() == this )
    {
        QPixmap  combined = icon.pixmap( 16, 16 );
        QPainter painter( &combined );
        QPixmap  updownpixmap( ":/StepUpDownCorner16x16.png" );
        painter.drawPixmap( 0, 0, updownpixmap );
        iconProvider.setPixmap( combined );
        setUiIcon( iconProvider );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_yPushButtonSelectSummaryAddress == field || &m_xPushButtonSelectSummaryAddress == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "...";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateOptionSensitivity();

    {
        QString curveDataGroupName = "Summary Vector";
        if ( isCrossPlotCurve() ) curveDataGroupName += " Y";
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroupWithKeyword( curveDataGroupName, "Summary Vector Y" );
        curveDataGroup->add( &m_yValuesSummaryCase, {true, 3, 1} );
        curveDataGroup->add( &m_yValuesSummaryAddressUiField, {true, 2, 1} );
        curveDataGroup->add( &m_yPushButtonSelectSummaryAddress, {false, 1, 0} );
        curveDataGroup->add( &m_plotAxis, {true, 3, 1} );

        if ( isCrossPlotCurve() )
            m_showErrorBars = false;
        else
            curveDataGroup->add( &m_showErrorBars );
    }

    if ( isCrossPlotCurve() )
    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector X" );
        curveDataGroup->add( &m_xValuesSummaryCase, {true, 3, 1} );
        curveDataGroup->add( &m_xValuesSummaryAddressUiField, {true, 2, 1} );
        curveDataGroup->add( &m_xPushButtonSelectSummaryAddress, {false, 1, 0} );
    }

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault( true );
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    if ( m_isUsingAutoName )
    {
        m_curveNameConfig->uiOrdering( uiConfigName, *nameGroup );
    }

    uiOrdering.skipRemainingFields(); // For now.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options,
                                                            RimSummaryCase*                summaryCase )
{
    if ( summaryCase )
    {
        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        if ( reader )
        {
            const std::set<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

            for ( auto& address : allAddresses )
            {
                if ( address.isErrorResult() ) continue;

                std::string name = address.uiText();
                QString     s    = QString::fromStdString( name );
                options->push_back( caf::PdmOptionItemInfo( s, QVariant::fromValue( address ) ) );
            }
        }

        options->push_front( caf::PdmOptionItemInfo( RiaDefines::undefinedResultName(),
                                                     QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setZIndexFromCurveInfo()
{
    auto sumAddr = summaryAddressY();
    auto sumCase = summaryCaseY();

    double zOrder = 0.0;

    if ( sumCase && sumAddr.isValid() )
    {
        if ( sumCase->isObservedData() )
        {
            zOrder = RiuQwtPlotCurve::Z_SINGLE_CURVE_OBSERVED;
        }
        else if ( sumAddr.category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
        {
            zOrder = RiuQwtPlotCurve::Z_ENSEMBLE_STAT_CURVE;
        }
        else if ( sumCase->ensemble() )
        {
            zOrder = RiuQwtPlotCurve::Z_ENSEMBLE_CURVE;
        }
        else
        {
            zOrder = RiuQwtPlotCurve::Z_SINGLE_CURVE_NON_OBSERVED;
        }
    }

    if ( m_isTopZWithinCategory )
    {
        zOrder += 1.0;
    }

    setZOrder( zOrder );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updateQwtPlotAxis()
{
    if ( m_qwtPlotCurve )
    {
        if ( this->axisY() == RiaDefines::PLOT_AXIS_LEFT )
        {
            m_qwtPlotCurve->setYAxis( QwtPlot::yLeft );
        }
        else
        {
            m_qwtPlotCurve->setYAxis( QwtPlot::yRight );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::applyCurveAutoNameSettings( const RimSummaryCurveAutoName& autoNameSettings )
{
    m_curveNameConfig->applySettings( autoNameSettings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurve::curveExportDescription( const RifEclipseSummaryAddress& address ) const
{
    auto addr = address.isValid() ? address : m_yValuesSummaryAddress->address();

    RimEnsembleCurveSetCollection* coll;
    firstAncestorOrThisOfType( coll );

    auto curveSet = coll ? coll->findRimCurveSetFromQwtCurve( m_qwtPlotCurve ) : nullptr;
    auto group    = curveSet ? curveSet->summaryCaseCollection() : nullptr;

    auto addressUiText = addr.uiText();
    if ( addr.category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
    {
        addressUiText =
            RiaStatisticsTools::replacePercentileByPValueText( QString::fromStdString( addressUiText ) ).toStdString();
    }

    if ( group && group->isEnsemble() )
    {
        return QString( "%1.%2.%3" )
            .arg( QString::fromStdString( addressUiText ) )
            .arg( m_yValuesSummaryCase->caseName() )
            .arg( group->name() );
    }
    else
    {
        return QString( "%1.%2" ).arg( QString::fromStdString( addressUiText ) ).arg( m_yValuesSummaryCase->caseName() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setCurveAppearanceFromCaseType()
{
    if ( m_yValuesSummaryCase )
    {
        if ( m_yValuesSummaryCase->isObservedData() )
        {
            setLineStyle( RiuQwtPlotCurve::STYLE_NONE );
            setSymbol( RiuQwtSymbol::SYMBOL_XCROSS );

            return;
        }
    }

    if ( m_yValuesSummaryAddress && m_yValuesSummaryAddress->address().isHistoryQuantity() )
    {
        RiaPreferences* prefs = RiaApplication::instance()->preferences();

        if ( prefs->defaultSummaryHistoryCurveStyle() == RiaPreferences::SYMBOLS )
        {
            m_symbolEdgeColor = m_curveColor;

            setSymbol( RiuQwtSymbol::SYMBOL_XCROSS );
            setLineStyle( RiuQwtPlotCurve::STYLE_NONE );
        }
        else if ( prefs->defaultSummaryHistoryCurveStyle() == RiaPreferences::SYMBOLS_AND_LINES )
        {
            m_symbolEdgeColor = m_curveColor;

            setSymbol( RiuQwtSymbol::SYMBOL_XCROSS );
            setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
        }
        else if ( prefs->defaultSummaryHistoryCurveStyle() == RiaPreferences::LINES )
        {
            setSymbol( RiuQwtSymbol::SYMBOL_NONE );
            setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
        }

        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::markCachedDataForPurge()
{
    auto reader = valuesSummaryReaderY();
    if ( reader ) reader->markForCachePurge( m_yValuesSummaryAddress->address() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setAsTopZWithinCategory( bool enable )
{
    m_isTopZWithinCategory = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    this->RimPlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    bool loadAndUpdate                     = false;
    bool crossPlotTestForMatchingTimeSteps = false;

    if ( changedField == &m_yValuesSummaryAddressUiField )
    {
        m_yValuesSummaryAddress->setAddress( m_yValuesSummaryAddressUiField() );

        this->calculateCurveInterpolationFromAddress();

        loadAndUpdate = true;
    }
    else if ( changedField == &m_xValuesSummaryAddressUiField )
    {
        m_xValuesSummaryAddress->setAddress( m_xValuesSummaryAddressUiField() );

        this->calculateCurveInterpolationFromAddress();

        loadAndUpdate = true;
    }
    else if ( &m_showCurve == changedField )
    {
        plot->updateAxes();
        plot->updatePlotTitle();
        plot->updateConnectedEditors();

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateSummaryPlotToolBar();

        if ( m_showCurve() == true )
        {
            plot->summaryCurveCollection()->setCurveAsTopZWithinCategory( this );
        }
    }
    else if ( changedField == &m_plotAxis )
    {
        updateQwtPlotAxis();

        plot->updateAxes();
    }
    else if ( changedField == &m_yValuesSummaryCase )
    {
        PdmObjectHandle* oldVal = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
        if ( oldVal == nullptr && m_yValuesSummaryCase->isObservedData() )
        {
            // If no previous case selected and observed data, use symbols to indicate observed data curve
            setLineStyle( RiuQwtPlotCurve::STYLE_NONE );
            setSymbol( RiuQwtSymbol::SYMBOL_XCROSS );
        }
        plot->updateCaseNameHasChanged();
        this->onLoadDataAndUpdate( true );
    }
    else if ( changedField == &m_yPushButtonSelectSummaryAddress )
    {
        RiuSummaryCurveDefSelectionDialog dlg( nullptr );
        RimSummaryCase*                   candidateCase    = m_yValuesSummaryCase();
        RifEclipseSummaryAddress          candicateAddress = m_yValuesSummaryAddress->address();

        if ( candidateCase == nullptr )
        {
            candidateCase = m_xValuesSummaryCase();
        }

        if ( !candicateAddress.isValid() )
        {
            candicateAddress = m_xValuesSummaryAddress->address();
        }

        dlg.hideEnsembles();
        dlg.setCaseAndAddress( candidateCase, candicateAddress );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( curveSelection.size() > 0 )
            {
                m_yValuesSummaryCase = curveSelection[0].summaryCase();
                m_yValuesSummaryAddress->setAddress( curveSelection[0].summaryAddress() );

                crossPlotTestForMatchingTimeSteps = true;
                loadAndUpdate                     = true;
            }
        }

        m_yPushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_xPushButtonSelectSummaryAddress )
    {
        RiuSummaryCurveDefSelectionDialog dlg( nullptr );
        RimSummaryCase*                   candidateCase    = m_xValuesSummaryCase();
        RifEclipseSummaryAddress          candicateAddress = m_xValuesSummaryAddress->address();

        if ( candidateCase == nullptr )
        {
            candidateCase = m_yValuesSummaryCase();
        }

        if ( !candicateAddress.isValid() )
        {
            candicateAddress = m_yValuesSummaryAddress->address();
        }

        dlg.hideEnsembles();
        dlg.setCaseAndAddress( candidateCase, candicateAddress );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( curveSelection.size() > 0 )
            {
                m_xValuesSummaryCase = curveSelection[0].summaryCase();
                m_xValuesSummaryAddress->setAddress( curveSelection[0].summaryAddress() );

                crossPlotTestForMatchingTimeSteps = true;
                loadAndUpdate                     = true;
            }
        }

        m_xPushButtonSelectSummaryAddress = false;
    }

    if ( crossPlotTestForMatchingTimeSteps )
    {
        auto curveValuesX    = this->valuesX();
        auto curveTimeStepsX = timeStepsX();

        auto curveValuesY    = this->valuesY();
        auto curveTimeStepsY = timeStepsY();

        if ( !curveValuesX.empty() && !curveValuesY.empty() )
        {
            RiaTimeHistoryCurveMerger curveMerger;
            curveMerger.addCurveData( curveTimeStepsX, curveValuesX );
            curveMerger.addCurveData( curveTimeStepsY, curveValuesY );
            curveMerger.computeInterpolatedValues();

            if ( curveMerger.validIntervalsForAllXValues().size() == 0 )
            {
                QString description;

                {
                    QDateTime first = QDateTime::fromTime_t( curveTimeStepsX.front() );
                    QDateTime last  = QDateTime::fromTime_t( curveTimeStepsX.back() );

                    std::vector<QDateTime> timeSteps;
                    timeSteps.push_back( first );
                    timeSteps.push_back( last );

                    QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( timeSteps );

                    description += QString( "Time step range for X : '%1' - '%2'" )
                                       .arg( first.toString( formatString ) )
                                       .arg( last.toString( formatString ) );
                }

                {
                    QDateTime first = QDateTime::fromTime_t( curveTimeStepsY.front() );
                    QDateTime last  = QDateTime::fromTime_t( curveTimeStepsY.back() );

                    std::vector<QDateTime> timeSteps;
                    timeSteps.push_back( first );
                    timeSteps.push_back( last );

                    QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( timeSteps );

                    description += "\n";
                    description += QString( "Time step range for Y : '%1' - '%2'" )
                                       .arg( first.toString( formatString ) )
                                       .arg( last.toString( formatString ) );
                }

                QMessageBox::warning( nullptr, "Detected no overlapping time steps", description );
            }
        }
    }

    if ( loadAndUpdate )
    {
        this->loadDataAndUpdate( true );

        plot->updateAxes();
        plot->updatePlotTitle();
        plot->updateConnectedEditors();

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateSummaryPlotToolBar();
    }

    if ( &m_showCurve == changedField )
    {
        // If no plot collection is found, we assume that we are inside a curve creator
        // Update the summary curve collection to make sure the curve names are updated in curve creator UI

        RimSummaryPlotCollection* plotCollection = nullptr;
        this->firstAncestorOrThisOfType( plotCollection );
        if ( !plotCollection )
        {
            RimSummaryCurveCollection* curveColl = nullptr;
            this->firstAncestorOrThisOfType( curveColl );
            if ( curveColl )
            {
                curveColl->updateConnectedEditors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryCurve::valuesSummaryReaderX() const
{
    if ( !m_xValuesSummaryCase() ) return nullptr;

    return m_xValuesSummaryCase()->summaryReader();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryCurve::valuesSummaryReaderY() const
{
    if ( !m_yValuesSummaryCase() ) return nullptr;

    return m_yValuesSummaryCase()->summaryReader();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCurve::timeStepsX() const
{
    static std::vector<time_t> emptyVector;
    RifSummaryReaderInterface* reader = valuesSummaryReaderX();

    if ( !reader ) return emptyVector;

    RifEclipseSummaryAddress addr = m_xValuesSummaryAddress()->address();

    return reader->timeSteps( addr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::calculateCurveInterpolationFromAddress()
{
    if ( m_yValuesSummaryAddress() )
    {
        auto address = m_yValuesSummaryAddress()->address();
        if ( address.hasAccumulatedData() )
        {
            m_curveInterpolation = RiuQwtPlotCurve::INTERPOLATION_POINT_TO_POINT;
        }
        else
        {
            m_curveInterpolation = RiuQwtPlotCurve::INTERPOLATION_STEP_LEFT;
        }
    }
}

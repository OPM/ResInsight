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

#include "RiaColorTables.h"
#include "RiaCurveMerger.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "RiaPreferencesSummary.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"

#include "RimEclipseResultCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMultipleSummaryPlotNameHelper.h"
#include "RimPlotAxisProperties.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"
#include "RimTools.h"

#include "RiuPlotAxis.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotCurve.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimSummaryCurve, "SummaryCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject( "Summary Curve", ":/SummaryCurve16x16.png" );

    // Y Values
    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryCase, "SummaryCase", "Case" );
    m_yValuesSummaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddressUiField, "SelectedVariableDisplayVar", "Vector" );
    m_yValuesSummaryAddressUiField.xmlCapability()->disableIO();
    m_yValuesSummaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddress, "SummaryAddress", "Summary Address" );
    m_yValuesSummaryAddress.uiCapability()->setUiTreeHidden( true );
    m_yValuesSummaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_yPushButtonSelectSummaryAddress, "SelectAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_yPushButtonSelectSummaryAddress );
    m_yPushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_yPushButtonSelectSummaryAddress = false;

    m_yValuesSummaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_resampling, "Resampling", "Resampling" );

    // X Values
    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryCase, "SummaryCaseX", "Case" );
    m_xValuesSummaryCase.uiCapability()->setUiTreeChildrenHidden( true );
    m_xValuesSummaryCase.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryAddressUiField, "SelectedVariableDisplayVarX", "Vector" );
    m_xValuesSummaryAddressUiField.xmlCapability()->disableIO();
    m_xValuesSummaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_xValuesSummaryAddress, "SummaryAddressX", "Summary Address" );
    m_xValuesSummaryAddress.uiCapability()->setUiTreeHidden( true );
    m_xValuesSummaryAddress.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_xPushButtonSelectSummaryAddress, "SelectAddressX", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_xPushButtonSelectSummaryAddress );
    m_xPushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_xPushButtonSelectSummaryAddress = false;

    m_xValuesSummaryAddress = new RimSummaryAddress;

    // Other members
    CAF_PDM_InitFieldNoDefault( &m_isEnsembleCurve, "IsEnsembleCurve", "Ensemble Curve" );
    m_isEnsembleCurve.v() = caf::Tristate::State::PartiallyTrue;

    CAF_PDM_InitFieldNoDefault( &m_plotAxis_OBSOLETE, "PlotAxis", "Axis" );
    m_plotAxis_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_plotAxisProperties, "Axis", "Axis" );

    CAF_PDM_InitFieldNoDefault( &m_curveNameConfig, "SummaryCurveNameConfig", "SummaryCurveNameConfig" );
    m_curveNameConfig.uiCapability()->setUiTreeHidden( true );
    m_curveNameConfig.uiCapability()->setUiTreeChildrenHidden( true );

    m_curveNameConfig = new RimSummaryCurveAutoName;

    CAF_PDM_InitField( &m_isTopZWithinCategory, "isTopZWithinCategory", false, "" );
    m_isTopZWithinCategory.uiCapability()->setUiHidden( true );

    setSymbolSkipDistance( 10.0f );
    setLineThickness( 2 );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::~RimSummaryCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition RimSummaryCurve::curveDefinitionY() const
{
    return RiaSummaryCurveDefinition( summaryCaseY(), summaryAddressY(), isEnsembleCurve() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryCaseY( RimSummaryCase* sumCase )
{
    if ( m_yValuesSummaryCase != sumCase )
    {
        clearErrorBars();
    }

    bool isEnsembleCurve = false;
    if ( sumCase && sumCase->ensemble() ) isEnsembleCurve = true;

    setIsEnsembleCurve( isEnsembleCurve );

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
        clearErrorBars();
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

    if ( values.empty() ) return values;

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
void RimSummaryCurve::applyCurveDefinitionY( const RiaSummaryCurveDefinition& curveDefinition )
{
    setSummaryCaseY( curveDefinition.summaryCase() );
    setSummaryAddressY( curveDefinition.summaryAddress() );
    setIsEnsembleCurve( curveDefinition.isEnsembleCurve() );
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
double RimSummaryCurve::yValueAtTimeT( time_t time ) const
{
    const std::vector<time_t>& timeSteps = timeStepsY();
    const std::vector<double>  values    = valuesY();

    if ( timeSteps.empty() || time < timeSteps.front() || time > timeSteps.back() )
        return std::numeric_limits<double>::infinity();

    for ( size_t i = 0; i < timeSteps.size(); ++i )
    {
        if ( timeSteps[i] == time )
        {
            return values[i];
        }
        else if ( i < timeSteps.size() - 1u && timeSteps[i] < time && time < timeSteps[i + 1] )
        {
            if ( m_curveAppearance->interpolation() ==
                 RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT )
            {
                return values[i + 1];
            }
            else
            {
                double slope = 0.0;
                if ( timeSteps[i + 1] != timeSteps[i] )
                {
                    slope = ( values[i + 1] - values[i] ) / (double)( timeSteps[i + 1] - timeSteps[i] );
                }
                return slope * ( time - timeSteps[i] ) + values[i];
            }
        }
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setOverrideCurveDataY( const std::vector<time_t>& dateTimes, const std::vector<double>& yValues )
{
    setSamplesFromTimeTAndYValues( dateTimes, yValues, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition RimSummaryCurve::curveDefinitionX() const
{
    return RiaSummaryCurveDefinition( summaryCaseX(), summaryAddressX(), isEnsembleCurve() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setSummaryCaseX( RimSummaryCase* sumCase )
{
    m_xValuesSummaryCase = sumCase;

    bool isEnsembleCurve = false;
    if ( sumCase && sumCase->ensemble() ) isEnsembleCurve = true;

    setIsEnsembleCurve( isEnsembleCurve );
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
void RimSummaryCurve::setLeftOrRightAxisY( RiuPlotAxis plotAxis )
{
    m_plotAxis_OBSOLETE = plotAxis.axis();

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );
    m_plotAxisProperties = plot->axisPropertiesForPlotAxis( plotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimSummaryCurve::axisY() const
{
    if ( m_plotAxisProperties )
        return m_plotAxisProperties->plotAxisType();
    else
        return RiuPlotAxis::defaultLeft();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurve::isEnsembleCurve() const
{
    return m_isEnsembleCurve().isTrue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setIsEnsembleCurve( bool isEnsembleCurve )
{
    m_isEnsembleCurve.v() = isEnsembleCurve ? caf::Tristate::State::True : caf::Tristate::State::False;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options =
        this->RimPlotCurve::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    if ( !options.isEmpty() ) return options;

    if ( fieldNeedingOptions == &m_yValuesSummaryCase || fieldNeedingOptions == &m_xValuesSummaryCase )
    {
        RimProject* proj = RimProject::current();

        std::vector<RimSummaryCase*> cases = proj->allSummaryCases();

        cases.push_back( proj->calculationCollection->calculationSummaryCase() );

        for ( RimSummaryCase* rimCase : cases )
        {
            options.push_back( caf::PdmOptionItemInfo( rimCase->displayCaseName(), rimCase ) );
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
    else if ( fieldNeedingOptions == &m_plotAxisProperties )
    {
        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted( plot );

        for ( auto axis : plot->plotAxes() )
        {
            if ( dynamic_cast<RimPlotAxisProperties*>( axis ) )
            {
                options.push_back( caf::PdmOptionItemInfo( axis->name(), axis ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurve::createCurveAutoName()
{
    std::vector<const RimSummaryNameHelper*> nameHelpers;
    {
        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfType( plot );
        if ( plot )
        {
            auto nameHelper = plot->plotTitleHelper();
            if ( nameHelper ) nameHelpers.push_back( nameHelper );
        }
    }
    {
        RimSummaryMultiPlot* summaryMultiPlot = nullptr;
        firstAncestorOrThisOfType( summaryMultiPlot );
        if ( summaryMultiPlot )
        {
            auto nameHelper = summaryMultiPlot->nameHelper();
            if ( nameHelper ) nameHelpers.push_back( nameHelper );
        }
    }

    RimMultiSummaryPlotNameHelper multiNameHelper( nameHelpers );
    QString curveName = m_curveNameConfig->curveNameY( m_yValuesSummaryAddress->address(), &multiNameHelper );
    if ( curveName.isEmpty() )
    {
        curveName = m_curveNameConfig->curveNameY( m_yValuesSummaryAddress->address(), nullptr );
    }

    if ( isCrossPlotCurve() )
    {
        QString curveNameX = m_curveNameConfig->curveNameX( m_xValuesSummaryAddress->address(), &multiNameHelper );
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

    plot->updateZoomInParentPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    m_yValuesSummaryAddressUiField = m_yValuesSummaryAddress->address();
    m_xValuesSummaryAddressUiField = m_xValuesSummaryAddress->address();

    updateConnectedEditors();

    setZIndexFromCurveInfo();

    if ( isCurveVisible() )
    {
        std::vector<double> curveValuesY = this->valuesY();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfTypeAsserted( plot );
        bool useLogarithmicScale = plot->isLogarithmicScaleEnabled( this->axisY() );

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
                    this->setSamplesFromXYValues( curveMerger.interpolatedYValuesForAllXValues( 0 ),
                                                  curveMerger.interpolatedYValuesForAllXValues( 1 ),
                                                  useLogarithmicScale );
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

                            auto timeSteps = RiuQwtPlotCurve::fromTime_t( curveTimeStepsY );

                            if ( !errValues.empty() )
                            {
                                this->setSamplesFromXYErrorValues( timeSteps, curveValuesY, errValues, useLogarithmicScale );
                            }
                            else
                            {
                                this->setSamplesFromXYValues( timeSteps, curveValuesY, useLogarithmicScale );
                            }
                        }
                        else
                        {
                            if ( m_resampling() != RiaDefines::DateTimePeriod::NONE )
                            {
                                auto [resampledTimeSteps, resampledValues] =
                                    RiaSummaryTools::resampledValuesForPeriod( m_yValuesSummaryAddress->address(),
                                                                               curveTimeStepsY,
                                                                               curveValuesY,
                                                                               m_resampling() );

                                if ( !resampledValues.empty() && !resampledTimeSteps.empty() )
                                {
                                    // When values are resampled, each time step value is reported at the end of each
                                    // resampling period. Insert a duplicate of the first value at the start of the time
                                    // series to make curve start at the very first reported time step.

                                    resampledTimeSteps.insert( resampledTimeSteps.begin(), curveTimeStepsY.front() );
                                    resampledValues.insert( resampledValues.begin(), resampledValues.front() );

                                    this->setSamplesFromTimeTAndYValues( resampledTimeSteps,
                                                                         resampledValues,
                                                                         useLogarithmicScale );
                                }
                            }
                            else
                            {
                                this->setSamplesFromTimeTAndYValues( curveTimeStepsY, curveValuesY, useLogarithmicScale );
                            }
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

                    this->setSamplesFromXYValues( timeFromSimulationStart, curveValuesY, useLogarithmicScale );
                }
            }
            else
            {
                shouldPopulateViewWithEmptyData = true;
            }
        }

        if ( shouldPopulateViewWithEmptyData )
        {
            this->setSamplesFromXYValues( std::vector<double>(), std::vector<double>(), useLogarithmicScale );
        }

        if ( updateParentPlot && hasParentPlot() )
        {
            updateZoomInParentPlot();
            replotParentPlot();
        }
    }

    if ( updateParentPlot ) updateAxisInPlot( axisY() );
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

    caf::IconProvider iconProvider = this->uiIconProvider();
    if ( !iconProvider.valid() ) return;

    RimSummaryCurveCollection* coll = nullptr;
    this->firstAncestorOrThisOfType( coll );
    if ( coll && coll->curveForSourceStepping() == this )
    {
        iconProvider.setOverlayResourceString( ":/StepUpDownCorner16x16.png" );
    }
    else
    {
        iconProvider.setOverlayResourceString( "" );
    }

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::initAfterRead()
{
    RimStackablePlotCurve::initAfterRead();

    if ( m_plotAxisProperties.value() == nullptr )
    {
        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfType( plot );
        if ( plot ) m_plotAxisProperties = plot->axisPropertiesForPlotAxis( RiuPlotAxis( m_plotAxis_OBSOLETE() ) );
    }

    if ( m_isEnsembleCurve().isPartiallyTrue() )
    {
        m_isEnsembleCurve.v() = ( summaryCaseY() && summaryCaseY()->ensemble() ) ? caf::Tristate::State::True
                                                                                 : caf::Tristate::State::False;
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
        curveDataGroup->add( &m_yValuesSummaryCase, { true, 3, 1 } );
        curveDataGroup->add( &m_yValuesSummaryAddressUiField, { true, 2, 1 } );
        curveDataGroup->add( &m_yPushButtonSelectSummaryAddress, { false, 1, 0 } );
        curveDataGroup->add( &m_resampling, { true, 3, 1 } );
        curveDataGroup->add( &m_plotAxisProperties, { true, 3, 1 } );

        if ( isCrossPlotCurve() )
            m_showErrorBars = false;
        else
            curveDataGroup->add( &m_showErrorBars );
    }

    if ( isCrossPlotCurve() )
    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector X" );
        curveDataGroup->add( &m_xValuesSummaryCase, { true, 3, 1 } );
        curveDataGroup->add( &m_xValuesSummaryAddressUiField, { true, 2, 1 } );
        curveDataGroup->add( &m_xPushButtonSelectSummaryAddress, { false, 1, 0 } );
    }

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

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

        options->push_front( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(),
                                                     QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setResampling( RiaDefines::DateTimePeriodEnum resampling )
{
    m_resampling = resampling;
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
            zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_OBSERVED );
        }
        else if ( sumAddr.category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
        {
            zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_STAT_CURVE );
        }
        else if ( sumCase->ensemble() )
        {
            zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ENSEMBLE_CURVE );
        }
        else
        {
            zOrder = RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_SINGLE_CURVE_NON_OBSERVED );
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
RiaDefines::PhaseType RimSummaryCurve::phaseType() const
{
    return m_yValuesSummaryAddress->addressPhaseType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::updatePlotAxis()
{
    updateAxisInPlot( axisY() );
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

    auto curveSet = coll ? coll->findCurveSetFromPlotCurve( m_plotCurve ) : nullptr;
    auto group    = curveSet ? curveSet->summaryCaseCollection() : nullptr;

    auto addressUiText = addr.uiText();

    if ( group && group->isEnsemble() )
    {
        return QString( "%1.%2.%3" )
            .arg( QString::fromStdString( addressUiText ) )
            .arg( m_yValuesSummaryCase->nativeCaseName() )
            .arg( group->name() );
    }
    else
    {
        return QString( "%1.%2" ).arg( QString::fromStdString( addressUiText ) ).arg( m_yValuesSummaryCase->nativeCaseName() );
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
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
            setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );

            return;
        }
    }

    if ( m_yValuesSummaryAddress && m_yValuesSummaryAddress->address().isHistoryQuantity() )
    {
        RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

        if ( prefs->defaultSummaryHistoryCurveStyle() == RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS )
        {
            setSymbolEdgeColor( m_curveAppearance->color() );

            setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
        }
        else if ( prefs->defaultSummaryHistoryCurveStyle() ==
                  RiaPreferencesSummary::SummaryHistoryCurveStyleMode::SYMBOLS_AND_LINES )
        {
            setSymbolEdgeColor( m_curveAppearance->color() );

            setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
        }
        else if ( prefs->defaultSummaryHistoryCurveStyle() == RiaPreferencesSummary::SummaryHistoryCurveStyleMode::LINES )
        {
            setSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
        }

        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::setDefaultCurveAppearance()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    if ( plot ) plot->applyDefaultCurveAppearances( { this } );
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
    RimStackablePlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

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
    else if ( changedField == &m_resampling )
    {
        loadAndUpdate = true;
    }
    else if ( &m_showCurve == changedField )
    {
        plot->updateAxes();
        plot->updatePlotTitle();

        RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();

        // If no plot collection is found, we assume that we are inside a curve creator
        // Update the summary curve collection to make sure the curve names are updated in curve creator UI
        visibilityChanged.send( m_showCurve() );
    }
    else if ( changedField == &m_plotAxisProperties )
    {
        updateAxisInPlot( axisY() );
        plot->updateAxes();
        dataChanged.send();
    }
    else if ( changedField == &m_yValuesSummaryCase )
    {
        PdmObjectHandle* oldVal = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
        if ( oldVal == nullptr && m_yValuesSummaryCase->isObservedData() )
        {
            // If no previous case selected and observed data, use symbols to indicate observed data curve
            setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
            setSymbol( RiuPlotCurveSymbol::SYMBOL_XCROSS );
        }
        plot->updateCaseNameHasChanged();

        // TODO: is it not ok to just set loadAndUpdate = true?
        this->onLoadDataAndUpdate( true );
        dataChanged.send();
    }
    else if ( changedField == &m_yPushButtonSelectSummaryAddress )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimSummaryCase*                 candidateCase    = m_yValuesSummaryCase();
        RifEclipseSummaryAddress        candicateAddress = m_yValuesSummaryAddress->address();

        if ( candidateCase == nullptr )
        {
            candidateCase = m_xValuesSummaryCase();
        }

        if ( !candicateAddress.isValid() )
        {
            candicateAddress = m_xValuesSummaryAddress->address();
        }

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
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimSummaryCase*                 candidateCase    = m_xValuesSummaryCase();
        RifEclipseSummaryAddress        candicateAddress = m_xValuesSummaryAddress->address();

        if ( candidateCase == nullptr )
        {
            candidateCase = m_yValuesSummaryCase();
        }

        if ( !candicateAddress.isValid() )
        {
            candicateAddress = m_yValuesSummaryAddress->address();
        }

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
                    QDateTime first = QDateTime::fromSecsSinceEpoch( curveTimeStepsX.front() );
                    QDateTime last  = QDateTime::fromSecsSinceEpoch( curveTimeStepsX.back() );

                    std::vector<QDateTime> timeSteps;
                    timeSteps.push_back( first );
                    timeSteps.push_back( last );

                    QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( timeSteps );

                    description += QString( "Time step range for X : '%1' - '%2'" )
                                       .arg( first.toString( formatString ) )
                                       .arg( last.toString( formatString ) );
                }

                {
                    QDateTime first = QDateTime::fromSecsSinceEpoch( curveTimeStepsY.front() );
                    QDateTime last  = QDateTime::fromSecsSinceEpoch( curveTimeStepsY.back() );

                    std::vector<QDateTime> timeSteps;
                    timeSteps.push_back( first );
                    timeSteps.push_back( last );

                    QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( timeSteps );

                    description += "\n";
                    description += QString( "Time step range for Y : '%1' - '%2'" )
                                       .arg( first.toString( formatString ) )
                                       .arg( last.toString( formatString ) );
                }

                RiaLogging::errorInMessageBox( nullptr, "Detected no overlapping time steps", description );
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
        mainPlotWindow->updateMultiPlotToolBar();

        dataChanged.send();
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
        if ( RiaSummaryTools::hasAccumulatedData( address ) )
        {
            m_curveAppearance->setInterpolation(
                RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT );
        }
        else
        {
            m_curveAppearance->setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT );
        }
    }
}

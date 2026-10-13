/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RimSummaryDeclineCurve.h"

#include "RiaColorTools.h"
#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RigDeclineCurveCalculator.h"

#include "RiaPreferences.h"

#include "RimSummaryPlot.h"
#include "RimTimeAxisAnnotation.h"

#include "RiuDraggableOverlayFrame.h"
#include "RiuPlotWidget.h"
#include "RiuTextContentFrame.h"

#include "cafPdmUiDateEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTextEditor.h"

#include <QDateTime>

#include <algorithm>
#include <cmath>
#include <limits>

CAF_PDM_SOURCE_INIT( RimSummaryDeclineCurve, "DeclineCurve" );

namespace caf
{
template <>
void caf::AppEnum<RimSummaryDeclineCurve::DeclineCurveType>::setUp()
{
    addItem( RimSummaryDeclineCurve::DeclineCurveType::EXPONENTIAL, "EXPONENTIAL", "Exponential" );
    addItem( RimSummaryDeclineCurve::DeclineCurveType::HARMONIC, "HARMONIC", "Harmonic" );
    addItem( RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC, "HYPERBOLIC", "Hyperbolic" );
    setDefault( RimSummaryDeclineCurve::DeclineCurveType::HARMONIC );
}

template <>
void caf::AppEnum<RimSummaryDeclineCurve::FitMethod>::setUp()
{
    addItem( RimSummaryDeclineCurve::FitMethod::START_END_POINT, "START_END_POINT", "Start and End Point" );
    addItem( RimSummaryDeclineCurve::FitMethod::REGRESSION, "REGRESSION", "Regression" );
    setDefault( RimSummaryDeclineCurve::FitMethod::START_END_POINT );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve::RimSummaryDeclineCurve()
{
    CAF_PDM_InitObject( "Decline Curve", ":/decline-curve.svg" );

    CAF_PDM_InitFieldNoDefault( &m_declineCurveType, "DeclineCurveType", "Type" );

    CAF_PDM_InitFieldNoDefault( &m_fitMethod, "FitMethod", "Fit Method" );

    CAF_PDM_InitField( &m_predictionYears, "PredictionYears", 5, "Years" );
    m_predictionYears.setRange( 1, 50 );
    m_predictionYears.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_hyperbolicDeclineConstant, "HyperbolicDeclineConstant", 0.5, "Decline Constant" );
    m_hyperbolicDeclineConstant.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_minTimeSliderPosition, "MinTimeSliderPosition", 75, "From" );
    m_minTimeSliderPosition.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxTimeSliderPosition, "MaxTimeSliderPosition", 100, "To" );
    m_maxTimeSliderPosition.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    const QString dateFormat = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat() );

    CAF_PDM_InitFieldNoDefault( &m_minDate, "MinDate", "From", "", dateFormat );
    m_minDate.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxDate, "MaxDate", "To", "", dateFormat );
    m_maxDate.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showTimeSelectionInPlot, "ShowTimeSelectionInPlot", true, "Show In Plot" );

    CAF_PDM_InitFieldNoDefault( &m_equationText, "EquationText", "Equation" );
    m_equationText.registerGetMethod( this, &RimSummaryDeclineCurve::equationTextProxy );
    m_equationText.uiCapability()->setUiReadOnly( true );
    m_equationText.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showEquationInPlot, "ShowEquationInPlot", true, "Show Equation In Plot" );

    // Multi-selection editing only notifies the last object by default, all selected curves must be recalculated
    for ( caf::PdmFieldHandle* field : std::initializer_list<caf::PdmFieldHandle*>{ &m_declineCurveType,
                                                                                    &m_fitMethod,
                                                                                    &m_predictionYears,
                                                                                    &m_hyperbolicDeclineConstant,
                                                                                    &m_minTimeSliderPosition,
                                                                                    &m_maxTimeSliderPosition,
                                                                                    &m_minDate,
                                                                                    &m_maxDate,
                                                                                    &m_showTimeSelectionInPlot,
                                                                                    &m_showEquationInPlot } )
    {
        field->uiCapability()->setNotifyAllFieldsInMultiFieldChangedEvents( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve::~RimSummaryDeclineCurve()
{
    if ( auto plot = firstAncestorOrThisOfType<RimSummaryPlot>() )
    {
        plot->scheduleRebuildEquationOverlays();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::valuesY() const
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

    return createDeclineCurveValues( RimSummaryCurve::valuesY(), RimSummaryCurve::timeStepsY(), minTimeStep, maxTimeStep, curveType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::valuesX() const
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

    return createDeclineCurveValues( RimSummaryCurve::valuesX(), RimSummaryCurve::timeStepsX(), minTimeStep, maxTimeStep, curveType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTimeAxisAnnotation*> RimSummaryDeclineCurve::createTimeAnnotations() const
{
    if ( m_showTimeSelectionInPlot && isChecked() )
    {
        auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

        auto annotation = RimTimeAxisAnnotation::createTimeRangeAnnotation( minTimeStep, maxTimeStep, color() );
        annotation->setName( "" );

        return { annotation };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::timeStepsY() const
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

    std::vector<time_t> timeSteps = getTimeStepsInRange( RimSummaryCurve::timeStepsY(), minTimeStep, maxTimeStep );
    appendFutureTimeSteps( timeSteps );
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::timeStepsX() const
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

    std::vector<time_t> timeSteps = getTimeStepsInRange( RimSummaryCurve::timeStepsX(), minTimeStep, maxTimeStep );
    appendFutureTimeSteps( timeSteps );
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::createDeclineCurveValues( const std::vector<double>&                 values,
                                                                      const std::vector<time_t>&                 timeSteps,
                                                                      time_t                                     minTimeStep,
                                                                      time_t                                     maxTimeStep,
                                                                      RifEclipseSummaryAddressDefines::CurveType curveType ) const
{
    if ( values.empty() || timeSteps.empty() ) return values;

    // Use only the values inside the range specified
    auto [timeStepsInRange, valuesInRange] = getInRangeValues( timeSteps, values, minTimeStep, maxTimeStep );

    if ( timeStepsInRange.empty() || valuesInRange.empty() ) return {};

    std::vector<time_t> futureTimeSteps;
    for ( const QDateTime& futureTime : createFutureTimeSteps( timeStepsInRange ) )
    {
        futureTimeSteps.push_back( RiaTimeTTools::fromQDateTime( futureTime ) );
    }

    auto outValues =
        RigDeclineCurveCalculator::computeDeclineCurveValues( declineModel( curveType ), valuesInRange, timeStepsInRange, futureTimeSteps );
    if ( outValues ) return *outValues;

    // The fit failed. Must match the time steps from timeStepsY(), so leave the prediction undefined.
    valuesInRange.resize( valuesInRange.size() + futureTimeSteps.size(), std::numeric_limits<double>::quiet_NaN() );
    return valuesInRange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryDeclineCurve::equationText() const
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

    auto [timeStepsInRange, valuesInRange] =
        getInRangeValues( RimSummaryCurve::timeStepsY(), RimSummaryCurve::valuesY(), minTimeStep, maxTimeStep );

    const auto model      = declineModel( curveType() );
    auto       parameters = RigDeclineCurveCalculator::computeDeclineParameters( model, valuesInRange, timeStepsInRange );
    if ( !parameters ) return "Equation not available";

    return RigDeclineCurveCalculator::equationText( model, *parameters );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigDeclineCurveCalculator::DeclineModel RimSummaryDeclineCurve::declineModel( RifEclipseSummaryAddressDefines::CurveType curveType ) const
{
    return { m_declineCurveType(),
             m_fitMethod(),
             m_hyperbolicDeclineConstant(),
             curveType == RifEclipseSummaryAddressDefines::CurveType::ACCUMULATED };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryDeclineCurve::equationTextProxy() const
{
    return equationText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimSummaryDeclineCurve::fullTimeStepRange() const
{
    auto timeSteps = RimSummaryCurve::timeStepsY();
    if ( !timeSteps.empty() )
    {
        return std::make_pair( *timeSteps.begin(), *timeSteps.rbegin() );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimSummaryDeclineCurve::selectedTimeStepRange() const
{
    // Scale the slider values to the full time step range

    auto [min, max] = fullTimeStepRange();

    if ( m_minDate().isValid() && m_maxDate().isValid() )
    {
        const time_t dateMin = RiaTimeTTools::fromQDateTime( RiaQDateTimeTools::createDateTime( m_minDate() ) );
        const time_t dateMax = RiaTimeTTools::fromQDateTime( RiaQDateTimeTools::createDateTime( m_maxDate() ) ) + 24 * 3600 - 1;
        return { dateMin, dateMax };
    }

    auto range       = max - min;
    auto selectedMin = min + static_cast<time_t>( range * ( m_minTimeSliderPosition / 100.0 ) );
    auto selectedMax = min + static_cast<time_t>( range * ( m_maxTimeSliderPosition / 100.0 ) );

    return { selectedMin, selectedMax };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimSummaryDeclineCurve::createFutureTimeSteps( const std::vector<time_t>& timeSteps ) const
{
    if ( timeSteps.empty() ) return {};

    // Create additional time steps
    QDateTime lastTimeStep  = RiaQDateTimeTools::fromTime_t( timeSteps.back() );
    QDateTime predictionEnd = RiaQDateTimeTools::addYears( lastTimeStep, m_predictionYears() );

    int numDates = 50;
    return RiaQDateTimeTools::createEvenlyDistributedDatesInInterval( lastTimeStep, predictionEnd, numDates );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::appendFutureTimeSteps( std::vector<time_t>& timeSteps ) const
{
    std::set<QDateTime> futureTimeSteps = createFutureTimeSteps( timeSteps );
    appendTimeSteps( timeSteps, futureTimeSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::appendTimeSteps( std::vector<time_t>& timeSteps, const std::set<QDateTime>& moreTimeSteps )
{
    for ( const QDateTime& t : moreTimeSteps )
        timeSteps.push_back( RiaTimeTTools::fromQDateTime( t ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    // A decline curve is only supported for time history curves, hide the X-axis group.
    hideXAxisGroup();

    caf::PdmUiGroup* declineCurveGroup = uiOrdering.addNewGroup( "Decline Curve" );
    declineCurveGroup->add( &m_declineCurveType );
    declineCurveGroup->add( &m_fitMethod );
    declineCurveGroup->add( &m_predictionYears );

    if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC )
    {
        declineCurveGroup->add( &m_hyperbolicDeclineConstant );
    }

    declineCurveGroup->add( &m_equationText );
    declineCurveGroup->add( &m_showEquationInPlot );

    if ( !m_minDate().isValid() || !m_maxDate().isValid() ) syncDatesFromSliders();

    caf::PdmUiGroup* timeSelectionGroup = uiOrdering.addNewGroup( "Time Selection" );
    timeSelectionGroup->add( &m_minDate );
    timeSelectionGroup->add( &m_maxDate );
    timeSelectionGroup->add( &m_minTimeSliderPosition );
    timeSelectionGroup->add( &m_maxTimeSliderPosition );
    timeSelectionGroup->add( &m_showTimeSelectionInPlot );

    RimSummaryCurve::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_minTimeSliderPosition == changedField && m_minTimeSliderPosition > m_maxTimeSliderPosition )
    {
        m_maxTimeSliderPosition = m_minTimeSliderPosition;
    }

    if ( &m_maxTimeSliderPosition == changedField && m_maxTimeSliderPosition < m_minTimeSliderPosition )
    {
        m_minTimeSliderPosition = m_maxTimeSliderPosition;
    }

    if ( changedField == &m_minTimeSliderPosition || changedField == &m_maxTimeSliderPosition )
    {
        syncDatesFromSliders();
    }

    if ( changedField == &m_minDate || changedField == &m_maxDate )
    {
        if ( m_minDate() > m_maxDate() )
        {
            if ( changedField == &m_minDate )
                m_maxDate = m_minDate();
            else
                m_minDate = m_maxDate();
        }
        syncSlidersFromDates();
    }

    RimSummaryCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == objectToggleField() ) updateEquationOverlay();
    if ( changedField == &m_declineCurveType || changedField == &m_fitMethod || changedField == &m_predictionYears ||
         changedField == &m_hyperbolicDeclineConstant || changedField == &m_minTimeSliderPosition ||
         changedField == &m_maxTimeSliderPosition || changedField == &m_minDate || changedField == &m_maxDate ||
         changedField == &m_showTimeSelectionInPlot || changedField == &m_showEquationInPlot )
    {
        loadAndUpdateDataAndPlot();
        auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
        if ( plot ) plot->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimSummaryCurve::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_hyperbolicDeclineConstant )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            // Hyperbolic decline constant must be larger than 0 to avoid calculation issues.
            myAttr->m_minimum  = 0.001;
            myAttr->m_maximum  = 1.0;
            myAttr->m_decimals = 2;
        }
    }
    else if ( field == &m_predictionYears )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum = 1;
            myAttr->m_maximum = 50;
        }
    }
    else if ( field == &m_equationText )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute ) )
        {
            myAttr->heightHint = -1;
        }
    }
    else if ( field == &m_minDate || field == &m_maxDate )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute ) )
        {
            myAttr->dateFormat = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat() );
        }
    }
    else if ( field == &m_minTimeSliderPosition || field == &m_maxTimeSliderPosition )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum     = 0;
            myAttr->m_maximum     = 100;
            myAttr->m_showSpinBox = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::syncDatesFromSliders()
{
    // No data loaded yet, keep the dates invalid so they are synced when data is available
    if ( RimSummaryCurve::timeStepsY().empty() ) return;

    auto [min, max]  = fullTimeStepRange();
    auto range       = max - min;
    auto selectedMin = min + static_cast<time_t>( range * ( m_minTimeSliderPosition / 100.0 ) );
    auto selectedMax = min + static_cast<time_t>( range * ( m_maxTimeSliderPosition / 100.0 ) );

    m_minDate = RiaQDateTimeTools::fromTime_t( selectedMin ).date();
    m_maxDate = RiaQDateTimeTools::fromTime_t( selectedMax ).date();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::syncSlidersFromDates()
{
    auto [min, max] = fullTimeStepRange();
    if ( max <= min ) return;

    auto toSliderPosition = [min = min, max = max]( const QDate& date )
    {
        const double fraction =
            static_cast<double>( RiaTimeTTools::fromQDateTime( RiaQDateTimeTools::createDateTime( date ) ) - min ) / ( max - min );
        return std::clamp( static_cast<int>( std::lround( fraction * 100.0 ) ), 0, 100 );
    };

    m_minTimeSliderPosition = toSliderPosition( m_minDate() );
    m_maxTimeSliderPosition = toSliderPosition( m_maxDate() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::setFitMethod( FitMethod fitMethod )
{
    m_fitMethod = fitMethod;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::setDeclineCurveType( DeclineCurveType declineCurveType )
{
    m_declineCurveType = declineCurveType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryDeclineCurve::createCurveAutoName()
{
    return RimSummaryCurve::createCurveAutoName() + " " + m_declineCurveType().uiText() + " Decline";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryDeclineCurve::curveExportDescription( const RifEclipseSummaryAddress& address ) const
{
    return RimSummaryCurve::curveExportDescription( {} ) + "." + m_declineCurveType().uiText() + "_Decline";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::updateTimeAnnotations()
{
    if ( auto plot = firstAncestorOrThisOfType<RimSummaryPlot>() )
    {
        plot->updateAndRedrawTimeAnnotations();
    }

    updateEquationOverlay();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::updateEquationOverlay()
{
    if ( auto plot = firstAncestorOrThisOfType<RimSummaryPlot>() )
    {
        plot->scheduleRebuildEquationOverlays();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDraggableOverlayFrame* RimSummaryDeclineCurve::createEquationOverlayFrame( QWidget* parent, int overlayMargins )
{
    if ( !m_showEquationInPlot() || !isChecked() ) return nullptr;

    auto frame        = new RiuDraggableOverlayFrame( parent, overlayMargins );
    auto contentFrame = new RiuTextContentFrame( nullptr, createCurveAutoName(), equationText(), -1 );
    contentFrame->setTitleColor( RiaColorTools::toQColor( color() ) );
    frame->setContentFrame( contentFrame );
    frame->setMaximumWidth( 10000 );
    return frame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>> RimSummaryDeclineCurve::getInRangeValues( const std::vector<time_t>& timeSteps,
                                                                                              const std::vector<double>& values,
                                                                                              time_t                     minTimeStep,
                                                                                              time_t                     maxTimeStep )
{
    // TODO: duplicated with RimSummarRegressionAnalysisCurve

    CAF_ASSERT( timeSteps.size() == values.size() );

    std::vector<time_t> filteredTimeSteps;
    std::vector<double> filteredValues;
    for ( size_t i = 0; i < timeSteps.size(); i++ )
    {
        time_t timeStep = timeSteps[i];
        if ( timeStep >= minTimeStep && timeStep <= maxTimeStep )
        {
            filteredTimeSteps.push_back( timeStep );
            filteredValues.push_back( values[i] );
        }
    }

    return std::make_pair( filteredTimeSteps, filteredValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::getTimeStepsInRange( const std::vector<time_t>& timeSteps, time_t minTimeStep, time_t maxTimeStep )
{
    std::vector<time_t> filteredTimeSteps;
    for ( size_t i = 0; i < timeSteps.size(); i++ )
    {
        time_t timeStep = timeSteps[i];
        if ( timeStep >= minTimeStep && timeStep <= maxTimeStep )
        {
            filteredTimeSteps.push_back( timeStep );
        }
    }

    return filteredTimeSteps;
}

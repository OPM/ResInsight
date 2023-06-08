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

#include "RiaQDateTimeTools.h"
#include "RiaSummaryTools.h"
#include "RiaTimeTTools.h"

#include "RigDeclineCurveCalculator.h"

#include "RimSummaryPlot.h"
#include "RimTimeAxisAnnotation.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiSliderEditor.h"

#include <QDateTime>

#include <cmath>

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
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve::RimSummaryDeclineCurve()
{
    CAF_PDM_InitObject( "Decline Curve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_declineCurveType, "DeclineCurveType", "Type" );
    CAF_PDM_InitField( &m_predictionYears, "PredictionYears", 5, "Years" );
    CAF_PDM_InitField( &m_hyperbolicDeclineConstant, "HyperbolicDeclineConstant", 0.5, "Decline Constant" );
    m_hyperbolicDeclineConstant.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_minTimeStep, "MinTimeStep", "From" );
    m_minTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxTimeStep, "MaxTimeStep", "To" );
    m_maxTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showTimeSelectionInPlot, "ShowTimeSelectionInPlot", true, "Show In Plot" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve::~RimSummaryDeclineCurve()
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot && m_timeRangeAnnotation ) plot->removeTimeAnnotation( m_timeRangeAnnotation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::valuesY() const
{
    return createDeclineCurveValues( RimSummaryCurve::valuesY(),
                                     RimSummaryCurve::timeStepsY(),
                                     m_minTimeStep,
                                     m_maxTimeStep,
                                     RiaSummaryTools::hasAccumulatedData( summaryAddressY() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::valuesX() const
{
    return createDeclineCurveValues( RimSummaryCurve::valuesX(),
                                     RimSummaryCurve::timeStepsX(),
                                     m_minTimeStep,
                                     m_maxTimeStep,
                                     RiaSummaryTools::hasAccumulatedData( summaryAddressX() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::timeStepsY() const
{
    std::vector<time_t> timeSteps = getTimeStepsInRange( RimSummaryCurve::timeStepsY(), m_minTimeStep, m_maxTimeStep );
    appendFutureTimeSteps( timeSteps );
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::timeStepsX() const
{
    std::vector<time_t> timeSteps = getTimeStepsInRange( RimSummaryCurve::timeStepsX(), m_minTimeStep, m_maxTimeStep );
    appendFutureTimeSteps( timeSteps );
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::createDeclineCurveValues( const std::vector<double>& values,
                                                                      const std::vector<time_t>& timeSteps,
                                                                      time_t                     minTimeStep,
                                                                      time_t                     maxTimeStep,
                                                                      bool                       isAccumulatedResult ) const
{
    if ( values.empty() || timeSteps.empty() ) return values;

    // Use only the values inside the range specified
    auto [timeStepsInRange, valuesInRange] = getInRangeValues( timeSteps, values, m_minTimeStep, m_maxTimeStep );

    if ( timeStepsInRange.empty() || valuesInRange.empty() ) return values;

    auto [initialProductionRate, initialDeclineRate] =
        computeInitialProductionAndDeclineRate( valuesInRange, timeStepsInRange, isAccumulatedResult );
    if ( std::isinf( initialProductionRate ) || std::isnan( initialProductionRate ) || std::isinf( initialDeclineRate ) ||
         std::isnan( initialDeclineRate ) )
    {
        return values;
    }

    QDateTime initialTime = RiaQDateTimeTools::fromTime_t( timeStepsInRange.back() );

    std::set<QDateTime> futureTimeSteps = createFutureTimeSteps( timeStepsInRange );
    std::vector<double> outValues       = valuesInRange;
    for ( const QDateTime& futureTime : futureTimeSteps )
    {
        double timeSinceStart = futureTime.toSecsSinceEpoch() - initialTime.toSecsSinceEpoch();
        double predictedValue = computePredictedValue( initialProductionRate, initialDeclineRate, timeSinceStart, isAccumulatedResult );
        if ( isAccumulatedResult ) predictedValue += valuesInRange.back();
        outValues.push_back( predictedValue );
    }

    return outValues;
}

std::pair<double, double> RimSummaryDeclineCurve::computeInitialProductionAndDeclineRate( const std::vector<double>& values,
                                                                                          const std::vector<time_t>& timeSteps,
                                                                                          bool                       isAccumulatedResult )
{
    CAF_ASSERT( values.size() == timeSteps.size() );

    auto computeProductionRate = []( double t0, double v0, double t1, double v1 ) { return ( v1 - v0 ) / ( t1 - t0 ); };

    if ( !isAccumulatedResult )
    {
        // Use the first (time step, value) pair as t0
        const size_t    idx0 = 0;
        const QDateTime t0   = RiaQDateTimeTools::fromTime_t( timeSteps[idx0] );
        const double    v0   = values[idx0];

        // Last point on the existing curve (within user-specified range) is the initial production rate (for non-accumulated data).
        const QDateTime initialTime           = RiaQDateTimeTools::fromTime_t( timeSteps.back() );
        const double    initialProductionRate = values.back();

        // Compute the decline rate using the rates at the two points
        double initialDeclineRate =
            RigDeclineCurveCalculator::computeDeclineRate( t0.toSecsSinceEpoch(), v0, initialTime.toSecsSinceEpoch(), initialProductionRate );
        return { initialProductionRate, initialDeclineRate };
    }
    else
    {
        // Select a point (t0) 1/4 into the user-specified range
        const double    historyStep = 0.25;
        const size_t    idx0        = static_cast<size_t>( timeSteps.size() * historyStep );
        const QDateTime t0          = RiaQDateTimeTools::fromTime_t( timeSteps[idx0] );
        const double    v0          = values[idx0];

        // For accumulated result: compute the initial production rate from the two points.
        const QDateTime initialTime  = RiaQDateTimeTools::fromTime_t( timeSteps.back() );
        double initialProductionRate = computeProductionRate( t0.toSecsSinceEpoch(), v0, initialTime.toSecsSinceEpoch(), values.back() );

        // Compute the at production rate at time t0 by using a point even further back in the existing curve (tx).
        size_t    idxX            = 0;
        QDateTime tx              = RiaQDateTimeTools::fromTime_t( timeSteps[idxX] );
        double    vx              = values[idxX];
        double    productionRate0 = computeProductionRate( tx.toSecsSinceEpoch(), vx, t0.toSecsSinceEpoch(), v0 );

        // Compute the decline rate using the rates at the two points
        double initialDeclineRate = RigDeclineCurveCalculator::computeDeclineRate( t0.toSecsSinceEpoch(),
                                                                                   productionRate0,
                                                                                   initialTime.toSecsSinceEpoch(),
                                                                                   initialProductionRate );
        return { initialProductionRate, initialDeclineRate };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSummaryDeclineCurve::computePredictedValue( double initialProductionRate,
                                                      double initialDeclineRate,
                                                      double timeSinceStart,
                                                      bool   isAccumulatedResult ) const
{
    if ( isAccumulatedResult )
    {
        if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::EXPONENTIAL )
        {
            return RigDeclineCurveCalculator::computeCumulativeProductionExponentialDecline( initialProductionRate,
                                                                                             initialDeclineRate,
                                                                                             timeSinceStart );
        }
        else if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::HARMONIC )
        {
            return RigDeclineCurveCalculator::computeCumulativeProductionHarmonicDecline( initialProductionRate,
                                                                                          initialDeclineRate,
                                                                                          timeSinceStart );
        }
        else if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC )
        {
            return RigDeclineCurveCalculator::computeCumulativeProductionHyperbolicDecline( initialProductionRate,
                                                                                            initialDeclineRate,
                                                                                            timeSinceStart,
                                                                                            m_hyperbolicDeclineConstant );
        }
    }
    else
    {
        if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::EXPONENTIAL )
        {
            return RigDeclineCurveCalculator::computeFlowRateExponentialDecline( initialProductionRate, initialDeclineRate, timeSinceStart );
        }
        else if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::HARMONIC )
        {
            return RigDeclineCurveCalculator::computeFlowRateHarmonicDecline( initialProductionRate, initialDeclineRate, timeSinceStart );
        }
        else if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC )
        {
            return RigDeclineCurveCalculator::computeFlowRateHyperbolicDecline( initialProductionRate,
                                                                                initialDeclineRate,
                                                                                timeSinceStart,
                                                                                m_hyperbolicDeclineConstant );
        }
    }

    return 0.0;
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

    caf::PdmUiGroup* declineCurveGroup = uiOrdering.addNewGroup( "Decline Curve" );
    declineCurveGroup->add( &m_declineCurveType );
    declineCurveGroup->add( &m_predictionYears );

    if ( m_declineCurveType == RimSummaryDeclineCurve::DeclineCurveType::HYPERBOLIC )
    {
        declineCurveGroup->add( &m_hyperbolicDeclineConstant );
    }

    caf::PdmUiGroup* timeSelectionGroup = uiOrdering.addNewGroup( "Time Selection" );
    timeSelectionGroup->add( &m_minTimeStep );
    timeSelectionGroup->add( &m_maxTimeStep );
    timeSelectionGroup->add( &m_showTimeSelectionInPlot );

    RimSummaryCurve::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_minTimeStep == changedField && m_minTimeStep > m_maxTimeStep )
    {
        m_maxTimeStep = m_minTimeStep;
    }

    if ( &m_maxTimeStep == changedField && m_maxTimeStep < m_minTimeStep )
    {
        m_minTimeStep = m_maxTimeStep;
    }

    RimSummaryCurve::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_declineCurveType || changedField == &m_predictionYears || changedField == &m_hyperbolicDeclineConstant ||
         changedField == &m_minTimeStep || changedField == &m_maxTimeStep || changedField == &m_showTimeSelectionInPlot )
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

    if ( field == &m_predictionYears )
    {
        if ( auto* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            // Predict into the future should be a positive number.
            lineEditorAttr->validator = new QIntValidator( 1, 50, nullptr );
        }
    }

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
    else if ( field == &m_minTimeStep || field == &m_maxTimeStep )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute ) )
        {
            auto timeSteps = RimSummaryCurve::timeStepsY();
            if ( !timeSteps.empty() )
            {
                myAttr->m_minimum = *timeSteps.begin();
                myAttr->m_maximum = *timeSteps.rbegin();
            }
            myAttr->m_showSpinBox = false;
        }
    }
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
    return RimSummaryCurve::curveExportDescription() + "." + m_declineCurveType().uiText() + "_Decline";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::updateTimeAnnotations()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    if ( m_timeRangeAnnotation ) plot->removeTimeAnnotation( m_timeRangeAnnotation );

    if ( m_showTimeSelectionInPlot )
    {
        m_timeRangeAnnotation = plot->addTimeRangeAnnotation( m_minTimeStep, m_maxTimeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::updateDefaultValues()
{
    auto timeSteps = RimSummaryCurve::timeStepsY();
    if ( !timeSteps.empty() )
    {
        // Default min time step is 3/4 into the data
        const double historyStep = 0.75;
        const size_t idx         = static_cast<size_t>( timeSteps.size() * historyStep );
        m_minTimeStep            = timeSteps[idx];
        m_maxTimeStep            = timeSteps.back();
    }
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

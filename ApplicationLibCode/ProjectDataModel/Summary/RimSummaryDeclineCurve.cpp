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

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryDeclineCurve::~RimSummaryDeclineCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::valuesY() const
{
    return createDeclineCurveValues( RimSummaryCurve::valuesY(),
                                     RimSummaryCurve::timeStepsY(),
                                     RiaSummaryTools::hasAccumulatedData( summaryAddressY() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::valuesX() const
{
    return createDeclineCurveValues( RimSummaryCurve::valuesX(),
                                     RimSummaryCurve::timeStepsX(),
                                     RiaSummaryTools::hasAccumulatedData( summaryAddressX() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::timeStepsY() const
{
    std::vector<time_t> timeSteps = RimSummaryCurve::timeStepsY();
    appendFutureTimeSteps( timeSteps );
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryDeclineCurve::timeStepsX() const
{
    std::vector<time_t> timeSteps = RimSummaryCurve::timeStepsX();
    appendFutureTimeSteps( timeSteps );
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryDeclineCurve::createDeclineCurveValues( const std::vector<double>& values,
                                                                      const std::vector<time_t>& timeSteps,
                                                                      bool                       isAccumulatedResult ) const
{
    if ( values.empty() ) return values;
    if ( timeSteps.empty() ) return values;

    auto [initialProductionRate, initialDeclineRate] = computeInitialProductionAndDeclineRate( values, timeSteps, isAccumulatedResult );
    if ( std::isinf( initialProductionRate ) || std::isnan( initialProductionRate ) || std::isinf( initialDeclineRate ) ||
         std::isnan( initialDeclineRate ) )
    {
        return values;
    }

    QDateTime initialTime = RiaQDateTimeTools::fromTime_t( timeSteps.back() );

    std::set<QDateTime> futureTimeSteps = createFutureTimeSteps( timeSteps );
    std::vector<double> outValues       = values;
    for ( const QDateTime& futureTime : futureTimeSteps )
    {
        double timeSinceStart = futureTime.toSecsSinceEpoch() - initialTime.toSecsSinceEpoch();
        double predictedValue = computePredictedValue( initialProductionRate, initialDeclineRate, timeSinceStart, isAccumulatedResult );
        if ( isAccumulatedResult ) predictedValue += values.back();
        outValues.push_back( predictedValue );
    }

    return outValues;
}

std::pair<double, double> RimSummaryDeclineCurve::computeInitialProductionAndDeclineRate( const std::vector<double>& values,
                                                                                          const std::vector<time_t>& timeSteps,
                                                                                          bool                       isAccumulatedResult )
{
    auto computeProductionRate = []( double t0, double v0, double t1, double v1 ) { return ( v1 - v0 ) / ( t1 - t0 ); };

    const double historyStep = 0.25;

    // Select a point a 1/4 back in the existing curve.
    const size_t    idx0 = static_cast<size_t>( timeSteps.size() * ( 1.0 - historyStep ) );
    const QDateTime t0   = RiaQDateTimeTools::fromTime_t( timeSteps[idx0] );
    const double    v0   = values[idx0];

    const QDateTime initialTime = RiaQDateTimeTools::fromTime_t( timeSteps.back() );

    if ( !isAccumulatedResult )
    {
        // Last point on the existing curve is the initial production rate (for non-accumulated data).
        double initialProductionRate = values.back();

        // Compute the decline rate using the rates at the two points
        double initialDeclineRate =
            RigDeclineCurveCalculator::computeDeclineRate( t0.toSecsSinceEpoch(), v0, initialTime.toSecsSinceEpoch(), initialProductionRate );
        return { initialProductionRate, initialDeclineRate };
    }
    else
    {
        // For accumulated result: compute the initial production rate from the two points.
        double initialProductionRate = computeProductionRate( t0.toSecsSinceEpoch(), v0, initialTime.toSecsSinceEpoch(), values.back() );

        // Compute the at production rate at time t0 by using a point even further back in the existing curve.
        size_t    idxX            = static_cast<size_t>( timeSteps.size() * ( 1.0 - ( historyStep * 2 ) ) );
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

    RimSummaryCurve::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryDeclineCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSummaryCurve::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_declineCurveType || changedField == &m_predictionYears || changedField == &m_hyperbolicDeclineConstant )
    {
        loadAndUpdateDataAndPlot();
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

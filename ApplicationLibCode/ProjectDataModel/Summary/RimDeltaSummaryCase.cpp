/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimDeltaSummaryCase.h"

#include "RiaCurveMerger.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimProject.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiTreeSelectionEditor.h"

#include <QDateTime>

#include <algorithm>
#include <memory>

namespace caf
{
template <>
void caf::AppEnum<DerivedSummaryOperator>::setUp()
{
    addItem( DerivedSummaryOperator::DERIVED_OPERATOR_SUB, "Sub", "-" );
    addItem( DerivedSummaryOperator::DERIVED_OPERATOR_ADD, "Add", "+" );
    setDefault( DerivedSummaryOperator::DERIVED_OPERATOR_SUB );
}

template <>
void caf::AppEnum<RimDeltaSummaryCase::FixedTimeStepMode>::setUp()
{
    addItem( RimDeltaSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_NONE, "FIXED_TIME_STEP_NONE", "None" );
    addItem( RimDeltaSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_CASE_1, "FIXED_TIME_STEP_CASE_1", "Summary Case 1" );
    addItem( RimDeltaSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_CASE_2, "FIXED_TIME_STEP_CASE_2", "Summary Case 2" );
    setDefault( RimDeltaSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_NONE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimDeltaSummaryCase, "RimDeltaSummaryCase", "RimDerivedEnsembleCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimDeltaSummaryCase::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimDeltaSummaryCase::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !resultAddress.isValid() )
    {
        return {};
    }

    if ( needsCalculation( resultAddress ) )
    {
        calculate( resultAddress );
    }

    if ( m_dataCache.count( resultAddress ) == 0 )
    {
        return {};
    }

    return m_dataCache.at( resultAddress ).first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RimDeltaSummaryCase::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !resultAddress.isValid() ) return { false, {} };

    if ( auto deltaEnsemble = firstAncestorOfType<RimDeltaSummaryEnsemble>() )
    {
        if ( deltaEnsemble->discardMissingOrIncompleteRealizations() )
        {
            RifSummaryReaderInterface* reader1 = m_summaryCase1 ? m_summaryCase1->summaryReader() : nullptr;
            RifSummaryReaderInterface* reader2 = m_summaryCase2 ? m_summaryCase2->summaryReader() : nullptr;

            if ( !reader1 || !reader2 ) return { false, {} };

            if ( !reader1->hasAddress( resultAddress ) || !reader2->hasAddress( resultAddress ) )
            {
                QString txt = "Summary vector " + QString::fromStdString( resultAddress.toEclipseTextAddress() ) +
                              " is only present in one of the source ensembles, no values are calculated for this vector.";

                RiaLogging::warning( txt );

                return { false, {} };
            }
        }
    }

    if ( needsCalculation( resultAddress ) )
    {
        calculate( resultAddress );
    }

    if ( m_dataCache.count( resultAddress ) == 0 )
    {
        return { false, {} };
    }

    if ( auto deltaEnsemble = firstAncestorOfType<RimDeltaSummaryEnsemble>() )
    {
        if ( deltaEnsemble->discardMissingOrIncompleteRealizations() )
        {
            auto ensembleTimeSteps = deltaEnsemble->ensembleTimeSteps();

            auto caseTimeSteps = m_dataCache.at( resultAddress ).first;

            if ( !ensembleTimeSteps.empty() && !caseTimeSteps.empty() )
            {
                const auto minTime = *std::min_element( ensembleTimeSteps.begin(), ensembleTimeSteps.end() );
                const auto maxTime = *std::max_element( ensembleTimeSteps.begin(), ensembleTimeSteps.end() );

                // The last time step for the individual realizations in an ensemble is usually identical. Add a small threshold to improve
                // robustness.
                const auto timeThreshold = RiaSummaryTools::calculateTimeThreshold( minTime, maxTime );

                if ( *caseTimeSteps.rbegin() < timeThreshold )
                {
                    QString txt = "Summary vector " + QString::fromStdString( resultAddress.toEclipseTextAddress() ) +
                                  " has different time steps in the source ensembles, no values are calculated for this vector.";

                    RiaLogging::warning( txt );

                    return { false, {} };
                }
            }
        }
    }

    return { true, m_dataCache.at( resultAddress ).second };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimDeltaSummaryCase::unitSystem() const
{
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDeltaSummaryCase::RimDeltaSummaryCase()
    : m_summaryCase1( nullptr )
    , m_summaryCase2( nullptr )
{
    CAF_PDM_InitObject( "Summary Case", ":/SummaryCase.svg" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase1, "SummaryCase1", "Summary Case 1" );

    CAF_PDM_InitFieldNoDefault( &m_operator, "Operator", "Operator" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase2, "SummaryCase2", "Summary Case 2" );

    CAF_PDM_InitFieldNoDefault( &m_useFixedTimeStep, "UseFixedTimeStep", "Use Fixed Time Step" );
    CAF_PDM_InitField( &m_fixedTimeStepIndex, "FixedTimeStepIndex", 0, "Time Step" );
    CAF_PDM_InitField( &m_inUse, "InUse", false, "In Use" );
    m_fixedTimeStepIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_fixedTimeStepIndex.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::setInUse( bool inUse )
{
    m_inUse = inUse;

    if ( !m_inUse )
    {
        m_summaryCase1 = nullptr;
        m_summaryCase2 = nullptr;
        m_dataCache.clear();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDeltaSummaryCase::isInUse() const
{
    return m_inUse;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::setSummaryCases( RimSummaryCase* sumCase1, RimSummaryCase* sumCase2 )
{
    m_summaryCase1 = sumCase1;
    m_summaryCase2 = sumCase2;

    clearCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDeltaSummaryCase::needsCalculation( const RifEclipseSummaryAddress& address ) const
{
    return m_dataCache.count( address ) == 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::calculate( const RifEclipseSummaryAddress& address ) const
{
    clearData( address );

    RifSummaryReaderInterface* reader1 = m_summaryCase1 ? m_summaryCase1->summaryReader() : nullptr;
    RifSummaryReaderInterface* reader2 = m_summaryCase2 ? m_summaryCase2->summaryReader() : nullptr;

    int fixedTimeStepCase1 = -1;
    int fixedTimeStepCase2 = -1;
    if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_1 )
    {
        fixedTimeStepCase1 = m_fixedTimeStepIndex;
    }
    else if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_2 )
    {
        fixedTimeStepCase2 = m_fixedTimeStepIndex;
    }

    bool includeIncompleteCurves = true;
    if ( auto deltaEnsemble = firstAncestorOfType<RimDeltaSummaryEnsemble>() )
    {
        includeIncompleteCurves = !deltaEnsemble->discardMissingOrIncompleteRealizations();
    }

    auto itAndIsInsertedPair = m_dataCache.insert(
        std::make_pair( address,
                        calculateDerivedValues( reader1, fixedTimeStepCase1, reader2, fixedTimeStepCase2, m_operator(), address, includeIncompleteCurves ) ) );

    // Check if we got any data. If not, erase the map entry to comply with previous behavior

    if ( itAndIsInsertedPair.first->second.first.empty() )
    {
        m_dataCache.erase( itAndIsInsertedPair.first );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>> RimDeltaSummaryCase::calculateDerivedValues( RifSummaryReaderInterface* reader1,
                                                                                                 int fixedTimeStepCase1,
                                                                                                 RifSummaryReaderInterface* reader2,
                                                                                                 int                    fixedTimeStepCase2,
                                                                                                 DerivedSummaryOperator summaryOperator,
                                                                                                 const RifEclipseSummaryAddress& address,
                                                                                                 bool includeIncompleteCurves )
{
    using ResultPair = std::pair<std::vector<time_t>, std::vector<double>>;

    if ( !reader1 || !reader2 ) return ResultPair();

    if ( !reader1->hasAddress( address ) && !reader2->hasAddress( address ) )
    {
        return ResultPair();
    }

    if ( reader1->hasAddress( address ) && !reader2->hasAddress( address ) )
    {
        auto [isOk, summaryValues] = reader1->values( address );
        return ResultPair( reader1->timeSteps( address ), summaryValues );
    }
    else if ( !reader1->hasAddress( address ) && reader2->hasAddress( address ) )
    {
        auto [isOk, summaryValues] = reader2->values( address );
        if ( summaryOperator == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        {
            for ( auto& v : summaryValues )
            {
                v = -v;
            }
        }

        return ResultPair( reader2->timeSteps( address ), summaryValues );
    }

    auto [isOk1, values1] = reader1->values( address );
    auto [isOk2, values2] = reader2->values( address );

    if ( values1.empty() && values2.empty() )
    {
        return ResultPair();
    }

    auto                      interpolationMethod = address.hasAccumulatedData() ? RiaCurveDefines::InterpolationMethod::LINEAR
                                                                                 : RiaCurveDefines::InterpolationMethod::STEP_RIGHT;
    RiaTimeHistoryCurveMerger merger( interpolationMethod );
    merger.addCurveData( reader1->timeSteps( address ), values1 );
    merger.addCurveData( reader2->timeSteps( address ), values2 );
    merger.computeInterpolatedValues( includeIncompleteCurves );
    if ( merger.curveCount() < 2 )
    {
        // If we have less than two curves, we cannot calculate a delta
        return ResultPair();
    }

    size_t sampleCount = merger.allXValues().size();
    if ( sampleCount == 0 )
    {
        return ResultPair();
    }

    std::vector<double> calculatedValues;
    calculatedValues.reserve( sampleCount );

    int clampedIndexCase1 = std::min( fixedTimeStepCase1, static_cast<int>( values1.size() ) );
    int clampedIndexCase2 = std::min( fixedTimeStepCase2, static_cast<int>( values2.size() ) );

    const std::vector<double>& allValues1 = merger.interpolatedYValuesForAllXValues( 0 );
    const std::vector<double>& allValues2 = merger.interpolatedYValuesForAllXValues( 1 );

    for ( size_t i = 0; i < sampleCount; i++ )
    {
        double valueCase1 = clampedIndexCase1 >= 0 ? values1[clampedIndexCase1] : allValues1[i];
        double valueCase2 = clampedIndexCase2 >= 0 ? values2[clampedIndexCase2] : allValues2[i];
        if ( summaryOperator == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        {
            calculatedValues.push_back( valueCase1 - valueCase2 );
        }
        else if ( summaryOperator == DerivedSummaryOperator::DERIVED_OPERATOR_ADD )
        {
            calculatedValues.push_back( valueCase1 + valueCase2 );
        }
    }

    return ResultPair( merger.allXValues(), calculatedValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDeltaSummaryCase::caseName() const
{
    return m_displayName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimDeltaSummaryCase::keywordCount() const
{
    return m_allResultAddresses.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::createSummaryReaderInterface()
{
    m_allResultAddresses.clear();

    if ( m_summaryCase1 )
    {
        if ( !m_summaryCase1->summaryReader() )
        {
            m_summaryCase1->createSummaryReaderInterface();
        }

        if ( m_summaryCase1->summaryReader() )
        {
            m_summaryCase1->summaryReader()->createAddressesIfRequired();

            auto adr = m_summaryCase1->summaryReader()->allResultAddresses();
            m_allResultAddresses.insert( adr.begin(), adr.end() );
        }
    }
    if ( m_summaryCase2 )
    {
        if ( !m_summaryCase2->summaryReader() )
        {
            m_summaryCase2->createSummaryReaderInterface();
        }

        if ( m_summaryCase2->summaryReader() )
        {
            m_summaryCase2->summaryReader()->createAddressesIfRequired();

            auto adr = m_summaryCase2->summaryReader()->allResultAddresses();
            m_allResultAddresses.insert( adr.begin(), adr.end() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimDeltaSummaryCase::summaryReader()
{
    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::setOperator( DerivedSummaryOperator oper )
{
    m_operator = oper;

    clearCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::setFixedTimeSteps( int fixedTimeStepCase1, int fixedTimeStepCase2 )
{
    m_useFixedTimeStep = FixedTimeStepMode::FIXED_TIME_STEP_NONE;

    if ( fixedTimeStepCase1 >= 0 )
    {
        m_useFixedTimeStep   = FixedTimeStepMode::FIXED_TIME_STEP_CASE_1;
        m_fixedTimeStepIndex = fixedTimeStepCase1;
    }
    else if ( fixedTimeStepCase2 >= 0 )
    {
        m_useFixedTimeStep   = FixedTimeStepMode::FIXED_TIME_STEP_CASE_2;
        m_fixedTimeStepIndex = fixedTimeStepCase2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::clearData( const RifEclipseSummaryAddress& address ) const
{
    m_dataCache.erase( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::clearCache()
{
    m_dataCache.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::updateDisplayNameFromCases()
{
    QString timeStepString;
    {
        RimSummaryCase* sourceEnsemble = nullptr;
        if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_1 )
        {
            sourceEnsemble = m_summaryCase1;
        }
        else if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_2 )
        {
            sourceEnsemble = m_summaryCase2;
        }

        if ( sourceEnsemble )
        {
            auto summaryReader = sourceEnsemble->summaryReader();
            if ( summaryReader )
            {
                const std::vector<time_t>& timeSteps = summaryReader->timeSteps( RifEclipseSummaryAddress() );
                if ( m_fixedTimeStepIndex >= 0 && m_fixedTimeStepIndex < static_cast<int>( timeSteps.size() ) )
                {
                    time_t    selectedTime = timeSteps[m_fixedTimeStepIndex];
                    QDateTime dt           = RiaQDateTimeTools::fromTime_t( selectedTime );
                    QString   formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( { dt } );

                    timeStepString = RiaQDateTimeTools::toStringUsingApplicationLocale( dt, formatString );
                }
            }
        }
    }

    QString case1Name = "None";
    QString case2Name = "None";

    if ( m_summaryCase1 )
    {
        case1Name = m_summaryCase1->displayCaseName();
        if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_1 ) case1Name += "@" + timeStepString;
    }

    if ( m_summaryCase2 )
    {
        case2Name = m_summaryCase2->displayCaseName();
        if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_2 ) case2Name += "@" + timeStepString;
    }

    QString operatorText;
    if ( m_operator() == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        operatorText = "Delta";
    else if ( m_operator() == DerivedSummaryOperator::DERIVED_OPERATOR_ADD )
        operatorText = "Sum";

    QString name;
    if ( case1Name == case2Name && m_summaryCase1 && m_summaryCase2 && m_summaryCase1->ensemble() && m_summaryCase2->ensemble() )
    {
        QString ensembleName1 = m_summaryCase1->ensemble()->name();
        QString ensembleName2 = m_summaryCase2->ensemble()->name();
        name                  = QString( "%1: %2 - %3" ).arg( case1Name ).arg( ensembleName1 ).arg( ensembleName2 );
    }
    else
    {
        name = operatorText + QString( "(%1 , %2)" ).arg( case1Name ).arg( case2Name );
    }

    m_displayName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimDeltaSummaryCase::summaryCase1() const
{
    return m_summaryCase1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimDeltaSummaryCase::summaryCase2() const
{
    return m_summaryCase2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Base class
    uiOrdering.add( &m_displayName );

    uiOrdering.add( &m_summaryCase1 );
    uiOrdering.add( &m_operator );
    uiOrdering.add( &m_summaryCase2 );

    uiOrdering.add( &m_useFixedTimeStep );
    if ( m_useFixedTimeStep() != FixedTimeStepMode::FIXED_TIME_STEP_NONE )
    {
        uiOrdering.add( &m_fixedTimeStepIndex );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimDeltaSummaryCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj         = RimProject::current();
    auto        summaryCases = proj->allSummaryCases();

    if ( fieldNeedingOptions == &m_fixedTimeStepIndex )
    {
        RimSummaryCase* sourceCase = nullptr;

        if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_1 )
        {
            sourceCase = m_summaryCase1;
        }
        else if ( m_useFixedTimeStep() == FixedTimeStepMode::FIXED_TIME_STEP_CASE_2 )
        {
            sourceCase = m_summaryCase2;
        }

        if ( sourceCase && sourceCase->summaryReader() )
        {
            const std::vector<time_t>& timeSteps = sourceCase->summaryReader()->timeSteps( RifEclipseSummaryAddress() );

            options = RiaQDateTimeTools::createOptionItems( timeSteps );
        }
    }

    if ( fieldNeedingOptions == &m_summaryCase1 || fieldNeedingOptions == &m_summaryCase2 )
    {
        for ( auto c : summaryCases )
        {
            if ( c != this ) options.push_back( caf::PdmOptionItemInfo( c->displayCaseName(), c ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    bool reloadData = false;
    if ( changedField == &m_summaryCase2 || changedField == &m_summaryCase1 )
    {
        createSummaryReaderInterface();

        reloadData = true;
    }
    else if ( changedField == &m_useFixedTimeStep || changedField == &m_fixedTimeStepIndex )
    {
        reloadData = true;
    }
    else if ( changedField == &m_operator )
    {
        reloadData = true;
    }
    else
    {
        RimSummaryCase::fieldChangedByUi( changedField, oldValue, newValue );
    }

    if ( reloadData )
    {
        updateDisplayNameFromCases();

        m_dataCache.clear();

        std::vector<caf::PdmObjectHandle*> referringObjects = objectsWithReferringPtrFields();

        std::set<RimSummaryPlot*> plotsToUpdate;
        for ( auto o : referringObjects )
        {
            RimSummaryPlot* sumPlot = o->firstAncestorOrThisOfType<RimSummaryPlot>();

            if ( sumPlot )
            {
                plotsToUpdate.insert( sumPlot );
            }
        }

        for ( auto p : plotsToUpdate )
        {
            p->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDeltaSummaryCase::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_fixedTimeStepIndex == field )
    {
        auto a = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        if ( a )
        {
            a->singleSelectionMode = true;
            a->showTextFilter      = true;
        }
    }
}

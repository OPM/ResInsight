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

#include "RimDerivedSummaryCase.h"

#include "RiaCurveMerger.h"
#include "RiaLogging.h"

#include "RifDerivedEnsembleReader.h"

#include "RimProject.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiTreeSelectionEditor.h"

#include <QDateTime>

#include <algorithm>

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
void caf::AppEnum<RimDerivedSummaryCase::FixedTimeStepMode>::setUp()
{
    addItem( RimDerivedSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_NONE, "FIXED_TIME_STEP_NONE", "None" );
    addItem( RimDerivedSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_CASE_1, "FIXED_TIME_STEP_CASE_1", "Summary Case 1" );
    addItem( RimDerivedSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_CASE_2, "FIXED_TIME_STEP_CASE_2", "Summary Case 2" );
    setDefault( RimDerivedSummaryCase::FixedTimeStepMode::FIXED_TIME_STEP_NONE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimDerivedSummaryCase, "RimDerivedEnsembleCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDerivedSummaryCase::RimDerivedSummaryCase()
    : m_summaryCase1( nullptr )
    , m_summaryCase2( nullptr )
{
    CAF_PDM_InitObject( "Summary Case", ":/SummaryCase.svg", "", "" );
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
RimDerivedSummaryCase::~RimDerivedSummaryCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::setInUse( bool inUse )
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
bool RimDerivedSummaryCase::isInUse() const
{
    return m_inUse;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::setSummaryCases( RimSummaryCase* sumCase1, RimSummaryCase* sumCase2 )
{
    m_summaryCase1 = sumCase1;
    m_summaryCase2 = sumCase2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDerivedSummaryCase::needsCalculation( const RifEclipseSummaryAddress& address ) const
{
    return m_dataCache.count( address ) == 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimDerivedSummaryCase::timeSteps( const RifEclipseSummaryAddress& address ) const
{
    if ( m_dataCache.count( address ) == 0 )
    {
        static std::vector<time_t> empty;
        return empty;
    }

    return m_dataCache.at( address ).first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimDerivedSummaryCase::values( const RifEclipseSummaryAddress& address ) const
{
    if ( m_dataCache.count( address ) == 0 )
    {
        static std::vector<double> empty;
        return empty;
    }

    return m_dataCache.at( address ).second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::calculate( const RifEclipseSummaryAddress& address )
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

    auto itAndIsInsertedPair = m_dataCache.insert(
        std::make_pair( address,
                        calculateDerivedValues( reader1, fixedTimeStepCase1, reader2, fixedTimeStepCase2, m_operator(), address ) ) );

    // Check if we got any data. If not, erase the map entry to comply with previous behavior

    if ( !itAndIsInsertedPair.first->second.first.size() )
    {
        m_dataCache.erase( itAndIsInsertedPair.first );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<time_t>, std::vector<double>>
    RimDerivedSummaryCase::calculateDerivedValues( RifSummaryReaderInterface*      reader1,
                                                   int                             fixedTimeStepCase1,
                                                   RifSummaryReaderInterface*      reader2,
                                                   int                             fixedTimeStepCase2,
                                                   DerivedSummaryOperator          m_operator,
                                                   const RifEclipseSummaryAddress& address )
{
    using ResultPair = std::pair<std::vector<time_t>, std::vector<double>>;

    if ( !reader1 || !reader2 ) return ResultPair();

    if ( !reader1->hasAddress( address ) && !reader2->hasAddress( address ) )
    {
        return ResultPair();
    }

    if ( reader1->hasAddress( address ) && !reader2->hasAddress( address ) )
    {
        std::vector<double> summaryValues;
        reader1->values( address, &summaryValues );

        return ResultPair( reader1->timeSteps( address ), summaryValues );
    }
    else if ( !reader1->hasAddress( address ) && reader2->hasAddress( address ) )
    {
        std::vector<double> summaryValues;
        reader2->values( address, &summaryValues );

        if ( m_operator == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        {
            for ( auto& v : summaryValues )
            {
                v = -v;
            }
        }

        return ResultPair( reader2->timeSteps( address ), summaryValues );
    }

    RiaTimeHistoryCurveMerger merger;
    std::vector<double>       values1;
    std::vector<double>       values2;

    reader1->values( address, &values1 );
    reader2->values( address, &values2 );

    if ( values1.empty() && values2.empty() )
    {
        return ResultPair();
    }

    merger.addCurveData( reader1->timeSteps( address ), values1 );
    merger.addCurveData( reader2->timeSteps( address ), values2 );
    merger.computeInterpolatedValues();

    const std::vector<double>& allValues1 = merger.interpolatedYValuesForAllXValues( 0 );
    const std::vector<double>& allValues2 = merger.interpolatedYValuesForAllXValues( 1 );

    size_t sampleCount = merger.allXValues().size();

    std::vector<double> calculatedValues;
    calculatedValues.reserve( sampleCount );

    int clampedIndexCase1 = std::min( fixedTimeStepCase1, static_cast<int>( values1.size() ) );
    int clampedIndexCase2 = std::min( fixedTimeStepCase2, static_cast<int>( values2.size() ) );

    for ( size_t i = 0; i < sampleCount; i++ )
    {
        double valueCase1 = clampedIndexCase1 >= 0 ? values1[clampedIndexCase1] : allValues1[i];
        double valueCase2 = clampedIndexCase2 >= 0 ? values2[clampedIndexCase2] : allValues2[i];
        if ( m_operator == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        {
            calculatedValues.push_back( valueCase1 - valueCase2 );
        }
        else if ( m_operator == DerivedSummaryOperator::DERIVED_OPERATOR_ADD )
        {
            calculatedValues.push_back( valueCase1 + valueCase2 );
        }
    }

    return ResultPair( merger.allXValues(), calculatedValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDerivedSummaryCase::caseName() const
{
    return m_displayName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::createSummaryReaderInterface()
{
    RifSummaryReaderInterface* summaryCase1Reader1 = nullptr;
    RifSummaryReaderInterface* summaryCase1Reader2 = nullptr;
    if ( m_summaryCase1 )
    {
        if ( !m_summaryCase1->summaryReader() )
        {
            m_summaryCase1->createSummaryReaderInterface();
        }

        summaryCase1Reader1 = m_summaryCase1->summaryReader();
    }
    if ( m_summaryCase2 )
    {
        if ( !m_summaryCase2->summaryReader() )
        {
            m_summaryCase2->createSummaryReaderInterface();
        }

        summaryCase1Reader2 = m_summaryCase2->summaryReader();
    }

    m_reader.reset( new RifDerivedEnsembleReader( this, summaryCase1Reader1, summaryCase1Reader2 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimDerivedSummaryCase::summaryReader()
{
    if ( !m_reader )
    {
        createSummaryReaderInterface();
    }
    return m_reader.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    // NOP
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::setOperator( DerivedSummaryOperator oper )
{
    m_operator = oper;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::setFixedTimeSteps( int fixedTimeStepCase1, int fixedTimeStepCase2 )
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
void RimDerivedSummaryCase::clearData( const RifEclipseSummaryAddress& address )
{
    m_dataCache.erase( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::updateDisplayNameFromCases()
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
    if ( case1Name == case2Name && m_summaryCase1 && m_summaryCase2 && m_summaryCase1->ensemble() &&
         m_summaryCase2->ensemble() )
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
RimSummaryCase* RimDerivedSummaryCase::summaryCase1() const
{
    return m_summaryCase1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimDerivedSummaryCase::summaryCase2() const
{
    return m_summaryCase2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
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
QList<caf::PdmOptionItemInfo>
    RimDerivedSummaryCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
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
void RimDerivedSummaryCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
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

        std::vector<caf::PdmObjectHandle*> referringObjects;
        this->objectsWithReferringPtrFields( referringObjects );

        std::set<RimSummaryPlot*> plotsToUpdate;
        for ( auto o : referringObjects )
        {
            RimSummaryPlot* sumPlot = nullptr;
            o->firstAncestorOrThisOfType( sumPlot );

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
void RimDerivedSummaryCase::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                   QString                    uiConfigName,
                                                   caf::PdmUiEditorAttribute* attribute )
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

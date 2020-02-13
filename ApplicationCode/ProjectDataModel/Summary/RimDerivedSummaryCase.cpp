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

#include "RiaApplication.h"
#include "RiaCurveMerger.h"
#include "RiaLogging.h"

#include "RifDerivedEnsembleReader.h"

#include "RimProject.h"
#include "RimSummaryPlot.h"

namespace caf
{
template <>
void caf::AppEnum<DerivedSummaryOperator>::setUp()
{
    addItem( DerivedSummaryOperator::DERIVED_OPERATOR_SUB, "Sub", "-" );
    addItem( DerivedSummaryOperator::DERIVED_OPERATOR_ADD, "Add", "+" );
    setDefault( DerivedSummaryOperator::DERIVED_OPERATOR_SUB );
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
    CAF_PDM_InitObject( "Summary Case", ":/SummaryCase16x16.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase1, "SummaryCase1", "SummaryCase1", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase2, "SummaryCase2", "SummaryCase2", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_operator, "Operator", "Operator", "", "", "" );

    m_inUse = false;
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
    if ( !sumCase1 || !sumCase2 ) return;
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
    if ( !reader1 || !reader2 ) return;

    if ( !reader1->hasAddress( address ) && !reader2->hasAddress( address ) )
    {
        return;
    }
    else if ( reader1->hasAddress( address ) && !reader2->hasAddress( address ) )
    {
        std::vector<double> summaryValues;
        reader1->values( address, &summaryValues );

        auto& dataItem  = m_dataCache[address];
        dataItem.first  = reader1->timeSteps( address );
        dataItem.second = summaryValues;

        return;
    }
    else if ( !reader1->hasAddress( address ) && reader2->hasAddress( address ) )
    {
        std::vector<double> summaryValues;
        reader2->values( address, &summaryValues );

        if ( m_operator() == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        {
            for ( auto& v : summaryValues )
            {
                v = -v;
            }
        }

        auto& dataItem  = m_dataCache[address];
        dataItem.first  = reader2->timeSteps( address );
        dataItem.second = summaryValues;

        return;
    }

    RiaTimeHistoryCurveMerger merger;
    std::vector<double>       values1;
    std::vector<double>       values2;

    reader1->values( address, &values1 );
    reader2->values( address, &values2 );

    merger.addCurveData( reader1->timeSteps( address ), values1 );
    merger.addCurveData( reader2->timeSteps( address ), values2 );
    merger.computeInterpolatedValues();

    const std::vector<double>& allValues1 = merger.interpolatedYValuesForAllXValues( 0 );
    const std::vector<double>& allValues2 = merger.interpolatedYValuesForAllXValues( 1 );

    size_t              sampleCount = merger.allXValues().size();
    std::vector<double> calculatedValues;
    calculatedValues.reserve( sampleCount );
    for ( size_t i = 0; i < sampleCount; i++ )
    {
        if ( m_operator() == DerivedSummaryOperator::DERIVED_OPERATOR_SUB )
        {
            calculatedValues.push_back( allValues1[i] - allValues2[i] );
        }
        else if ( m_operator() == DerivedSummaryOperator::DERIVED_OPERATOR_ADD )
        {
            calculatedValues.push_back( allValues1[i] + allValues2[i] );
        }
    }

    auto& dataItem  = m_dataCache[address];
    dataItem.first  = merger.allXValues();
    dataItem.second = calculatedValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimDerivedSummaryCase::caseName() const
{
    if ( m_summaryCase1 && m_summaryCase2 )
    {
        auto case1Name = m_summaryCase1->displayCaseName();
        auto case2Name = m_summaryCase2->displayCaseName();

        if ( case1Name == case2Name )
            return case1Name;
        else
            return QString( "%1/%2" ).arg( case1Name ).arg( case2Name );
    }

    return m_shortName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::createSummaryReaderInterface()
{
    RifSummaryReaderInterface* summaryCase1Reader = nullptr;
    if ( m_summaryCase1 )
    {
        if ( !m_summaryCase1->summaryReader() )
        {
            m_summaryCase1->createSummaryReaderInterface();
        }

        summaryCase1Reader = m_summaryCase1->summaryReader();
    }

    m_reader.reset( new RifDerivedEnsembleReader( this, summaryCase1Reader ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimDerivedSummaryCase::summaryReader()
{
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
void RimDerivedSummaryCase::clearData( const RifEclipseSummaryAddress& address )
{
    m_dataCache.erase( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDerivedSummaryCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Base class
    uiOrdering.add( &m_shortName );

    // This class
    uiOrdering.add( &m_summaryCase1 );
    uiOrdering.add( &m_operator );
    uiOrdering.add( &m_summaryCase2 );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimDerivedSummaryCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj         = RiaApplication::instance()->project();
    auto        summaryCases = proj->allSummaryCases();

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
        if ( !m_reader )
        {
            createSummaryReaderInterface();
        }

        if ( m_reader )
        {
            m_reader->updateData( m_summaryCase1(), m_summaryCase2() );
        }

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

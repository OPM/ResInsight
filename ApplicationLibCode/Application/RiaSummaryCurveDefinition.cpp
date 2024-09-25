/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaSummaryCurveDefinition.h"
#include "RiaStdStringTools.h"
#include "RiaSummaryCurveAddress.h"

#include "RifSummaryReaderInterface.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition()
    : m_summaryCaseY( nullptr )
    , m_summaryCaseX( nullptr )
    , m_summaryAddressX( RifEclipseSummaryAddress::timeAddress() )
    , m_ensemble( nullptr )
    , m_isEnsembleCurve( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition( RimSummaryCase*                 summaryCaseY,
                                                      const RifEclipseSummaryAddress& summaryAddressY,
                                                      bool                            isEnsembleCurve )
    : m_summaryCaseY( summaryCaseY )
    , m_summaryAddressY( summaryAddressY )
    , m_summaryCaseX( nullptr )
    , m_summaryAddressX( RifEclipseSummaryAddress::timeAddress() )
    , m_ensemble( nullptr )
    , m_isEnsembleCurve( isEnsembleCurve )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition( RimSummaryEnsemble* ensemble, const RifEclipseSummaryAddress& summaryAddressY )
    : m_summaryCaseY( nullptr )
    , m_summaryAddressY( summaryAddressY )
    , m_summaryCaseX( nullptr )
    , m_summaryAddressX( RifEclipseSummaryAddress::timeAddress() )
    , m_ensemble( ensemble )
    , m_isEnsembleCurve( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinition::RiaSummaryCurveDefinition( RimSummaryEnsemble* ensemble, const RiaSummaryCurveAddress& summaryCurveAddress )
    : m_summaryCaseY( nullptr )
    , m_summaryAddressY( summaryCurveAddress.summaryAddressY() )
    , m_summaryCaseX( nullptr )
    , m_summaryAddressX( summaryCurveAddress.summaryAddressX() )
    , m_ensemble( ensemble )
    , m_isEnsembleCurve( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiaSummaryCurveDefinition::summaryCaseY() const
{
    return m_summaryCaseY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RiaSummaryCurveDefinition::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RiaSummaryCurveDefinition::summaryAddressY() const
{
    return m_summaryAddressY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::isEnsembleCurve() const
{
    return m_isEnsembleCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::setSummaryAddressY( const RifEclipseSummaryAddress& address )
{
    m_summaryAddressY = address;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::setSummaryCaseX( RimSummaryCase* summaryCase )
{
    m_summaryCaseX = summaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::setSummaryAddressX( const RifEclipseSummaryAddress& summaryAddress )
{
    m_summaryAddressX = summaryAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RiaSummaryCurveDefinition::summaryCaseX() const
{
    return m_summaryCaseX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RiaSummaryCurveDefinition::summaryAddressX() const
{
    return m_summaryAddressX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress RiaSummaryCurveDefinition::summaryCurveAddress() const
{
    return RiaSummaryCurveAddress( m_summaryAddressX, m_summaryAddressY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinition::setIdentifierText( SummaryCategory category, const std::string& name )
{
    if ( RifEclipseSummaryAddress::isDependentOnWellName( category ) )
    {
        m_summaryAddressY.setWellName( name );
        m_summaryAddressX.setWellName( name );
    }

    int id = RiaStdStringTools::toInt( name );

    switch ( category )
    {
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER:
            m_summaryAddressY.setAquiferNumber( id );
            m_summaryAddressX.setAquiferNumber( id );
            break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION:
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION:
            m_summaryAddressY.setRegion( id );
            m_summaryAddressX.setRegion( id );
            break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP:
            m_summaryAddressY.setGroupName( name );
            m_summaryAddressX.setGroupName( name );
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RiaSummaryCurveDefinition::resultValues( const RiaSummaryCurveDefinition& curveDefinition )
{
    if ( !curveDefinition.summaryAddressY().isValid() ) return {};
    if ( !curveDefinition.summaryCaseY() ) return {};

    RifSummaryReaderInterface* reader = curveDefinition.summaryCaseY()->summaryReader();
    if ( !reader ) return {};

    auto [isOk, values] = reader->values( curveDefinition.summaryAddressY() );
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RiaSummaryCurveDefinition::timeSteps( const RiaSummaryCurveDefinition& curveDefinition )
{
    if ( !curveDefinition.summaryAddressY().isValid() ) return {};
    if ( !curveDefinition.summaryCaseY() ) return {};

    RifSummaryReaderInterface* reader = curveDefinition.summaryCaseY()->summaryReader();
    if ( !reader ) return {};

    return reader->timeSteps( curveDefinition.summaryAddressY() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSummaryCurveDefinition::curveDefinitionText() const
{
    QString caseName;
    if ( summaryCaseY() )
        caseName = summaryCaseY()->displayCaseName();
    else if ( ensemble() )
        caseName = ensemble()->name();

    return RiaSummaryCurveDefinition::curveDefinitionText( caseName, summaryAddressY() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaSummaryCurveDefinition::curveDefinitionText( const QString& caseName, const RifEclipseSummaryAddress& summaryAddress )
{
    QString txt;

    txt += caseName;
    txt += ", ";

    txt += QString::fromStdString( summaryAddress.uiText() );

    return txt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryCurveDefinition::operator<( const RiaSummaryCurveDefinition& other ) const
{
    if ( m_ensemble != other.ensemble() )
    {
        QString ensembleName;
        QString otherEnsembleName;

        if ( m_ensemble )
        {
            ensembleName = m_ensemble->name();
        }

        if ( other.ensemble() )
        {
            otherEnsembleName = other.ensemble()->name();
        }

        // First check if names are different to ensure stable alphabetic sort
        if ( ensembleName != otherEnsembleName )
        {
            return ensembleName < otherEnsembleName;
        }

        // Use pointer address, sorting will be be unstable
        return m_ensemble < other.ensemble();
    }

    if ( m_summaryCaseY != other.summaryCaseY() )
    {
        QString summaryCaseName;
        QString otherSummaryCaseName;

        if ( m_summaryCaseY )
        {
            summaryCaseName = m_summaryCaseY->displayCaseName();
        }
        if ( other.summaryCaseY() )
        {
            otherSummaryCaseName = other.summaryCaseY()->displayCaseName();
        }

        // First check if names are different to ensure stable alphabetic sort
        if ( summaryCaseName != otherSummaryCaseName )
        {
            return summaryCaseName < otherSummaryCaseName;
        }

        // Use pointer address, sorting will be be unstable
        return m_summaryCaseY < other.summaryCaseY();
    }

    if ( m_summaryAddressY != other.summaryAddressY() )
    {
        return ( m_summaryAddressY < other.summaryAddressY() );
    }

    return m_isEnsembleCurve < other.isEnsembleCurve();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryCurveDefinitionAnalyser::setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefs )
{
    m_singleSummaryCases.clear();
    m_ensembles.clear();
    m_vectorNames.clear();
    m_summaryAdresses.clear();

    for ( const auto& curveDef : curveDefs )
    {
        bool valid = false;
        if ( curveDef.ensemble() && curveDef.isEnsembleCurve() )
        {
            m_ensembles.insert( curveDef.ensemble() );
            valid = true;
        }
        else if ( curveDef.summaryCaseY() )
        {
            m_singleSummaryCases.insert( curveDef.summaryCaseY() );

            if ( curveDef.summaryCaseY()->ensemble() )
            {
                m_ensembles.insert( curveDef.summaryCaseY()->ensemble() );
            }
            valid = true;
        }

        if ( valid )
        {
            const RifEclipseSummaryAddress& address = curveDef.summaryAddressY();

            m_vectorNames.insert( address.vectorName() );
            m_summaryAdresses.insert( address );
        }
    }
}

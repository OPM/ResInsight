/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RimRftTopologyCurve.h"

#include "RiaSummaryTools.h"

#include "RifReaderOpmRft.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimSummaryCase.h"

CAF_PDM_SOURCE_INIT( RimRftTopologyCurve, "RimRftTopologyCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTopologyCurve::RimRftTopologyCurve()
{
    CAF_PDM_InitObject( "RFT Topology Curve" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveSummaryCase", "Summary Case" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );

    CAF_PDM_InitField( &m_segmentBranchIndex, "SegmentBranchIndex", -1, "Branch" );
    CAF_PDM_InitFieldNoDefault( &m_segmentBranchType, "SegmentBranchType", "Completion" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::setDataSource( RimSummaryCase*           summaryCase,
                                         const QDateTime&          timeStep,
                                         const QString&            wellName,
                                         int                       segmentBranchIndex,
                                         RiaDefines::RftBranchType branchType )
{
    m_summaryCase        = summaryCase;
    m_timeStep           = timeStep;
    m_wellName           = wellName;
    m_segmentBranchIndex = segmentBranchIndex;
    m_segmentBranchType  = branchType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellName() const
{
    return "Topology curve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellLogChannelUiName() const
{
    return "Topology curve channel";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::wellLogChannelUnits() const
{
    return "Topology curve units";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTopologyCurve::createCurveAutoName()
{
    return "Topology curve auto name";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellLogCurve::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_summaryCase );
    uiOrdering.add( &m_wellName );
    uiOrdering.add( &m_timeStep );
    uiOrdering.add( &m_segmentBranchIndex );
    uiOrdering.add( &m_segmentBranchType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTopologyCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( !m_summaryCase ) return {};

    QList<caf::PdmOptionItemInfo> options;

    auto reader = m_summaryCase->rftReader();

    if ( fieldNeedingOptions == &m_summaryCase )
    {
        options = RiaSummaryTools::optionsForSummaryCases( RimProject::current()->allSummaryCases() );
        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_wellName )
    {
        options = RimRftTools::wellNameOptions( reader );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        options = RimRftTools::segmentTimeStepOptions( reader, m_wellName );
    }
    else if ( fieldNeedingOptions == &m_segmentBranchIndex )
    {
        options = RimRftTools::segmentBranchIndexOptions( reader,
                                                          m_wellName(),
                                                          m_timeStep(),
                                                          RiaDefines::RftBranchType::RFT_UNKNOWN );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTopologyCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( m_summaryCase )
    {
        auto rftReader = dynamic_cast<RifReaderOpmRft*>( m_summaryCase->rftReader() );

        // Update well path attributes, packers and casing based on RFT data
        if ( rftReader )
        {
            auto segment = rftReader->segmentForWell( m_wellName, m_timeStep );

            {
                std::vector<double> seglenstValues;
                std::vector<double> seglenenValues;
                {
                    auto resultName = RifEclipseRftAddress::createSegmentAddress( m_wellName, m_timeStep, "SEGLENST" );

                    rftReader->values( resultName, &seglenstValues );

                    //                     if ( seglenstValues.size() > 2 )
                    //                     {
                    //                         seglenstValues[0] = seglenstValues[1];
                    //                     }
                }
                {
                    auto resultName = RifEclipseRftAddress::createSegmentAddress( m_wellName, m_timeStep, "SEGLENEN" );

                    rftReader->values( resultName, &seglenenValues );
                }

                auto segmentIndices = segment.segmentIndicesForBranchIndex( m_segmentBranchIndex(), m_segmentBranchType() );
                if ( !segmentIndices.empty() )
                {
                    std::vector<double> depths;
                    std::vector<double> propertyValues;

                    double myValue = 1.0;
                    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_TUBING ) myValue = 2.0;
                    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE ) myValue = 3.0;
                    if ( m_segmentBranchType() == RiaDefines::RftBranchType::RFT_ANNULUS ) myValue = 4.0;

                    for ( auto segmentIndex : segmentIndices )
                    {
                        depths.push_back( seglenstValues[segmentIndex] );
                        propertyValues.push_back( myValue + segmentIndex );

                        depths.push_back( seglenenValues[segmentIndex] );
                        propertyValues.push_back( myValue );
                    }

                    RiaDefines::DepthUnitType depthUnit           = RiaDefines::DepthUnitType::UNIT_METER;
                    QString                   xUnits              = RiaWellLogUnitTools<double>::noUnitString();
                    bool                      isExtractionCurve   = false;
                    bool                      useLogarithmicScale = false;

                    setPropertyValuesAndDepths( propertyValues,
                                                depths,
                                                RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                                0.0,
                                                depthUnit,
                                                isExtractionCurve,
                                                useLogarithmicScale );
                }
            }
        }
    }
}

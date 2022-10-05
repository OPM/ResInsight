/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RimRftWellCompletionTrack.h"

#include "RiaSummaryTools.h"

#include "RifReaderOpmRft.h"

#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimSummaryCase.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"

#include "RimRftTopologyCurve.h"
#include "WellLogCommands/RicAppendWellPathFromRftDataFeature.h"

CAF_PDM_SOURCE_INIT( RimRftWellCompletionTrack, "RimRftWellCompletionTrack" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftWellCompletionTrack::RimRftWellCompletionTrack()
{
    CAF_PDM_InitObject( "Rft Track", ":/WellLogTrack16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "CurveSummaryCase", "Summary Case" );
    m_summaryCase.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_wellName, "WellName", "Well Name" );

    CAF_PDM_InitField( &m_segmentBranchIndex, "SegmentBranchIndex", -1, "Branch" );

    setShowWellPathAttributes( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::setDataSource( RimSummaryCase*  summaryCase,
                                               const QDateTime& timeStep,
                                               const QString&   wellName,
                                               int              segmentBranchIndex )
{
    m_summaryCase        = summaryCase;
    m_timeStep           = timeStep;
    m_wellName           = wellName;
    m_segmentBranchIndex = segmentBranchIndex;

    configureForWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathAttribute*>
    RimRftWellCompletionTrack::createWellPathAttributes( RifRftSegment*            segment,
                                                         RiaDefines::RftBranchType branchType,
                                                         int                       branchIndex,
                                                         double                    diameter,
                                                         QColor                    color,

                                                         const QString&             name,
                                                         const std::vector<double>& segmentStart,
                                                         const std::vector<double>& segmentEnd )
{
    std::vector<RimWellPathAttribute*> wellPathAttributes;
    auto segmentIndices = segment->segmentIndicesForBranchIndex( branchIndex, branchType );
    if ( segmentIndices.empty() ) return {};

    auto w = new RimWellPathAttribute;
    w->setComponentType( RiaDefines::WellPathComponentType::SEGMENT );
    w->setCustomLabel( name );
    w->setDiameter( diameter );
    w->setCustomColor( color );

    w->setStartEndMD( segmentStart[segmentIndices.front()], segmentEnd[segmentIndices.back()] );

    wellPathAttributes.push_back( w );

    return wellPathAttributes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::configureForWellPath()
{
    createCurves();
    return;

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

                    if ( seglenstValues.size() > 2 )
                    {
                        seglenstValues[0] = seglenstValues[1];
                    }
                }
                {
                    auto resultName = RifEclipseRftAddress::createSegmentAddress( m_wellName, m_timeStep, "SEGLENEN" );

                    rftReader->values( resultName, &seglenenValues );
                }

                double wellStartMd = 1.0;
                double wellEndMd   = 100.0;

                if ( !seglenstValues.empty() ) wellStartMd = seglenstValues.front();
                if ( !seglenenValues.empty() ) wellEndMd = seglenenValues.back();

                QString decoratedWellName =
                    QString( "[RFT Dummy] - %1 - %2)" ).arg( m_wellName() ).arg( m_segmentBranchIndex() );

                auto wellPath = RicAppendWellPathFromRftDataFeature::findOrCreateWellAttributeWellPath( decoratedWellName,
                                                                                                        wellStartMd,
                                                                                                        wellEndMd );
                setWellPathAttributesSource( wellPath );

                wellPath->attributeCollection()->deleteAllAttributes();

                {
                    // Tubing

                    RiaDefines::RftBranchType branchType = RiaDefines::RftBranchType::RFT_TUBING;
                    double                    diameter   = 1.0;
                    QString                   name       = "Tubing";
                    QColor                    color      = QColor( "limegreen" );

                    auto wellPathAttributes = createWellPathAttributes( &segment,
                                                                        branchType,
                                                                        m_segmentBranchIndex,
                                                                        diameter,
                                                                        color,
                                                                        name,
                                                                        seglenstValues,
                                                                        seglenenValues );
                    for ( auto w : wellPathAttributes )
                    {
                        wellPath->attributeCollection()->insertAttribute( nullptr, w );
                    }
                }

                {
                    // Device

                    RiaDefines::RftBranchType branchType = RiaDefines::RftBranchType::RFT_DEVICE;
                    double                    diameter   = 15.0;
                    QString                   name       = "Device";
                    QColor                    color      = QColor( "mediumvioletred" );

                    auto wellPathAttributes = createWellPathAttributes( &segment,
                                                                        branchType,
                                                                        m_segmentBranchIndex,
                                                                        diameter,
                                                                        color,
                                                                        name,
                                                                        seglenstValues,
                                                                        seglenenValues );
                    for ( auto w : wellPathAttributes )
                    {
                        wellPath->attributeCollection()->insertAttribute( nullptr, w );
                    }
                }

                {
                    // Annulus

                    RiaDefines::RftBranchType branchType = RiaDefines::RftBranchType::RFT_ANNULUS;
                    double                    diameter   = 28.0;
                    QString                   name       = "Annulus";
                    QColor                    color      = QColor( "royalblue" );

                    auto wellPathAttributes = createWellPathAttributes( &segment,
                                                                        branchType,
                                                                        m_segmentBranchIndex,
                                                                        diameter,
                                                                        color,
                                                                        name,
                                                                        seglenstValues,
                                                                        seglenenValues );
                    for ( auto w : wellPathAttributes )
                    {
                        wellPath->attributeCollection()->insertAttribute( nullptr, w );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::createCurves()
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

                    if ( seglenstValues.size() > 2 )
                    {
                        seglenstValues[0] = seglenstValues[1];
                    }
                }
                {
                    auto resultName = RifEclipseRftAddress::createSegmentAddress( m_wellName, m_timeStep, "SEGLENEN" );

                    rftReader->values( resultName, &seglenenValues );
                }

                deleteAllCurves();

                {
                    auto tubingCurve = new RimRftTopologyCurve;
                    tubingCurve->setDataSource( m_summaryCase(),
                                                m_timeStep(),
                                                m_wellName(),
                                                m_segmentBranchIndex(),
                                                RiaDefines::RftBranchType::RFT_TUBING );
                    addCurve( tubingCurve );
                }
                {
                    auto tubingCurve = new RimRftTopologyCurve;
                    tubingCurve->setDataSource( m_summaryCase(),
                                                m_timeStep(),
                                                m_wellName(),
                                                m_segmentBranchIndex(),
                                                RiaDefines::RftBranchType::RFT_DEVICE );
                    addCurve( tubingCurve );
                }

                {
                    auto tubingCurve = new RimRftTopologyCurve;
                    tubingCurve->setDataSource( m_summaryCase(),
                                                m_timeStep(),
                                                m_wellName(),
                                                m_segmentBranchIndex(),
                                                RiaDefines::RftBranchType::RFT_ANNULUS );
                    addCurve( tubingCurve );
                }
            }
        }
    }

    // loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellLogTrack::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_summaryCase );
    uiOrdering.add( &m_wellName );
    uiOrdering.add( &m_timeStep );
    uiOrdering.add( &m_segmentBranchIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimRftWellCompletionTrack::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
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
void RimRftWellCompletionTrack::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    configureForWellPath();
}

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

    setShowWellPathAttributes( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::setDataSource( RimSummaryCase* summaryCase, const QDateTime& timeStep, const QString& wellName )
{
    m_summaryCase = summaryCase;
    m_timeStep    = timeStep;
    m_wellName    = wellName;

    configureForWellPath( m_summaryCase, m_timeStep, m_wellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::configureForWellPath( RimSummaryCase*  summaryCase,
                                                      const QDateTime& timeStep,
                                                      const QString&   wellName )
{
    QString decoratedWellName = "[RFT Dummy] - " + wellName;

    auto wellPath = RicAppendWellPathFromRftDataFeature::findOrCreateWellAttributeWellPath( decoratedWellName );

    if ( m_summaryCase )
    {
        auto rftReader = m_summaryCase->rftReader();

        // Update well path attributes, packers and casing based on RFT data
        if ( rftReader )
        {
            // rftReader->segmentForWell(wellPathName)
        }
    }

    wellPath->attributeCollection()->deleteAllAttributes();
    auto attribute = new RimWellPathAttribute;
    attribute->setDepthsFromWellPath( wellPath );
    wellPath->attributeCollection()->insertAttribute( nullptr, attribute );

    setWellPathAttributesSource( wellPath );
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

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftWellCompletionTrack::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    configureForWellPath( m_summaryCase, m_timeStep, m_wellName );
}

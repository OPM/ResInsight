/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryCurveAutoName.h"

#include "RifEclipseSummaryAddress.h"

#include "RimEnsembleCurveSet.h"
#include "RimProject.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryNameHelper.h"

#include "SummaryPlotCommands/RicSummaryPlotEditorUi.h"

#include "RiuSummaryQuantityNameInfoProvider.h"

#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimSummaryCurveAutoName, "SummaryCurveAutoName" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveAutoName::RimSummaryCurveAutoName()
{
    CAF_PDM_InitObject( "RimSummaryCurveAutoName" );

    CAF_PDM_InitField( &m_longVectorName, "LongVectorName", true, "Long Vector Name" );
    CAF_PDM_InitField( &m_vectorName, "VectorName", false, "Vector Name" );
    CAF_PDM_InitField( &m_unit, "Unit", false, "Unit" );
    CAF_PDM_InitField( &m_regionNumber, "RegionNumber", true, "Region Number" );
    CAF_PDM_InitField( &m_groupName, "WellGroupName", true, "Group Name" );
    CAF_PDM_InitField( &m_wellName, "WellName", true, "Well Name" );
    CAF_PDM_InitField( &m_wellSegmentNumber, "WellSegmentNumber", true, "Well Segment Number" );
    CAF_PDM_InitField( &m_wellCompletionNumber, "WellCompletionNumber", true, "Well Completion Number" );
    CAF_PDM_InitField( &m_lgrName, "LgrName", true, "Lgr Name" );
    CAF_PDM_InitField( &m_connection, "Completion", true, "I, J, K" );
    CAF_PDM_InitField( &m_aquiferNumber, "Aquifer", true, "Aquifer Number" );

    CAF_PDM_InitField( &m_caseName, "CaseName", true, "Case/Ensemble Name" );

    if ( RimProject::current() && RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.1.0" ) )
    {
        // Make sure that behavior in older projects behave as before
        m_longVectorName = false;
        m_vectorName     = true;
    }

    // When multiple curves are selected, we need to issue fieldChanged on all curves
    setNotifyAllFieldsInMultiFieldChangedEvents( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::curveName( const RiaSummaryCurveAddress& summaryCurveAddress,
                                            const RimSummaryNameHelper*   currentNameHelper,
                                            const RimSummaryNameHelper*   plotNameHelper ) const
{
    QString name;

    {
        auto nameForY = curveNameY( summaryCurveAddress.summaryAddressY(), currentNameHelper, plotNameHelper );
        if ( nameForY.isEmpty() )
        {
            nameForY = curveNameY( summaryCurveAddress.summaryAddressY(), nullptr, nullptr );
        }

        name += nameForY;
    }

    if ( summaryCurveAddress.summaryAddressX().category() != SummaryCategory::SUMMARY_TIME )
    {
        auto nameForX = curveNameX( summaryCurveAddress.summaryAddressX(), currentNameHelper, plotNameHelper );
        if ( nameForX.isEmpty() )
        {
            nameForX = curveNameX( summaryCurveAddress.summaryAddressX(), nullptr, nullptr );
        }

        if ( nameForX != name ) name += " | " + nameForX;
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::curveNameY( const RifEclipseSummaryAddress& summaryAddress,
                                             const RimSummaryNameHelper*     currentNameHelper,
                                             const RimSummaryNameHelper*     plotNameHelper ) const
{
    auto summaryCurve = firstAncestorOrThisOfType<RimSummaryCurve>();

    std::string unitNameY;
    if ( summaryCurve )
    {
        unitNameY = summaryCurve->unitNameY();
    }

    std::string caseNameY;
    if ( caseNameY.empty() && summaryCurve && summaryCurve->summaryCaseY() )
    {
        caseNameY = summaryCurve->summaryCaseY()->displayCaseName().toStdString();
    }

    {
        auto ensembleCurveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
        if ( ensembleCurveSet && ensembleCurveSet->summaryEnsemble() )
        {
            caseNameY = ensembleCurveSet->summaryEnsemble()->name().toStdString();
        }
    }

    QString curveName = buildCurveName( summaryAddress, currentNameHelper, plotNameHelper, unitNameY, caseNameY );

    return curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::curveNameX( const RifEclipseSummaryAddress& summaryAddress,
                                             const RimSummaryNameHelper*     currentNameHelper,
                                             const RimSummaryNameHelper*     plotNameHelper ) const
{
    auto summaryCurve = firstAncestorOrThisOfType<RimSummaryCurve>();

    std::string unitNameX;
    if ( summaryCurve )
    {
        unitNameX = summaryCurve->unitNameX();
    }

    std::string caseNameX;
    if ( caseNameX.empty() && summaryCurve && summaryCurve->summaryCaseX() )
    {
        caseNameX = summaryCurve->summaryCaseX()->displayCaseName().toStdString();
    }

    {
        auto ensembleCurveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
        if ( ensembleCurveSet && ensembleCurveSet->summaryEnsemble() )
        {
            caseNameX = ensembleCurveSet->summaryEnsemble()->name().toStdString();
        }
    }

    QString curveName = buildCurveName( summaryAddress, currentNameHelper, plotNameHelper, unitNameX, caseNameX );

    return curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::enableVectorName( bool enable )
{
    m_vectorName = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendWellName( std::string&                    text,
                                              const RifEclipseSummaryAddress& summaryAddress,
                                              const RimSummaryNameHelper*     nameHelper ) const
{
    bool skipSubString = nameHelper && nameHelper->isWellNameInTitle();
    if ( skipSubString ) return;

    if ( m_wellName )
    {
        if ( !text.empty() ) text += ":";
        text += summaryAddress.wellName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendLgrName( std::string& text, const RifEclipseSummaryAddress& summaryAddress ) const
{
    if ( m_lgrName )
    {
        if ( !text.empty() ) text += ":";
        text += ":" + summaryAddress.lgrName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveAutoName::buildCurveName( const RifEclipseSummaryAddress& summaryAddress,
                                                 const RimSummaryNameHelper*     currentNameHelper,
                                                 const RimSummaryNameHelper*     plotNameHelper,
                                                 const std::string&              unitText,
                                                 const std::string&              caseName ) const
{
    std::string text; // Using std::string locally to avoid a lot of conversion when building the curve name

    if ( m_vectorName || m_longVectorName )
    {
        if ( currentNameHelper && currentNameHelper->vectorNames().size() > 1 )
        {
            text = summaryAddress.vectorName();
        }
        else
        {
            bool skipSubString = currentNameHelper && currentNameHelper->isPlotDisplayingSingleCurve();
            if ( !skipSubString )
            {
                if ( m_longVectorName() )
                {
                    auto quantityName = summaryAddress.vectorName();
                    if ( summaryAddress.isHistoryVector() ) quantityName = quantityName.substr( 0, quantityName.size() - 1 );

                    text = RiuSummaryQuantityNameInfoProvider::instance()->longNameFromVectorName( quantityName );

                    if ( m_vectorName ) text += " (" + summaryAddress.vectorName() + ")";

                    // Handle cases where longNameFromVectorName fails to produce a long name.
                    // This can happen for non-standard vector names.
                    if ( text.empty() && !summaryAddress.vectorName().empty() ) text = summaryAddress.vectorName();
                }
                else
                {
                    text = summaryAddress.vectorName();
                }
            }
        }

        if ( summaryAddress.isStatistics() )
        {
            auto prefix = RifEclipseSummaryAddressDefines::statisticsTypeToString( summaryAddress.statisticsType() );
            text        = prefix + ":" + summaryAddress.vectorName();
        }

        if ( m_unit && !unitText.empty() )
        {
            text += "[" + unitText + "]";
        }
    }

    appendAddressDetails( text, summaryAddress, plotNameHelper );

    if ( !caseName.empty() )
    {
        bool skipSubString = plotNameHelper && plotNameHelper->isCaseInTitle();

        if ( m_caseName && !skipSubString )
        {
            const bool isTextEmptyBeforeCaseName = text.empty();

            if ( !text.empty() ) text += ", ";
            text += caseName;

            if ( isTextEmptyBeforeCaseName && currentNameHelper && currentNameHelper->numberOfCases() > 1 &&
                 currentNameHelper->vectorNames().size() > 1 )
            {
                // Add vector name to the case name if there are multiple cases and multiple vectors

                text += ":" + summaryAddress.vectorName();
            }
        }
    }

    if ( text.empty() ) text = summaryAddress.vectorName();

    return QString::fromStdString( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::appendAddressDetails( std::string&                    text,
                                                    const RifEclipseSummaryAddress& summaryAddress,
                                                    const RimSummaryNameHelper*     nameHelper ) const
{
    switch ( summaryAddress.category() )
    {
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_AQUIFER:
        {
            if ( m_aquiferNumber )
            {
                if ( !text.empty() ) text += ":";
                text += std::to_string( summaryAddress.aquiferNumber() );
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION:
        {
            if ( m_regionNumber )
            {
                bool skipSubString = nameHelper && nameHelper->isRegionInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.regionNumber() );
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_REGION_2_REGION:
        {
            if ( m_regionNumber )
            {
                if ( !text.empty() ) text += ":";
                text += std::to_string( summaryAddress.regionNumber() );
                text += "-" + std::to_string( summaryAddress.regionNumber2() );
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_GROUP:
        {
            if ( m_groupName )
            {
                bool skipSubString = nameHelper && nameHelper->isGroupNameInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += summaryAddress.groupName();
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_NETWORK:
        {
            bool skipSubString = nameHelper && nameHelper->isNetworkInTitle();
            if ( !skipSubString )
            {
                if ( !text.empty() ) text += ":";
                text += summaryAddress.networkName();
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL:
        {
            appendWellName( text, summaryAddress, nameHelper );
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION:
        {
            appendWellName( text, summaryAddress, nameHelper );

            if ( m_connection )
            {
                bool skipSubString = nameHelper && nameHelper->isConnectionInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.cellI() ) + ", " + std::to_string( summaryAddress.cellJ() ) + ", " +
                            std::to_string( summaryAddress.cellK() );
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_LGR:
        {
            appendLgrName( text, summaryAddress );
            appendWellName( text, summaryAddress, nameHelper );
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_CONNECTION_LGR:
        {
            appendLgrName( text, summaryAddress );
            appendWellName( text, summaryAddress, nameHelper );

            if ( m_connection )
            {
                bool skipSubString = nameHelper && nameHelper->isConnectionInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.cellI() ) + ", " + std::to_string( summaryAddress.cellJ() ) + ", " +
                            std::to_string( summaryAddress.cellK() );
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_COMPLETION:
        {
            appendWellName( text, summaryAddress, nameHelper );

            if ( m_wellCompletionNumber )
            {
                bool skipSubString = nameHelper && nameHelper->isWellCompletionInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.wellCompletionNumber() );
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL_SEGMENT:
        {
            appendWellName( text, summaryAddress, nameHelper );

            if ( m_wellSegmentNumber )
            {
                bool skipSubString = nameHelper && nameHelper->isSegmentInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.wellSegmentNumber() );
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK:
        {
            if ( m_connection )
            {
                bool skipSubString = nameHelper && nameHelper->isBlockInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.cellI() ) + ", " + std::to_string( summaryAddress.cellJ() ) + ", " +
                            std::to_string( summaryAddress.cellK() );
                }
            }
        }
        break;
        case RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_BLOCK_LGR:
        {
            appendLgrName( text, summaryAddress );

            if ( m_connection )
            {
                bool skipSubString = nameHelper && nameHelper->isBlockInTitle();
                if ( !skipSubString )
                {
                    if ( !text.empty() ) text += ":";
                    text += std::to_string( summaryAddress.cellI() ) + ", " + std::to_string( summaryAddress.cellJ() ) + ", " +
                            std::to_string( summaryAddress.cellK() );
                }
            }
        }
        break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    // NOTE: The curve filter is parent object of a summary curve, and the update is supposed to update
    // the first parent, not the grandparent. This is the reason for not using firstAncestorOrThisOfType()

    RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>( parentField()->ownerObject() );
    if ( summaryCurve )
    {
        summaryCurve->updateCurveNameAndUpdatePlotLegendAndTitle();
        summaryCurve->updateConnectedEditors();

        return;
    }

    RicSummaryPlotEditorUi* curveCreator = dynamic_cast<RicSummaryPlotEditorUi*>( parentField()->ownerObject() );
    if ( curveCreator )
    {
        curveCreator->updateCurveNames();
        curveCreator->updateConnectedEditors();

        return;
    }

    {
        auto ensembleCurveSet = dynamic_cast<RimEnsembleCurveSet*>( parentField()->ownerObject() );
        if ( ensembleCurveSet )
        {
            ensembleCurveSet->updateAllTextInPlot();
            ensembleCurveSet->updateConnectedEditors();

            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveAutoName::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseName );
    uiOrdering.add( &m_vectorName );
    uiOrdering.add( &m_longVectorName );
    uiOrdering.add( &m_groupName );
    uiOrdering.add( &m_wellName );

    caf::PdmUiGroup& advanced = *( uiOrdering.addNewGroup( "Advanced" ) );
    advanced.setCollapsedByDefault();
    advanced.add( &m_regionNumber );
    advanced.add( &m_lgrName );
    advanced.add( &m_connection );
    advanced.add( &m_wellSegmentNumber );
    advanced.add( &m_wellCompletionNumber );
    advanced.add( &m_aquiferNumber );
    advanced.add( &m_unit );

    uiOrdering.skipRemainingFields();
}

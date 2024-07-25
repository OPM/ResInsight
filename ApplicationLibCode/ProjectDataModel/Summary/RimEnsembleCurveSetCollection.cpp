/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleCurveSetCollection.h"

#include "RiaStdStringTools.h"

#include "RimEnsembleCurveSet.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotSourceStepping.h"

#include "RiuPlotCurve.h"

#include "cafPdmFieldReorderCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleCurveSetCollection, "RimEnsembleCurveSetCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSetCollection::RimEnsembleCurveSetCollection()
{
    CAF_PDM_InitObject( "Ensemble Curve Sets", ":/EnsembleCurveSets16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_curveSets, "EnsembleCurveSets", "Ensemble Curve Sets" );
    m_curveSets.uiCapability()->setUiTreeChildrenHidden( false );
    caf::PdmFieldReorderCapability::addToFieldWithCallback( &m_curveSets, this, &RimEnsembleCurveSetCollection::onCurveSetsReordered );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves" );
    m_showCurves.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ySourceStepping, "YSourceStepping", "" );
    m_ySourceStepping = new RimSummaryPlotSourceStepping;
    m_ySourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_ySourceStepping.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSetCollection::isCurveSetsVisible() const
{
    return m_showCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::loadDataAndUpdate( bool updateParentPlot )
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        curveSet->loadDataAndUpdate( false );
    }

    if ( updateParentPlot )
    {
        auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
        parentPlot->updateAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::setParentPlotAndReplot( RiuPlotWidget* plot )
{
    setParentPlotNoReplot( plot );

    if ( plot ) plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::setParentPlotNoReplot( RiuPlotWidget* plot )
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        curveSet->setParentPlotNoReplot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::detachPlotCurves()
{
    for ( const auto& curveSet : m_curveSets )
    {
        curveSet->deletePlotCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::reattachPlotCurves()
{
    for ( const auto& curveSet : m_curveSets )
    {
        curveSet->reattachPlotCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimEnsembleCurveSetCollection::findRimCurveFromPlotCurve( const RiuPlotCurve* curve ) const
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        for ( RimSummaryCurve* rimCurve : curveSet->curves() )
        {
            if ( rimCurve->isSameCurve( curve ) )
            {
                return rimCurve;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveSetCollection::findCurveSetFromPlotCurve( const RiuPlotCurve* curve ) const
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        for ( RimSummaryCurve* rimCurve : curveSet->curves() )
        {
            if ( rimCurve->isSameCurve( curve ) )
            {
                return curveSet;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::addCurveSet( RimEnsembleCurveSet* curveSet )
{
    if ( curveSet )
    {
        m_curveSets.push_back( curveSet );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::deleteCurveSet( RimEnsembleCurveSet* curveSet )
{
    deleteCurveSets( { curveSet } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::deleteCurveSets( const std::vector<RimEnsembleCurveSet*> curveSets )
{
    for ( const auto curveSet : curveSets )
    {
        m_curveSets.removeChild( curveSet );
        delete curveSet;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RimEnsembleCurveSetCollection::curveSets() const
{
    return m_curveSets.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEnsembleCurveSetCollection::curveSetCount() const
{
    return m_curveSets.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimEnsembleCurveSetCollection::fieldsToShowInToolbar()
{
    if ( m_ySourceStepping )
    {
        return m_ySourceStepping->fieldsToShowInToolbar();
    }

    return std::vector<caf::PdmFieldHandle*>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::setCurveSetForSourceStepping( RimEnsembleCurveSet* curveSet )
{
    m_curveSetForSourceStepping = curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveSetCollection::curveSetForSourceStepping() const
{
    return m_curveSetForSourceStepping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RimEnsembleCurveSetCollection::curveSetsForSourceStepping() const
{
    std::vector<RimEnsembleCurveSet*> steppingCurveSets;

    if ( m_curveSetForSourceStepping )
    {
        steppingCurveSets.push_back( m_curveSetForSourceStepping );

        {
            // Add corresponding history/summary curve with or without H

            std::string vectorName = m_curveSetForSourceStepping->summaryAddressY().vectorName();

            std::string candidateName;
            if ( m_curveSetForSourceStepping->summaryAddressY().isHistoryVector() )
            {
                candidateName = vectorName.substr( 0, vectorName.size() - 1 );
            }
            else
            {
                candidateName = vectorName + "H";
            }

            for ( const auto& c : curveSets() )
            {
                if ( c->summaryAddressY().vectorName() == candidateName )
                {
                    steppingCurveSets.push_back( c );
                }
            }
        }
    }
    else
    {
        steppingCurveSets = curveSets();
    }

    return steppingCurveSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping* RimEnsembleCurveSetCollection::sourceSteppingObject() const
{
    return m_ySourceStepping();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::deleteAllCurveSets()
{
    m_curveSets.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveSetCollection::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                    std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot ) plot->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::onCurveSetsReordered( const SignalEmitter* emitter )
{
}

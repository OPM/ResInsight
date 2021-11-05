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
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotSourceStepping.h"

#include "RiuQwtPlotCurve.h"

#include "qwt_plot.h"

CAF_PDM_SOURCE_INIT( RimEnsembleCurveSetCollection, "RimEnsembleCurveSetCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSetCollection::RimEnsembleCurveSetCollection()
{
    CAF_PDM_InitObject( "Ensemble Curve Sets", ":/EnsembleCurveSets16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_curveSets, "EnsembleCurveSets", "Ensemble Curve Sets" );
    m_curveSets.uiCapability()->setUiTreeHidden( true );
    m_curveSets.uiCapability()->setUiTreeChildrenHidden( false );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves" );
    m_showCurves.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ySourceStepping, "YSourceStepping", "" );
    m_ySourceStepping = new RimSummaryPlotSourceStepping;
    m_ySourceStepping->setSourceSteppingType( RimSummaryPlotSourceStepping::Y_AXIS );
    m_ySourceStepping.uiCapability()->setUiTreeHidden( true );
    m_ySourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_ySourceStepping.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSetCollection::~RimEnsembleCurveSetCollection()
{
    m_curveSets.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSetCollection::isCurveSetsVisible()
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
        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted( parentPlot );
        parentPlot->updateAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::setParentQwtPlotAndReplot( QwtPlot* plot )
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        curveSet->setParentQwtPlotNoReplot( plot );
    }

    if ( plot ) plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::detachQwtCurves()
{
    for ( const auto& curveSet : m_curveSets )
    {
        curveSet->detachQwtCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::reattachQwtCurves()
{
    for ( const auto& curveSet : m_curveSets )
    {
        curveSet->reattachQwtCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimEnsembleCurveSetCollection::findRimCurveFromQwtCurve( const QwtPlotCurve* qwtCurve ) const
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        for ( RimSummaryCurve* rimCurve : curveSet->curves() )
        {
            if ( rimCurve->qwtPlotCurve() == qwtCurve )
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
RimEnsembleCurveSet* RimEnsembleCurveSetCollection::findRimCurveSetFromQwtCurve( const QwtPlotCurve* qwtCurve ) const
{
    for ( RimEnsembleCurveSet* curveSet : m_curveSets )
    {
        for ( RimSummaryCurve* rimCurve : curveSet->curves() )
        {
            if ( rimCurve->qwtPlotCurve() == qwtCurve )
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
        m_curveSets.removeChildObject( curveSet );
        delete curveSet;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RimEnsembleCurveSetCollection::curveSets() const
{
    return m_curveSets.childObjects();
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

            std::string quantity = m_curveSetForSourceStepping->summaryAddress().quantityName();

            std::string candidateName;
            if ( m_curveSetForSourceStepping->summaryAddress().isHistoryQuantity() )
            {
                candidateName = quantity.substr( 0, quantity.size() - 1 );
            }
            else
            {
                candidateName = quantity + "H";
            }

            for ( const auto& c : curveSets() )
            {
                if ( c->summaryAddress().quantityName() == candidateName )
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
    m_curveSets.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );

        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( summaryPlot );
        summaryPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( "Data Source" );

    m_ySourceStepping()->uiOrdering( uiConfigName, *group );
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
    RimSummaryPlot* plot = nullptr;
    this->firstAncestorOrThisOfType( plot );
    if ( plot ) plot->updateConnectedEditors();
}

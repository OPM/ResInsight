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

#include "RimSummaryCurveCollection.h"

#include "RiaStdStringTools.h"

#include "SummaryPlotCommands/RicEditSummaryPlotFeature.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotSourceStepping.h"

#include "RiuQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeViewEditor.h"

#include "qwt_plot.h"

#include <QKeyEvent>

CAF_PDM_SOURCE_INIT( RimSummaryCurveCollection, "RimSummaryCurveCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection::RimSummaryCurveCollection()
    : curvesChanged( this )
{
    CAF_PDM_InitObject( "Summary Curves", ":/SummaryCurveFilter16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_curves, "CollectionCurves", "Curves" );
    caf::PdmFieldReorderCapability::addToFieldWithCallback( &m_curves, this, &RimSummaryCurveCollection::onCurvesReordered );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves" );
    m_showCurves.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_editPlot, "EditPlot", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_editPlot );

    CAF_PDM_InitFieldNoDefault( &m_ySourceStepping, "YSourceStepping", "" );
    m_ySourceStepping = new RimSummaryPlotSourceStepping;
    m_ySourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_ySourceStepping.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurveCollection::isCurvesVisible()
{
    return m_showCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::loadDataAndUpdate( bool updateParentPlot )
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->loadDataAndUpdate( false );
        curve->updatePlotAxis();
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
void RimSummaryCurveCollection::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_curves )
    {
        for ( auto obj : updatedObjects )
        {
            // Must use firstAncestorOrThisOfType, since we need to update the curve appearance if the object is a child of a
            // RimSummaryCurve. This is the case when modifying curve appearance.
            if ( auto curve = obj->firstAncestorOrThisOfType<RimSummaryCurve>() )
            {
                curve->updateCurveAppearance();
            }
        }

        auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
        parentPlot->plotWidget()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::setParentPlotAndReplot( RiuPlotWidget* plot )
{
    setParentPlotNoReplot( plot );

    if ( plot ) plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::setParentPlotNoReplot( RiuPlotWidget* plot )
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->setParentPlotNoReplot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::detachPlotCurves()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::reattachPlotCurves()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        if ( curve->isChecked() ) curve->reattach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryCurveCollection::findRimCurveFromPlotCurve( const RiuPlotCurve* curve ) const
{
    for ( RimSummaryCurve* rimCurve : m_curves )
    {
        if ( rimCurve->isSameCurve( curve ) )
        {
            return rimCurve;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::addCurve( RimSummaryCurve* curve )
{
    if ( curve )
    {
        m_curves.push_back( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::insertCurve( RimSummaryCurve* curve, size_t index )
{
    if ( index >= m_curves.size() )
    {
        m_curves.push_back( curve );
    }
    else
    {
        m_curves.insert( index, curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::deleteCurve( RimSummaryCurve* curve )
{
    removeCurve( curve );
    if ( curve )
    {
        delete curve;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::removeCurve( RimSummaryCurve* curve )
{
    if ( curve )
    {
        m_curves.removeChild( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimSummaryCurveCollection::curves() const
{
    return m_curves.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::deleteCurvesAssosiatedWithCase( RimSummaryCase* summaryCase )
{
    std::vector<RimSummaryCurve*> summaryCurvesToDelete;

    for ( RimSummaryCurve* summaryCurve : m_curves )
    {
        if ( !summaryCurve ) continue;
        if ( !summaryCurve->summaryCaseY() ) continue;

        if ( summaryCurve->summaryCaseY() == summaryCase )
        {
            summaryCurvesToDelete.push_back( summaryCurve );
        }
    }
    for ( RimSummaryCurve* summaryCurve : summaryCurvesToDelete )
    {
        m_curves.removeChild( summaryCurve );
        delete summaryCurve;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::deleteAllCurves()
{
    m_curves.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::updateCaseNameHasChanged()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->updateCurveNameNoLegendUpdate();
        curve->updateConnectedEditors();
    }

    auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();

    parentPlot->updatePlotTitle();
    if ( parentPlot->plotWidget() ) parentPlot->plotWidget()->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::setCurrentSummaryCurve( RimSummaryCurve* curve )
{
    m_currentSummaryCurve = curve;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryCurveCollection::fieldsToShowInToolbar()
{
    return m_ySourceStepping()->fieldsToShowInToolbar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::setCurveAsTopZWithinCategory( RimSummaryCurve* curve )
{
    for ( const auto& c : m_curves )
    {
        if ( c == curve )
        {
            c->setAsTopZWithinCategory( true );
        }
        else
        {
            c->setAsTopZWithinCategory( false );
        }

        c->setZIndexFromCurveInfo();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::setCurveForSourceStepping( RimSummaryCurve* curve )
{
    m_curveForSourceStepping = curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryCurveCollection::curveForSourceStepping() const
{
    return m_curveForSourceStepping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotSourceStepping* RimSummaryCurveCollection::sourceSteppingObject() const
{
    return m_ySourceStepping();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::HorizontalAxisType> RimSummaryCurveCollection::horizontalAxisTypes() const
{
    std::set<RiaDefines::HorizontalAxisType> axisTypes;

    for ( const auto& curve : m_curves )
    {
        axisTypes.insert( curve->axisTypeX() );
    }

    return axisTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_editPlot )
    {
        auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
        if ( plot )
        {
            RicEditSummaryPlotFeature::editSummaryPlot( plot );
        }
        m_editPlot = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_editPlot == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Edit Plot";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::onCurvesReordered( const SignalEmitter* emitter )
{
    updateCurveOrder();
    curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCurveCollection::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTreeViewEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>( attribute );
    if ( myAttr && m_currentSummaryCurve.notNull() )
    {
        myAttr->currentObject = m_currentSummaryCurve.p();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::updateCurveOrder()
{
    detachPlotCurves();
    reattachPlotCurves();
}

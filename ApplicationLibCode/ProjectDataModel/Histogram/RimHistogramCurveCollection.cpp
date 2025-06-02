/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimHistogramCurveCollection.h"

#include "RimHistogramCurve.h"
#include "RimHistogramPlot.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeViewEditor.h"

#include <QKeyEvent>

CAF_PDM_SOURCE_INIT( RimHistogramCurveCollection, "RimHistogramCurveCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCurveCollection::RimHistogramCurveCollection()
    : curvesChanged( this )
{
    CAF_PDM_InitObject( "Histogram Curves", ":/HistogramCurveFilter16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_curves, "CollectionCurves", "Curves" );
    caf::PdmFieldReorderCapability::addToFieldWithCallback( &m_curves, this, &RimHistogramCurveCollection::onCurvesReordered );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves" );
    m_showCurves.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramCurveCollection::isCurvesVisible()
{
    return m_showCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::loadDataAndUpdate( bool updateParentPlot )
{
    for ( RimHistogramCurve* curve : m_curves )
    {
        curve->loadDataAndUpdate( false );
        curve->updatePlotAxis();
    }

    if ( updateParentPlot )
    {
        auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
        parentPlot->updateAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::onChildrenUpdated( caf::PdmChildArrayFieldHandle*      childArray,
                                                     std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_curves )
    {
        for ( auto obj : updatedObjects )
        {
            // Must use firstAncestorOrThisOfType, since we need to update the curve appearance if the object is a child of a
            // RimHistogramCurve. This is the case when modifying curve appearance.
            if ( auto curve = obj->firstAncestorOrThisOfType<RimHistogramCurve>() )
            {
                curve->updateCurveAppearance();
            }
        }

        auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
        parentPlot->plotWidget()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::setParentPlotAndReplot( RiuPlotWidget* plot )
{
    setParentPlotNoReplot( plot );

    if ( plot ) plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::setParentPlotNoReplot( RiuPlotWidget* plot )
{
    for ( RimHistogramCurve* curve : m_curves )
    {
        curve->setParentPlotNoReplot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::detachPlotCurves()
{
    for ( RimHistogramCurve* curve : m_curves )
    {
        curve->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::reattachPlotCurves()
{
    for ( RimHistogramCurve* curve : m_curves )
    {
        if ( curve->isChecked() ) curve->reattach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::addCurve( RimHistogramCurve* curve )
{
    if ( curve )
    {
        m_curves.push_back( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::insertCurve( RimHistogramCurve* curve, size_t index )
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
void RimHistogramCurveCollection::deleteCurve( RimHistogramCurve* curve )
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
void RimHistogramCurveCollection::removeCurve( RimHistogramCurve* curve )
{
    if ( curve )
    {
        m_curves.removeChild( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramCurve*> RimHistogramCurveCollection::curves() const
{
    return m_curves.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::deleteAllCurves()
{
    m_curves.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::updateCaseNameHasChanged()
{
    for ( RimHistogramCurve* curve : m_curves )
    {
        curve->updateCurveNameNoLegendUpdate();
        curve->updateConnectedEditors();
    }

    auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    parentPlot->updatePlotTitle();
    if ( parentPlot->plotWidget() ) parentPlot->plotWidget()->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::setCurrentHistogramCurve( RimHistogramCurve* curve )
{
    m_currentHistogramCurve = curve;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimHistogramCurveCollection::fieldsToShowInToolbar()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::onCurvesReordered( const SignalEmitter* emitter )
{
    updateCurveOrder();
    curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                  std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimHistogramCurveCollection::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTreeViewEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>( attribute );
    if ( myAttr && m_currentHistogramCurve.notNull() )
    {
        myAttr->currentObject = m_currentHistogramCurve.p();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurveCollection::updateCurveOrder()
{
    detachPlotCurves();
    reattachPlotCurves();
}

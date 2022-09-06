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
#include "RimSummaryCrossPlot.h"
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

    CAF_PDM_InitFieldNoDefault( &m_curves, "CollectionCurves", "Collection Curves" );
    m_curves.uiCapability()->setUiTreeHidden( true );
    m_curves.uiCapability()->setUiTreeChildrenHidden( false );
    caf::PdmFieldReorderCapability::addToFieldWithCallback( &m_curves, this, &RimSummaryCurveCollection::onCurvesReordered );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves" );
    m_showCurves.uiCapability()->setUiHidden( true );
    m_showCurves.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_editPlot, "EditPlot", false, "" );
    m_editPlot.xmlCapability()->disableIO();
    m_editPlot.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_ySourceStepping, "YSourceStepping", "" );
    m_ySourceStepping = new RimSummaryPlotSourceStepping;
    m_ySourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::Y_AXIS );
    m_ySourceStepping.uiCapability()->setUiTreeHidden( true );
    m_ySourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_ySourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_xSourceStepping, "XSourceStepping", "" );
    m_xSourceStepping = new RimSummaryPlotSourceStepping;
    m_xSourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::X_AXIS );
    m_xSourceStepping.uiCapability()->setUiTreeHidden( true );
    m_xSourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_xSourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_unionSourceStepping, "UnionSourceStepping", "" );
    m_unionSourceStepping = new RimSummaryPlotSourceStepping;
    m_unionSourceStepping->setSourceSteppingType( RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS );
    m_unionSourceStepping.uiCapability()->setUiTreeHidden( true );
    m_unionSourceStepping.uiCapability()->setUiTreeChildrenHidden( true );
    m_unionSourceStepping.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection::~RimSummaryCurveCollection()
{
    m_curves.deleteChildren();
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
        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted( parentPlot );
        parentPlot->updateAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::onChildrenUpdated( caf::PdmChildArrayFieldHandle*      childArray,
                                                   std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    if ( childArray == &m_curves )
    {
        for ( RimSummaryCurve* curve : m_curves )
        {
            curve->updateCurveAppearance();
        }

        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted( parentPlot );
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
        if ( curve->isCurveVisible() ) curve->reattach();
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
    return m_curves.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*>
    RimSummaryCurveCollection::curvesForSourceStepping( RimSummaryDataSourceStepping::Axis steppingType ) const
{
    std::vector<RimSummaryCurve*> stepCurves;

    if ( m_curveForSourceStepping )
    {
        stepCurves.push_back( m_curveForSourceStepping );

        {
            // Add corresponding history/summary curve with or without H

            const std::string historyIdentifier = "H";

            std::string vectorName;

            if ( steppingType == RimSummaryDataSourceStepping::Axis::X_AXIS )
            {
                vectorName = m_curveForSourceStepping->summaryAddressX().vectorName();
            }
            else if ( steppingType == RimSummaryDataSourceStepping::Axis::Y_AXIS )
            {
                vectorName = m_curveForSourceStepping->summaryAddressY().vectorName();
            }

            std::string candidateName;
            if ( RiaStdStringTools::endsWith( vectorName, historyIdentifier ) )
            {
                candidateName = vectorName.substr( 0, vectorName.size() - 1 );
            }
            else
            {
                candidateName = vectorName + historyIdentifier;
            }

            for ( const auto& c : curves() )
            {
                if ( steppingType == RimSummaryDataSourceStepping::Axis::X_AXIS )
                {
                    if ( c->summaryCaseX() == m_curveForSourceStepping->summaryCaseX() &&
                         c->summaryAddressX().vectorName() == candidateName )
                    {
                        stepCurves.push_back( c );
                    }
                }
                else if ( steppingType == RimSummaryDataSourceStepping::Axis::Y_AXIS )
                {
                    if ( c->summaryCaseY() == m_curveForSourceStepping->summaryCaseY() &&
                         c->summaryAddressY().vectorName() == candidateName )
                    {
                        stepCurves.push_back( c );
                    }
                }
            }
        }
    }
    else
    {
        stepCurves = curves();
    }

    return stepCurves;
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

    RimSummaryPlot* parentPlot;
    firstAncestorOrThisOfTypeAsserted( parentPlot );

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
    RimSummaryCrossPlot* parentCrossPlot;
    firstAncestorOrThisOfType( parentCrossPlot );

    if ( parentCrossPlot )
    {
        return m_unionSourceStepping->fieldsToShowInToolbar();
    }

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
RimSummaryPlotSourceStepping*
    RimSummaryCurveCollection::sourceSteppingObject( RimSummaryDataSourceStepping::Axis sourceSteppingType ) const
{
    if ( sourceSteppingType == RimSummaryDataSourceStepping::Axis::X_AXIS )
    {
        return m_xSourceStepping();
    }
    else if ( sourceSteppingType == RimSummaryDataSourceStepping::Axis::Y_AXIS )
    {
        return m_ySourceStepping();
    }
    if ( sourceSteppingType == RimSummaryDataSourceStepping::Axis::UNION_X_Y_AXIS )
    {
        return m_unionSourceStepping();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );
    }
    else if ( changedField == &m_editPlot )
    {
        RimSummaryPlot* plot = nullptr;
        this->firstAncestorOrThisOfType( plot );
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
void RimSummaryCurveCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute )
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
void RimSummaryCurveCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                std::vector<caf::PdmObjectHandle*>& referringObjects )
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

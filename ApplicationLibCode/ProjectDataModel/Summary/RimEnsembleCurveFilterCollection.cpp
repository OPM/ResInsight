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

#include "RimEnsembleCurveFilterCollection.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveSet.h"
#include "RimSummaryPlot.h"

#include "RiuTextContentFrame.h"

#include <cafPdmUiPushButtonEditor.h>
#include <cafPdmUiTreeOrdering.h>

CAF_PDM_SOURCE_INIT( RimEnsembleCurveFilterCollection, "RimEnsembleCurveFilterCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection::RimEnsembleCurveFilterCollection()
{
    CAF_PDM_InitObject( "Curve Filters", ":/FilterCollection.svg" );

    CAF_PDM_InitField( &m_active, "Active", true, "Active" );

    CAF_PDM_InitFieldNoDefault( &m_filters, "CurveFilters", "" );
    m_filters.uiCapability()->setUiTreeChildrenHidden( true );
    m_filters.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_newFilterButton, "NewEnsembleFilter", "New Filter" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_newFilterButton );
    m_newFilterButton = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilter* RimEnsembleCurveFilterCollection::addFilter( const QString& ensembleParameterName )
{
    auto newFilter = new RimEnsembleCurveFilter( ensembleParameterName );
    m_filters.push_back( newFilter );
    return newFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveFilter*> RimEnsembleCurveFilterCollection::filters() const
{
    return m_filters.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveFilterCollection::isActive() const
{
    return m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEnsembleCurveFilterCollection::countActiveFilters() const
{
    int activeFilters = 0;
    for ( auto& filter : m_filters )
    {
        if ( filter->isActive() )
        {
            activeFilters++;
        }
    }
    return activeFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveFilterCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimEnsembleCurveSet* curveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
    if ( !curveSet ) return;

    if ( changedField == &m_active )
    {
        curveSet->updateAllCurves();
    }
    else if ( changedField == &m_newFilterButton )
    {
        m_newFilterButton = false;

        addFilter();
        updateConnectedEditors();
        curveSet->updateAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Filters" );

    group->add( &m_newFilterButton );

    for ( auto& filter : m_filters )
    {
        QString groupTitle;
        auto    selEnsembleParam = filter->selectedEnsembleParameter();
        if ( selEnsembleParam.isNumeric() )
        {
            groupTitle = filter->ensembleParameterName();

            if ( !filter->isActive() )
            {
                groupTitle += " - [Disabled]";
            }
            else
            {
                groupTitle +=
                    QString( " [%2 .. %3]" ).arg( QString::number( filter->minValue() ) ).arg( QString::number( filter->maxValue() ) );
            }
        }
        else if ( selEnsembleParam.isText() )
        {
            groupTitle = filter->ensembleParameterName();

            if ( !filter->isActive() )
            {
                groupTitle += " - [Disabled]";
            }
            else
            {
                groupTitle += " { ";

                bool first = true;
                for ( const auto& cat : filter->categories() )
                {
                    if ( !first ) groupTitle += ", ";
                    groupTitle += cat;
                    first = false;
                }
                groupTitle += " }";

                if ( groupTitle.size() > 45 )
                {
                    groupTitle = groupTitle.left( 40 ) + "... }";
                }
            }
        }

        caf::PdmUiGroup* filterGroup = group->addNewGroupWithKeyword( groupTitle, QString( "EnsembleFilter_" ) + filter->filterId() );
        filter->defineUiOrdering( uiConfigName, *filterGroup );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /* = "" */ )
{
    for ( auto filter : filters() )
    {
        uiTreeOrdering.add( filter );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                              QString                    uiConfigName,
                                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_newFilterButton )
    {
        caf::PdmUiPushButtonEditorAttribute* attr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( !attr ) return;

        attr->m_buttonText = "Add Ensemble Curve Filter";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::loadDataAndUpdate()
{
    for ( auto& filter : m_filters )
        filter->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveFilterCollection::filterDescriptions() const
{
    QStringList descriptions;
    for ( auto filter : m_filters )
    {
        if ( filter->isActive() )
        {
            descriptions << filter->description();
        }
    }
    return descriptions.join( "\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTextContentFrame* RimEnsembleCurveFilterCollection::makeFilterDescriptionFrame() const
{
    QString descriptions = filterDescriptions();
    descriptions.replace( "+", "\n+" );

    // A size of -1 use default plot font
    int fontSize = -1;

    auto plotWindow = firstAncestorOrThisOfType<RimPlotWindow>();
    if ( plotWindow )
    {
        const double scalingFactor = 1.4;
        fontSize                   = scalingFactor * plotWindow->fontSize();
    }

    return new RiuTextContentFrame( nullptr, QString( "Active curve filters:" ), descriptions, fontSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveFilterCollection::objectToggleField()
{
    return &m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveFilterCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                       std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimSummaryPlot* plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot ) plot->loadDataAndUpdate();

    RimEnsembleCurveSet* curveSet = firstAncestorOrThisOfType<RimEnsembleCurveSet>();
    if ( curveSet ) curveSet->updateConnectedEditors();
}

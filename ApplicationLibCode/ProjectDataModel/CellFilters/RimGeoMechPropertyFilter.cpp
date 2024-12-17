/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechPropertyFilter.h"

#include "RigFemPartResultsCollection.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimViewController.h"

#include "RiuMainWindow.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT( RimGeoMechPropertyFilter, "GeoMechPropertyFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter::RimGeoMechPropertyFilter()
    : m_parentContainer( nullptr )
{
    CAF_PDM_InitObject( "Property Filter", ":/CellFilter_Values.png" );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "Result Definition" );
    m_resultDefinition = new RimGeoMechResultDefinition();

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_linkedWithCellResult,
                                "LinkedWithCellResult",
                                "Linked With Cell Result",
                                "",
                                "The selected cell result is automatically used to update the property filter." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_linkedWithCellResult );

    CAF_PDM_InitField( &m_lowerBound, "LowerBound", 0.0, "Min" );
    m_lowerBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_upperBound, "UpperBound", 0.0, "Max" );
    m_upperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    updateIconState();

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter::~RimGeoMechPropertyFilter()
{
    delete m_resultDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechResultDefinition* RimGeoMechPropertyFilter::resultDefinition() const
{
    return m_resultDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilter::isLinkedWithCellResult() const
{
    return m_linkedWithCellResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::setLinkedWithCellResult( bool linkedWithCellResult )
{
    m_linkedWithCellResult = linkedWithCellResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechPropertyFilter::lowerBound() const
{
    return m_lowerBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechPropertyFilter::upperBound() const
{
    return m_upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_lowerBound == changedField || &m_upperBound == changedField || &m_isActive == changedField || &m_filterMode == changedField ||
         &m_selectedCategoryValues == changedField || &m_linkedWithCellResult == changedField )
    {
        updateIconState();
        updateFilterName();
        uiCapability()->updateConnectedEditors();

        parentContainer()->updateDisplayModelNotifyManagedViews( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::setParentContainer( RimGeoMechPropertyFilterCollection* parentContainer )
{
    m_parentContainer = parentContainer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection* RimGeoMechPropertyFilter::parentContainer()
{
    return m_parentContainer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::setToDefaultValues()
{
    CVF_ASSERT( m_parentContainer );

    computeResultValueRange();

    m_lowerBound = m_minimumResultValue;
    m_upperBound = m_maximumResultValue;

    m_selectedCategoryValues = m_categoryValues;

    updateFilterName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_linkedWithCellResult );
    if ( m_linkedWithCellResult() )
    {
        uiOrdering.skipRemainingFields( true );
        return;
    }

    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup( "Result" );
    m_resultDefinition->uiOrdering( uiConfigName, *group1 );

    caf::PdmUiGroup& group2 = *( uiOrdering.addNewGroup( "Filter Settings" ) );

    group2.add( &m_filterMode );

    if ( m_resultDefinition->hasCategoryResult() )
    {
        group2.add( &m_selectedCategoryValues );
    }
    else
    {
        group2.add( &m_lowerBound );
        group2.add( &m_upperBound );
    }

    updateReadOnlyStateOfAllFields();

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    PdmObject::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    updateActiveState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::updateReadOnlyStateOfAllFields()
{
    bool readOnlyState = isPropertyFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields = fields();

    // Include fields declared in RimResultDefinition
    objFields.push_back( &( m_resultDefinition->m_resultPositionTypeUiField ) );
    objFields.push_back( &( m_resultDefinition->m_resultVariableUiField ) );
    objFields.push_back( &( m_resultDefinition->m_timeLapseBaseTimestep ) );

    for ( size_t i = 0; i < objFields.size(); i++ )
    {
        objFields[i]->uiCapability()->setUiReadOnly( readOnlyState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilter::isPropertyFilterControlled()
{
    bool isPropertyFilterControlled = false;

    auto rimView = firstAncestorOrThisOfTypeAsserted<Rim3dView>();
    if ( rimView )
    {
        RimViewController* vc = rimView->viewController();
        if ( vc && vc->isPropertyFilterOveridden() )
        {
            isPropertyFilterControlled = true;
        }
    }

    return isPropertyFilterControlled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::updateActiveState()
{
    m_isActive.uiCapability()->setUiReadOnly( isPropertyFilterControlled() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilter::isActiveAndHasResult()
{
    return isActive() && m_resultDefinition->hasResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_minimumResultValue == cvf::UNDEFINED_DOUBLE || m_maximumResultValue == cvf::UNDEFINED_DOUBLE )
    {
        return;
    }

    if ( field == &m_lowerBound || field == &m_upperBound )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum = m_minimumResultValue;
        myAttr->m_maximum = m_maximumResultValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::computeResultValueRange()
{
    CVF_ASSERT( m_parentContainer );

    double min = 0.0;
    double max = 0.0;

    clearCategories();

    RigFemResultAddress resultAddress = m_resultDefinition->resultAddress();
    if ( resultAddress.isValid() && m_resultDefinition->ownerCaseData() )
    {
        if ( m_resultDefinition->hasCategoryResult() )
        {
            std::vector<QString> fnVector = m_resultDefinition->ownerCaseData()->femPartResults()->formationNames();
            setCategoryNames( fnVector );
        }
        else
        {
            m_resultDefinition->ownerCaseData()->femPartResults()->minMaxScalarValues( resultAddress, &min, &max );
        }
    }

    m_maximumResultValue = max;
    m_minimumResultValue = min;

    m_lowerBound.uiCapability()->setUiName( QString( "Min (%1)" ).arg( min ) );
    m_upperBound.uiCapability()->setUiName( QString( "Max (%1)" ).arg( max ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::updateFilterName()
{
    RigFemResultAddress resultAddress = m_resultDefinition->resultAddress();
    QString             newFiltername;

    if ( resultAddress.resultPosType == RIG_FORMATION_NAMES )
    {
        newFiltername = m_resultDefinition->resultFieldName();
    }
    else
    {
        QString posName;

        switch ( resultAddress.resultPosType )
        {
            case RIG_NODAL:
                posName = "N";
                break;
            case RIG_ELEMENT_NODAL:
                posName = "EN";
                break;
            case RIG_INTEGRATION_POINT:
                posName = "IP";
                break;
            case RIG_ELEMENT:
                posName = "E";
                break;
        }

        QString fieldUiName = m_resultDefinition->resultFieldUiName();
        QString compoUiName = m_resultDefinition->resultComponentUiName();

        newFiltername = posName + ", " + fieldUiName + ", " + compoUiName + " (" + QString::number( m_lowerBound() ) + " .. " +
                        QString::number( m_upperBound ) + ")";
    }

    m_name = newFiltername;

    uiCapability()->updateConnectedEditors();
}

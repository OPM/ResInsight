/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipsePropertyFilter.h"

#include "RiaDefines.h"
#include "RiaFieldHandleTools.h"
#include "RiaGuiApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigFormationNames.h"

#include "RimEclipseCase.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimViewController.h"

#include "RiuMainWindow.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeAttributes.h"

#include "cvfAssert.h"

#include <cmath> // Needed for HUGE_VAL on Linux

CAF_PDM_SOURCE_INIT( RimEclipsePropertyFilter, "CellPropertyFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter::RimEclipsePropertyFilter()
{
    CAF_PDM_InitObject( "Cell Property Filter", ":/CellFilter_Values.png" );

    CAF_PDM_InitFieldNoDefault( &m_resultDefinition, "ResultDefinition", "Result Definition" );
    m_resultDefinition = new RimEclipseResultDefinition();
    m_resultDefinition->enableDeltaResults( true );

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_linkedWithCellResult,
                                "LinkedWithCellResult",
                                "Linked With Cell Result",
                                "",
                                "The selected cell result is automatically used to update the property filter." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_linkedWithCellResult );

    CAF_PDM_InitField( &m_rangeLabelText, "Dummy_keyword", QString( "Range Type" ), "Range Type" );
    m_rangeLabelText.xmlCapability()->disableIO();
    m_rangeLabelText.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_lowerBound, "LowerBound", 0.0, "Min" );
    m_lowerBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_upperBound, "UpperBound", 0.0, "Max" );
    m_upperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_integerLowerBound, "IntegerLowerBound", "Min" );
    m_integerLowerBound.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_integerLowerBound.registerGetMethod( this, &RimEclipsePropertyFilter::lowerBound );
    m_integerLowerBound.registerSetMethod( this, &RimEclipsePropertyFilter::setLowerBound );
    m_integerLowerBound.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_integerUpperBound, "IntegerUpperBound", "Max" );
    m_integerUpperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );
    m_integerUpperBound.registerGetMethod( this, &RimEclipsePropertyFilter::upperBound );
    m_integerUpperBound.registerSetMethod( this, &RimEclipsePropertyFilter::setUpperBound );
    m_integerUpperBound.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_useCategorySelection, "CategorySelection", false, "Category Selection" );
    m_upperBound.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_isDuplicatedFromLinkedView, "IsDuplicatedFromLinkedView", false, "Duplicated" );
    m_isDuplicatedFromLinkedView.uiCapability()->setUiHidden( true );

    // HEADLESS HACK
    if ( RiaGuiApplication::isRunning() )
    {
        updateIconState();
    }

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter::~RimEclipsePropertyFilter()
{
    delete m_resultDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition* RimEclipsePropertyFilter::resultDefinition() const
{
    return m_resultDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilter::isLinkedWithCellResult() const
{
    return m_linkedWithCellResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setLinkedWithCellResult( bool linkedWithCellResult )
{
    m_linkedWithCellResult = linkedWithCellResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::rangeValues( double* lower, double* upper ) const
{
    *lower = m_lowerBound;
    *upper = m_upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilter::isCategorySelectionActive() const
{
    return m_resultDefinition->hasCategoryResult() && m_useCategorySelection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setIsDuplicatedFromLinkedView( bool isDuplicated )
{
    m_isDuplicatedFromLinkedView = isDuplicated;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_lowerBound == changedField || &m_integerLowerBound == changedField )
    {
        if ( m_lowerBound > m_upperBound ) m_upperBound = m_lowerBound;
    }

    if ( &m_upperBound == changedField || &m_integerUpperBound == changedField )
    {
        if ( m_upperBound < m_lowerBound ) m_lowerBound = m_upperBound;
    }

    if ( &m_lowerBound == changedField || &m_upperBound == changedField || &m_isActive == changedField || &m_filterMode == changedField ||
         &m_selectedCategoryValues == changedField || &m_useCategorySelection == changedField || &m_integerUpperBound == changedField ||
         &m_integerLowerBound == changedField || &m_linkedWithCellResult == changedField )
    {
        m_isDuplicatedFromLinkedView = false;

        m_resultDefinition->loadResult();
        computeResultValueRange();
        updateFilterName();
        updateIconState();
        uiCapability()->updateConnectedEditors();

        parentContainer()->updateDisplayModelNotifyManagedViews( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection* RimEclipsePropertyFilter::parentContainer()
{
    return firstAncestorOrThisOfTypeAsserted<RimEclipsePropertyFilterCollection>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setToDefaultValues()
{
    CVF_ASSERT( parentContainer() );

    computeResultValueRange();

    m_lowerBound.setValueWithFieldChanged( m_minimumResultValue );
    m_upperBound.setValueWithFieldChanged( m_maximumResultValue );

    m_selectedCategoryValues = m_categoryValues;
    m_useCategorySelection   = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Fields declared in RimCellFilter
    uiOrdering.add( &m_name );

    uiOrdering.add( &m_linkedWithCellResult );
    if ( m_linkedWithCellResult )
    {
        uiOrdering.skipRemainingFields( true );
        return;
    }

    // Fields declared in Rimm_resultDefinition
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup( "Result" );
    m_resultDefinition->uiOrdering( uiConfigName, *group1 );

    caf::PdmUiGroup& group2 = *( uiOrdering.addNewGroup( "Filter Settings" ) );

    // Fields declared in RimCellFilter
    group2.add( &m_filterMode );

    group2.add( &m_rangeLabelText );

    if ( m_resultDefinition->hasCategoryResult() )
    {
        group2.add( &m_useCategorySelection );
    }

    if ( m_resultDefinition->hasCategoryResult() && m_useCategorySelection() )
    {
        group2.add( &m_selectedCategoryValues );
    }
    else
    {
        if ( m_resultDefinition->hasCategoryResult() )
        {
            group2.add( &m_integerLowerBound );
            group2.add( &m_integerUpperBound );
        }
        else
        {
            group2.add( &m_lowerBound );
            group2.add( &m_upperBound );
        }
    }

    uiOrdering.skipRemainingFields( true );

    updateReadOnlyStateOfAllFields();
    updateRangeLabel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<caf::PdmFieldHandle*>> RimEclipsePropertyFilter::quickAccessFields()
{
    std::map<QString, std::vector<caf::PdmFieldHandle*>> fields;

    auto name = "Property Filter : " + m_resultDefinition->resultVariableUiName();

    if ( m_resultDefinition->hasCategoryResult() && m_useCategorySelection() )
    {
        fields[name].push_back( &m_selectedCategoryValues );
    }
    else
    {
        fields[name].push_back( &m_lowerBound );
        fields[name].push_back( &m_upperBound );
    }

    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    PdmObject::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    updateActiveState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateReadOnlyStateOfAllFields()
{
    bool readOnlyState = isPropertyFilterControlled();

    std::vector<caf::PdmFieldHandle*> objFields = fields();

    // Include fields declared in Rimm_resultDefinition
    objFields.push_back( &( m_resultDefinition->m_resultTypeUiField ) );
    objFields.push_back( &( m_resultDefinition->m_porosityModelUiField ) );
    objFields.push_back( &( m_resultDefinition->m_resultVariableUiField ) );

    for ( auto f : objFields )
    {
        if ( f == &m_rangeLabelText ) continue;

        f->uiCapability()->setUiReadOnly( readOnlyState );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateRangeLabel()
{
    if ( m_resultDefinition->isFlowDiagOrInjectionFlooding() )
    {
        m_rangeLabelText = "Current Timestep";
    }
    else
    {
        m_rangeLabelText = "All Timesteps";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilter::isPropertyFilterControlled()
{
    auto rimView = firstAncestorOrThisOfTypeAsserted<Rim3dView>();

    bool isPropertyFilterControlled = false;

    RimViewController* vc = rimView->viewController();
    if ( vc && vc->isPropertyFilterOveridden() )
    {
        isPropertyFilterControlled = true;
    }

    return isPropertyFilterControlled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setCategoriesFromTracerNames( const std::vector<QString>& tracerNames )
{
    std::vector<std::pair<QString, int>> tracerNameValuesSorted;

    {
        std::set<std::pair<QString, int>> tracerNameSet;

        for ( size_t i = 0; i < tracerNames.size(); i++ )
        {
            tracerNameSet.insert( std::make_pair( tracerNames[i], static_cast<int>( i ) ) );
        }

        for ( const auto& it : tracerNameSet )
        {
            tracerNameValuesSorted.push_back( it );
        }
    }

    setCategoryNamesAndValues( tracerNameValuesSorted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateActiveState()
{
    m_isActive.uiCapability()->setUiReadOnly( isPropertyFilterControlled() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_minimumResultValue == cvf::UNDEFINED_DOUBLE || m_maximumResultValue == cvf::UNDEFINED_DOUBLE )
    {
        return;
    }

    if ( field == &m_lowerBound || field == &m_upperBound )
    {
        if ( auto doubleAttributes = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            doubleAttributes->m_minimum = m_minimumResultValue;
            doubleAttributes->m_maximum = m_maximumResultValue;
        }
    }

    if ( field == &m_integerLowerBound || field == &m_integerUpperBound )
    {
        if ( auto integerAttributes = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute ) )
        {
            integerAttributes->m_minimum = m_minimumResultValue;
            integerAttributes->m_maximum = m_maximumResultValue;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( !m_isDuplicatedFromLinkedView ) return;

    auto rimView = firstAncestorOrThisOfTypeAsserted<Rim3dView>();

    RimViewController* vc = rimView->viewController();
    if ( vc && vc->isPropertyFilterDuplicationActive() )
    {
        auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute );
        if ( treeItemAttribute )
        {
            treeItemAttribute->tags.clear();
            auto tag  = caf::PdmUiTreeViewItemAttribute::createTag();
            tag->icon = caf::IconProvider( ":/chain.png" );

            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEclipsePropertyFilter::upperBound() const
{
    return m_upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setUpperBound( const int& upperBound )
{
    m_upperBound = upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEclipsePropertyFilter::lowerBound() const
{
    return m_lowerBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::setLowerBound( const int& lowerBound )
{
    m_lowerBound = lowerBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::computeResultValueRange()
{
    CVF_ASSERT( parentContainer() );

    double min = HUGE_VAL;
    double max = -HUGE_VAL;

    clearCategories();

    if ( m_resultDefinition->isFlowDiagOrInjectionFlooding() )
    {
        auto view = firstAncestorOrThisOfType<Rim3dView>();

        int timeStep = 0;
        if ( view ) timeStep = view->currentTimeStep();
        RigFlowDiagResultAddress resAddr = m_resultDefinition->flowDiagResAddress();
        if ( m_resultDefinition->flowDiagSolution() )
        {
            RigFlowDiagResults* results = m_resultDefinition->flowDiagSolution()->flowDiagResults();
            results->minMaxScalarValues( resAddr, timeStep, &min, &max );

            if ( m_resultDefinition->hasCategoryResult() )
            {
                setCategoriesFromTracerNames( m_resultDefinition->flowDiagSolution()->tracerNames() );
            }
        }
    }
    else
    {
        RigEclipseResultAddress scalarIndex = m_resultDefinition->eclipseResultAddress();
        if ( scalarIndex.isValid() )
        {
            RigCaseCellResultsData* results = m_resultDefinition->currentGridCellResults();
            if ( results )
            {
                if ( results->hasResultEntry( scalarIndex ) )
                {
                    results->minMaxCellScalarValues( scalarIndex, min, max );
                }

                if ( m_resultDefinition->hasCategoryResult() )
                {
                    if ( m_resultDefinition->resultType() == RiaDefines::ResultCatType::FORMATION_NAMES )
                    {
                        CVF_ASSERT( parentContainer()->reservoirView()->eclipseCase()->eclipseCaseData() );

                        const std::vector<QString> fnVector =
                            parentContainer()->reservoirView()->eclipseCase()->eclipseCaseData()->formationNames();

                        setCategoryNames( fnVector );
                    }
                    else if ( m_resultDefinition->resultVariable() == RiaResultNames::completionTypeResultName() )
                    {
                        std::vector<RiaDefines::WellPathComponentType> componentTypes = { RiaDefines::WellPathComponentType::WELL_PATH,
                                                                                          RiaDefines::WellPathComponentType::PERFORATION_INTERVAL,
                                                                                          RiaDefines::WellPathComponentType::FISHBONES,
                                                                                          RiaDefines::WellPathComponentType::FRACTURE };
                        std::vector<std::pair<QString, int>>           ctNamesAndValues;
                        for ( RiaDefines::WellPathComponentType type : componentTypes )
                        {
                            ctNamesAndValues.push_back( std::make_pair( caf::AppEnum<RiaDefines::WellPathComponentType>::uiText( type ),
                                                                        static_cast<int>( type ) ) );
                        }
                        setCategoryNamesAndValues( ctNamesAndValues );
                    }
                    else if ( results->hasResultEntry( scalarIndex ) )
                    {
                        setCategoryValues( results->uniqueCellScalarValues( scalarIndex ) );
                    }
                }
            }
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
void RimEclipsePropertyFilter::updateFromCurrentTimeStep()
{
    // Update range for flow diagnostics values when time step changes
    // Range for flow is always current time step, not computed across all time steps
    // If upper/lower slider is set to available extrema, the filter values will be
    // updated with the min/max values for the current time step
    //
    // If the user manually has set a filter value, this value is left untouched

    if ( !m_resultDefinition->isFlowDiagOrInjectionFlooding() )
    {
        return;
    }

    double threshold = 1e-6;
    bool   followMin = false;
    if ( fabs( m_lowerBound - m_minimumResultValue ) < threshold || m_minimumResultValue == HUGE_VAL )
    {
        followMin = true;
    }

    bool followMax = false;
    if ( fabs( m_upperBound - m_maximumResultValue ) < threshold || m_maximumResultValue == -HUGE_VAL )
    {
        followMax = true;
    }

    double min = HUGE_VAL;
    double max = -HUGE_VAL;

    clearCategories();

    auto view = firstAncestorOrThisOfTypeAsserted<Rim3dView>();

    int                      timeStep = view->currentTimeStep();
    RigFlowDiagResultAddress resAddr  = m_resultDefinition->flowDiagResAddress();
    if ( m_resultDefinition->flowDiagSolution() )
    {
        RigFlowDiagResults* results = m_resultDefinition->flowDiagSolution()->flowDiagResults();
        results->minMaxScalarValues( resAddr, timeStep, &min, &max );

        if ( m_resultDefinition->hasCategoryResult() )
        {
            setCategoriesFromTracerNames( m_resultDefinition->flowDiagSolution()->tracerNames() );
        }
    }

    if ( min == HUGE_VAL && max == -HUGE_VAL )
    {
        m_lowerBound.uiCapability()->setUiName( QString( "Min (inf)" ) );
        m_upperBound.uiCapability()->setUiName( QString( "Max (inf)" ) );
    }
    else
    {
        m_maximumResultValue = max;
        m_minimumResultValue = min;

        if ( followMin )
        {
            m_lowerBound = min;
        }

        if ( followMax )
        {
            m_upperBound = m_maximumResultValue;
        }

        m_lowerBound.uiCapability()->setUiName( QString( "Min (%1)" ).arg( min ) );
        m_upperBound.uiCapability()->setUiName( QString( "Max (%1)" ).arg( max ) );
    }

    m_lowerBound.uiCapability()->updateConnectedEditors();
    m_upperBound.uiCapability()->updateConnectedEditors();

    updateFilterName();
    m_name.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateFilterName()
{
    QString newFiltername = m_resultDefinition->resultVariableUiShortName();

    if ( isCategorySelectionActive() )
    {
        if ( m_categoryNames.empty() )
        {
            newFiltername += " (";

            if ( !m_selectedCategoryValues().empty() && m_selectedCategoryValues().size() == m_categoryValues.size() )
            {
                newFiltername += QString::number( m_selectedCategoryValues()[0] );
                newFiltername += "..";
                newFiltername += QString::number( m_selectedCategoryValues()[m_selectedCategoryValues().size() - 1] );
            }
            else
            {
                for ( size_t i = 0; i < m_selectedCategoryValues().size(); i++ )
                {
                    int val = m_selectedCategoryValues()[i];
                    newFiltername += QString::number( val );

                    if ( i < m_selectedCategoryValues().size() - 1 )
                    {
                        newFiltername += ", ";
                    }
                }
            }

            newFiltername += ")";
        }
    }
    else
    {
        newFiltername += " (" + QString::number( m_lowerBound ) + " .. " + QString::number( m_upperBound ) + ")";
    }

    m_name = newFiltername;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::initAfterRead()
{
    m_resultDefinition->initAfterRead();

    m_resultDefinition->setEclipseCase( parentContainer()->reservoirView()->eclipseCase() );
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilter::updateUiFieldsFromActiveResult()
{
    m_resultDefinition->updateUiFieldsFromActiveResult();
}

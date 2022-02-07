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

#include "RimIntersectionCollection.h"

#include "RigMainGrid.h"

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "Rim3dView.h"
#include "RimBoxIntersection.h"
#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimSimWellInView.h"

#include "Riu3DMainWindowTools.h"

#include "RivBoxIntersectionPartMgr.h"
#include "RivExtrudedCurveIntersectionPartMgr.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimIntersectionCollection, "IntersectionCollection", "CrossSectionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection::RimIntersectionCollection()
{
    CAF_PDM_InitScriptableObject( "Intersections", ":/CrossSections16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_intersections, "CrossSections", "Intersections" );
    m_intersections.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_intersectionBoxes, "IntersectionBoxes", "IntersectionBoxes" );
    m_intersectionBoxes.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &isActive, "Active", true, "Active" );
    isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_depthUpperThreshold, "UpperDepthThreshold", "Upper Threshold" );
    m_depthUpperThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_depthLowerThreshold, "LowerDepthThreshold", "Lower Threshold" );
    m_depthLowerThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_depthThresholdOverridden, "DepthFilterOverride", false, "Override Intersection Depth Filters" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_depthThresholdOverridden );

    CAF_PDM_InitFieldNoDefault( &m_depthFilterType, "CollectionDepthFilterType", "Depth Filter Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection::~RimIntersectionCollection()
{
    m_intersections.deleteAllChildObjects();
    m_intersectionBoxes.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                      QString                 uiConfigName /*= "" */ )
{
    RimGridView* gridView = nullptr;
    this->firstAncestorOfType( gridView );
    if ( gridView )
    {
        auto uiTree = gridView->separateIntersectionResultsCollection()->uiTreeOrdering();

        uiTreeOrdering.appendChild( uiTree );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::applySingleColorEffect()
{
    if ( !this->isActive() ) return;

    for ( RimExtrudedCurveIntersection* cs : m_intersections )
    {
        if ( cs->isActive() )
        {
            cs->intersectionPartMgr()->applySingleColorEffect();
        }
    }

    for ( RimBoxIntersection* cs : m_intersectionBoxes )
    {
        if ( cs->isActive() )
        {
            cs->intersectionBoxPartMgr()->applySingleColorEffect();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex )
{
    if ( !this->isActive() ) return;

    for ( RimExtrudedCurveIntersection* cs : m_intersections )
    {
        if ( cs->isActive() )
        {
            bool showResults = cs->activeSeparateResultDefinition() ? cs->activeSeparateResultDefinition()->hasResult()
                                                                    : hasGeneralCellResult;

            if ( showResults )
            {
                cs->intersectionPartMgr()->updateCellResultColor( timeStepIndex, nullptr, nullptr );
            }
            else
            {
                cs->intersectionPartMgr()->applySingleColorEffect();
            }
        }
    }

    for ( RimBoxIntersection* cs : m_intersectionBoxes )
    {
        if ( cs->isActive() )
        {
            bool hasSeparateInterResult = cs->activeSeparateResultDefinition() &&
                                          cs->activeSeparateResultDefinition()->hasResult();
            if ( hasSeparateInterResult || hasGeneralCellResult )
            {
                cs->intersectionBoxPartMgr()->updateCellResultColor( timeStepIndex );
            }
            else
            {
                cs->intersectionBoxPartMgr()->applySingleColorEffect();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionCollection::hasAnyActiveSeparateResults()
{
    if ( !this->isActive() ) return false;

    for ( RimExtrudedCurveIntersection* cs : m_intersections )
    {
        if ( cs->isActive() && cs->activeSeparateResultDefinition() && cs->activeSeparateResultDefinition()->hasResult() )
        {
            return true;
        }
    }

    for ( RimBoxIntersection* cs : m_intersectionBoxes )
    {
        if ( cs->isActive() && cs->activeSeparateResultDefinition() && cs->activeSeparateResultDefinition()->hasResult() )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendPartsToModel( Rim3dView& view, cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( !isActive() ) return;

    for ( RimExtrudedCurveIntersection* cs : m_intersections )
    {
        if ( cs->isActive() )
        {
            cs->intersectionPartMgr()->appendIntersectionFacesToModel( model, scaleTransform );
            cs->intersectionPartMgr()->appendMeshLinePartsToModel( model, scaleTransform );
            cs->intersectionPartMgr()->appendPolylinePartsToModel( view, model, scaleTransform );
        }
    }

    for ( RimBoxIntersection* cs : m_intersectionBoxes )
    {
        if ( cs->isActive() )
        {
            cs->intersectionBoxPartMgr()->appendNativeIntersectionFacesToModel( model, scaleTransform );
            cs->intersectionBoxPartMgr()->appendMeshLinePartsToModel( model, scaleTransform );

            if ( cs->show3dManipulator() )
            {
                cs->appendManipulatorPartsToModel( model );
            }
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::rebuildGeometry()
{
    for ( RimExtrudedCurveIntersection* intersection : m_intersections )
    {
        intersection->rebuildGeometry();
    }

    for ( RimBoxIntersection* intersectionBox : m_intersectionBoxes )
    {
        intersectionBox->rebuildGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimExtrudedCurveIntersection*> RimIntersectionCollection::intersections() const
{
    return m_intersections.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimBoxIntersection*> RimIntersectionCollection::intersectionBoxes() const
{
    return m_intersectionBoxes.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::recomputeSimWellBranchData()
{
    for ( const auto& intersection : intersections() )
    {
        intersection->recomputeSimulationWellBranchData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionAndUpdate( RimExtrudedCurveIntersection* intersection,
                                                             bool                          allowActiveViewChange )
{
    m_intersections.push_back( intersection );

    intersection->setDepthOverride( m_depthThresholdOverridden );
    intersection->setDepthOverrideParameters( m_depthUpperThreshold, m_depthLowerThreshold, m_depthFilterType() );

    syncronize2dIntersectionViews();

    updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( intersection, allowActiveViewChange );

    rebuild3dView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionNoUpdate( RimExtrudedCurveIntersection* intersection )
{
    intersection->setDepthOverride( m_depthThresholdOverridden );
    intersection->setDepthOverrideParameters( m_depthUpperThreshold, m_depthLowerThreshold, m_depthFilterType() );
    m_intersections.push_back( intersection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::syncronize2dIntersectionViews()
{
    RimCase* ownerCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( ownerCase );
    ownerCase->intersectionViewCollection()->syncFromExistingIntersections( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::scheduleCreateDisplayModelAndRedraw2dIntersectionViews()
{
    for ( RimExtrudedCurveIntersection* isection : m_intersections )
    {
        if ( isection->correspondingIntersectionView() )
        {
            isection->correspondingIntersectionView()->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionBoxAndUpdate( RimBoxIntersection* intersectionBox )
{
    m_intersectionBoxes.push_back( intersectionBox );

    updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( intersectionBox, false );

    rebuild3dView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionBoxNoUpdate( RimBoxIntersection* intersectionBox )
{
    m_intersectionBoxes.push_back( intersectionBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    bool rebuildView = false;

    if ( changedField == &isActive )
    {
        updateUiIconFromToggleField();
        rebuildView = true;
    }
    if ( changedField == &m_depthThresholdOverridden )
    {
        for ( RimExtrudedCurveIntersection* cs : m_intersections )
        {
            cs->setDepthOverride( m_depthThresholdOverridden );
            if ( m_depthThresholdOverridden() )
            {
                cs->setDepthOverrideParameters( m_depthUpperThreshold, m_depthLowerThreshold, m_depthFilterType() );
            }
            cs->rebuildGeometryAndScheduleCreateDisplayModel();
        }
        rebuildView = true;
    }

    if ( ( changedField == &m_depthUpperThreshold ) || ( changedField == &m_depthLowerThreshold ) ||
         ( changedField == &m_depthFilterType ) )
    {
        for ( RimExtrudedCurveIntersection* cs : m_intersections )
        {
            cs->setDepthOverrideParameters( m_depthUpperThreshold, m_depthLowerThreshold, m_depthFilterType() );
            cs->rebuildGeometryAndScheduleCreateDisplayModel();
        }
        rebuildView = true;
    }

    if ( rebuildView )
    {
        rebuild3dView();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    syncronize2dIntersectionViews();
    rebuild3dView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::onChildAdded( caf::PdmFieldHandle* containerForNewObject )
{
    syncronize2dIntersectionViews();
    rebuild3dView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersectionCollection::hasActiveIntersectionForSimulationWell( const RimSimWellInView* simWell ) const
{
    if ( !isActive() ) return false;

    for ( RimExtrudedCurveIntersection* cs : m_intersections )
    {
        if ( cs->isActive() && cs->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_SIMULATION_WELL &&
             cs->simulationWell() == simWell )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::updateIntersectionBoxGeometry()
{
    for ( RimBoxIntersection* intersectionBox : m_intersectionBoxes )
    {
        intersectionBox->updateBoxManipulatorGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( eclipseView() )
    {
        caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup( "Depth Filter - Curve Intersections" );

        m_depthFilterType.uiCapability()->setUiReadOnly( !m_depthThresholdOverridden() );
        m_depthUpperThreshold.uiCapability()->setUiReadOnly( !m_depthThresholdOverridden() );
        m_depthLowerThreshold.uiCapability()->setUiReadOnly( !m_depthThresholdOverridden() );

        filterGroup->add( &m_depthThresholdOverridden );
        filterGroup->add( &m_depthFilterType );

        switch ( m_depthFilterType() )
        {
            case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
                m_depthUpperThreshold.uiCapability()->setUiName( "Depth" );
                filterGroup->add( &m_depthUpperThreshold );
                break;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
                m_depthUpperThreshold.uiCapability()->setUiName( "Upper Depth" );
                filterGroup->add( &m_depthUpperThreshold );
                m_depthLowerThreshold.uiCapability()->setUiName( "Lower Depth" );
                filterGroup->add( &m_depthLowerThreshold );
                break;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
                m_depthLowerThreshold.uiCapability()->setUiName( "Depth" );
                filterGroup->add( &m_depthLowerThreshold );
                break;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
            default:
                break;
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute )
{
    auto* doubleSliderAttrib = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
    if ( doubleSliderAttrib )
    {
        if ( ( field == &m_depthUpperThreshold ) || ( field == &m_depthLowerThreshold ) )
        {
            RimEclipseView* eclView = eclipseView();

            if ( eclView )
            {
                const cvf::BoundingBox bb = eclView->mainGrid()->boundingBox();

                doubleSliderAttrib->m_minimum = -1.0 * bb.max().z();
                doubleSliderAttrib->m_maximum = -1.0 * bb.min().z();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimIntersectionCollection::eclipseView() const
{
    RimEclipseView* eclipseView = nullptr;
    firstAncestorOrThisOfType( eclipseView );
    return eclipseView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::rebuild3dView() const
{
    Rim3dView* rimView = nullptr;
    firstAncestorOrThisOfType( rimView );
    if ( rimView )
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

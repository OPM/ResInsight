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

#include "RimViewController.h"

#include "RiaApplication.h"
#include "RiaOptionItemFactory.h"

#include "RigCaseToCaseCellMapper.h"
#include "RigCaseToCaseRangeFilterMapper.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellFilter.h"
#include "RimCellFilterCollection.h"
#include "RimCellRangeFilter.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimPolygonFilter.h"
#include "RimProject.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"

#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimViewController, "ViewController" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewController::RimViewController()
{
    CAF_PDM_InitObject( "View Link" );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    QString defaultName = "View Config: Empty view";
    CAF_PDM_InitField( &m_name, "Name", defaultName, "Managed View Name" );
    m_name.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_managedView, "ManagedView", "Linked View" );
    m_managedView.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_syncCamera, "SyncCamera", true, "Camera" );
    CAF_PDM_InitField( &m_showCursor, "ShowCursor", true, "   Show Cursor" );
    CAF_PDM_InitField( &m_syncTimeStep, "SyncTimeStep", true, "Time Step" );
    CAF_PDM_InitField( &m_syncCellResult, "SyncCellResult", false, "Cell Result" );
    CAF_PDM_InitField( &m_syncLegendDefinitions, "SyncLegendDefinitions", true, "   Color Legend" );

    CAF_PDM_InitField( &m_syncCellFilters, "SyncRangeFilters", false, "Cell Filters" );
    CAF_PDM_InitField( &m_syncPropertyFilters, "SyncPropertyFilters", false, "Property Filters" );
    m_syncPropertyFilters.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_duplicatePropertyFilters, "DuplicatePropertyFilters", false, "Property Filters" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewController::~RimViewController()
{
    removeOverrides();
    auto managedView = m_managedView();
    m_managedView    = nullptr;

    if ( managedView ) managedView->updateAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimViewController::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_managedView )
    {
        RimProject*             proj  = RimProject::current();
        std::vector<Rim3dView*> views = proj->allNotLinkedViews();

        // Add currently linked view to list
        if ( managedView() )
        {
            views.push_back( managedView() );
        }

        RimViewLinker* viewLinker = firstAncestorOrThisOfTypeAsserted<RimViewLinker>();
        for ( auto view : views )
        {
            if ( view != viewLinker->masterView() )
            {
                RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( view, &options );
            }
        }

        if ( !options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    updateDisplayNameAndIcon();
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isActive )
    {
        if ( !m_isActive )
        {
            applyCellFilterCollectionByUserChoice();
        }

        updateOverrides();
        updateDuplicatedPropertyFilters();
        updateResultColorsControl();
        updateCameraLink();
        updateDisplayNameAndIcon();
        updateTimeStepLink();

        if ( m_managedView ) m_managedView->updateMdiWindowTitle();
    }
    else if ( changedField == &m_syncCamera )
    {
        updateCameraLink();
    }
    else if ( changedField == &m_syncTimeStep )
    {
        updateTimeStepLink();
    }
    else if ( changedField == &m_showCursor )
    {
        if ( !m_showCursor && m_managedView && m_managedView->viewer() )
        {
            m_managedView->viewer()->setCursorPosition( cvf::Vec3d::UNDEFINED );
        }
    }
    else if ( changedField == &m_syncCellResult )
    {
        updateResultColorsControl();
        if ( managedEclipseView() )
        {
            managedEclipseView()->cellResult()->updateIconState();
        }
        else if ( managedGeoView() )
        {
            managedGeoView()->cellResult()->updateIconState();
        }
    }
    else if ( changedField == &m_syncLegendDefinitions )
    {
        updateLegendDefinitions();
    }
    else if ( changedField == &m_syncCellFilters )
    {
        if ( !m_syncCellFilters )
        {
            applyCellFilterCollectionByUserChoice();
        }
        updateOverrides();
    }
    else if ( changedField == &m_syncPropertyFilters )
    {
        updateOverrides();
    }
    else if ( changedField == &m_duplicatePropertyFilters )
    {
        updateDuplicatedPropertyFilters();
    }
    else if ( changedField == &m_managedView )
    {
        PdmObjectHandle* prevValue           = oldValue.value<caf::PdmPointer<PdmObjectHandle>>().rawPtr();
        auto*            previousManagedView = dynamic_cast<RimGridView*>( prevValue );
        RimViewController::removeOverrides( previousManagedView );

        setManagedView( m_managedView() );

        m_name.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimViewController::managedEclipseView() const
{
    return dynamic_cast<RimEclipseView*>( m_managedView() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimViewController::managedGeoView() const
{
    return dynamic_cast<RimGeoMechView*>( m_managedView() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateOverrides()
{
    RimViewLinker* viewLinker = ownerViewLinker();

    auto masterView = viewLinker->masterView();

    CVF_ASSERT( masterView );

    if ( m_managedView )
    {
        RimEclipseView* manEclView = managedEclipseView();
        RimGeoMechView* manGeoView = managedGeoView();

        if ( isVisibleCellsOveridden() )
        {
            if ( manEclView ) manEclView->setOverridePropertyFilterCollection( nullptr );
            if ( manGeoView ) manGeoView->setOverridePropertyFilterCollection( nullptr );
            m_managedView->scheduleGeometryRegen( OVERRIDDEN_CELL_VISIBILITY );
            m_managedView->scheduleCreateDisplayModelAndRedraw();
        }
        else
        {
            auto* masterEclipseView = dynamic_cast<RimEclipseView*>( masterView );
            if ( masterEclipseView )
            {
                if ( manEclView )
                {
                    if ( isPropertyFilterOveridden() )
                    {
                        manEclView->setOverridePropertyFilterCollection( masterEclipseView->eclipsePropertyFilterCollection() );
                    }
                    else
                    {
                        manEclView->setOverridePropertyFilterCollection( nullptr );
                    }
                }
            }

            auto* masterGeoView = dynamic_cast<RimGeoMechView*>( masterView );
            if ( masterGeoView )
            {
                if ( manGeoView )
                {
                    if ( isPropertyFilterOveridden() )
                    {
                        manGeoView->setOverridePropertyFilterCollection( masterGeoView->geoMechPropertyFilterCollection() );
                    }
                    else
                    {
                        manGeoView->setOverridePropertyFilterCollection( nullptr );
                    }
                }
            }
        }

        updateCellFilterOverrides( nullptr );

        if ( manGeoView )
        {
            manGeoView->updateIconStateForFilterCollections();
        }

        if ( manEclView )
        {
            manEclView->updateIconStateForFilterCollections();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::removeOverrides()
{
    removeOverrides( dynamic_cast<RimGridView*>( m_managedView() ) );

    RimEclipseView* manEclView = managedEclipseView();
    RimGeoMechView* manGeoView = managedGeoView();

    if ( manGeoView )
    {
        manGeoView->updateIconStateForFilterCollections();
    }

    if ( manEclView )
    {
        manEclView->updateIconStateForFilterCollections();
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::removeOverrides( RimGridView* view )
{
    if ( view )
    {
        auto* manEclView = dynamic_cast<RimEclipseView*>( view );
        auto* manGeoView = dynamic_cast<RimGeoMechView*>( view );

        if ( manEclView ) manEclView->setOverridePropertyFilterCollection( nullptr );
        if ( manGeoView ) manGeoView->setOverridePropertyFilterCollection( nullptr );

        view->setOverrideCellFilterCollection( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateOptionSensitivity()
{
    Rim3dView* mainView = nullptr;

    {
        RimViewLinker* linkedViews = firstAncestorOrThisOfTypeAsserted<RimViewLinker>();
        if ( linkedViews )
        {
            mainView = linkedViews->masterView();
        }
        CVF_ASSERT( mainView );
    }

    auto* eclipseMasterView = dynamic_cast<RimEclipseView*>( mainView );
    auto* geoMasterView     = dynamic_cast<RimGeoMechView*>( mainView );

    bool isMasterAndDependentViewDifferentType = false;
    if ( eclipseMasterView && !managedEclipseView() )
    {
        isMasterAndDependentViewDifferentType = true;
    }

    if ( geoMasterView && !managedGeoView() )
    {
        isMasterAndDependentViewDifferentType = true;
    }

    if ( isMasterAndDependentViewDifferentType )
    {
        m_syncCellResult.uiCapability()->setUiReadOnly( true );
        m_syncCellResult = false;

        m_syncLegendDefinitions.uiCapability()->setUiReadOnly( true );
        m_syncLegendDefinitions = false;
    }
    else
    {
        m_syncCellResult.uiCapability()->setUiReadOnly( false );

        if ( m_syncCellResult )
        {
            m_syncLegendDefinitions.uiCapability()->setUiReadOnly( false );
        }
        else
        {
            m_syncLegendDefinitions.uiCapability()->setUiReadOnly( true );
        }
    }

    if ( isPropertyFilterControlPossible() )
    {
        m_syncPropertyFilters.uiCapability()->setUiReadOnly( false );
    }
    else
    {
        m_syncPropertyFilters.uiCapability()->setUiReadOnly( true );
        m_syncPropertyFilters = false;
    }

    if ( m_syncCamera )
    {
        m_showCursor.uiCapability()->setUiReadOnly( false );
    }
    else
    {
        m_showCursor.uiCapability()->setUiReadOnly( true );
        m_showCursor = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RimViewController::managedView() const
{
    return m_managedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::setManagedView( Rim3dView* view )
{
    m_managedView = view;

    updateOptionSensitivity();
    updateOverrides();
    updateDuplicatedPropertyFilters();
    updateResultColorsControl();
    updateCameraLink();
    updateDisplayNameAndIcon();
    updateTimeStepLink();

    if ( m_managedView )
    {
        m_managedView->updateAutoName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    updateOptionSensitivity();
    uiOrdering.add( &m_managedView );

    caf::PdmUiGroup* scriptGroup = uiOrdering.addNewGroup( "Link Options" );

    scriptGroup->add( &m_syncCamera );
    scriptGroup->add( &m_showCursor );
    scriptGroup->add( &m_syncTimeStep );
    scriptGroup->add( &m_syncCellResult );
    scriptGroup->add( &m_syncLegendDefinitions );

    caf::PdmUiGroup* visibleCells = uiOrdering.addNewGroup( "Link Cell Filters" );
    visibleCells->add( &m_syncCellFilters );
    visibleCells->add( &m_syncPropertyFilters );
    visibleCells->add( &m_duplicatePropertyFilters );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateDisplayNameAndIcon()
{
    caf::IconProvider iconProvider;
    RimViewLinker::findNameAndIconFromView( &m_name.v(), &iconProvider, managedView() );
    iconProvider.setActive( m_isActive() );
    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateDuplicatedPropertyFilters()
{
    if ( !m_duplicatePropertyFilters )
    {
        // A chain icon is used to indicate that a property filter is linked. If a property filter is unlinked, update
        // the property filters to make sure the chain icon is removed

        auto eclipsePropertyFilters = RimProject::current()->descendantsIncludingThisOfType<RimPropertyFilterCollection>();
        for ( auto p : eclipsePropertyFilters )
        {
            p->updateConnectedEditors();
        }

        auto geoMechPropertyFilters = RimProject::current()->descendantsIncludingThisOfType<RimGeoMechPropertyFilterCollection>();
        for ( auto p : geoMechPropertyFilters )
        {
            p->updateConnectedEditors();
        }

        return;
    }

    RimViewLinker* viewLinker = ownerViewLinker();

    auto masterView = viewLinker->masterView();
    CVF_ASSERT( masterView );

    if ( m_managedView )
    {
        RimEclipseView* manEclView        = managedEclipseView();
        auto*           masterEclipseView = dynamic_cast<RimEclipseView*>( masterView );

        if ( masterEclipseView && manEclView )
        {
            auto propertyString = masterEclipseView->eclipsePropertyFilterCollection()->writeObjectToXmlString();
            manEclView->eclipsePropertyFilterCollection()->readObjectFromXmlString( propertyString, caf::PdmDefaultObjectFactory::instance() );
            manEclView->eclipsePropertyFilterCollection()->loadAndInitializePropertyFilters();
            manEclView->eclipsePropertyFilterCollection()->setIsDuplicatedFromLinkedView();
            manEclView->eclipsePropertyFilterCollection()->updateAllRequiredEditors();

            manEclView->scheduleGeometryRegen( PROPERTY_FILTERED );
            manEclView->scheduleCreateDisplayModelAndRedraw();
        }

        auto*           masterGeoView = dynamic_cast<RimGeoMechView*>( masterView );
        RimGeoMechView* manGeoView    = managedGeoView();
        if ( masterGeoView && manGeoView )
        {
            auto propertyString = masterGeoView->geoMechPropertyFilterCollection()->writeObjectToXmlString();
            manGeoView->geoMechPropertyFilterCollection()->readObjectFromXmlString( propertyString, caf::PdmDefaultObjectFactory::instance() );
            manGeoView->geoMechPropertyFilterCollection()->loadAndInitializePropertyFilters();
            manGeoView->geoMechPropertyFilterCollection()->updateAllRequiredEditors();

            manGeoView->scheduleGeometryRegen( PROPERTY_FILTERED );
            manGeoView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateCameraLink()
{
    if ( !isCameraLinked() ) return;
    if ( m_managedView )
    {
        RimViewLinker* viewLinker = ownerViewLinker();

        viewLinker->updateScaleZ( viewLinker->masterView(), viewLinker->masterView()->scaleZ() );
        viewLinker->updateCamera( viewLinker->masterView() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateTimeStepLink()
{
    if ( !isTimeStepLinked() ) return;

    if ( m_managedView )
    {
        RimViewLinker* viewLinker = ownerViewLinker();

        viewLinker->updateTimeStep( viewLinker->masterView(), viewLinker->masterView()->currentTimeStep() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateResultColorsControl()
{
    if ( !isResultColorControlled() ) return;

    RimViewLinker* viewLinker = ownerViewLinker();
    viewLinker->updateCellResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateLegendDefinitions()
{
    if ( !isLegendDefinitionsControlled() ) return;

    RimViewLinker* viewLinker = ownerViewLinker();
    viewLinker->updateCellResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker* RimViewController::ownerViewLinker() const
{
    return firstAncestorOrThisOfType<RimViewLinker>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCaseToCaseCellMapper* RimViewController::cellMapper()
{
    auto*           masterEclipseView = dynamic_cast<RimEclipseView*>( masterView() );
    RimEclipseView* dependEclipseView = managedEclipseView();
    auto*           masterGeomechView = dynamic_cast<RimGeoMechView*>( masterView() );
    RimGeoMechView* dependGeomechView = managedGeoView();

    RigMainGrid* masterEclGrid = nullptr;
    RigMainGrid* dependEclGrid = nullptr;
    RigFemPart*  masterFemPart = nullptr;
    RigFemPart*  dependFemPart = nullptr;

    if ( masterEclipseView )
    {
        masterEclGrid = masterEclipseView->mainGrid();
    }

    if ( dependEclipseView )
    {
        dependEclGrid = dependEclipseView->mainGrid();
    }

    if ( masterGeomechView && masterGeomechView->geoMechCase()->geoMechData() && masterGeomechView->femParts()->partCount() )
    {
        masterFemPart = masterGeomechView->femParts()->part( 0 );
    }

    if ( dependGeomechView && dependGeomechView->geoMechCase()->geoMechData() && dependGeomechView->femParts()->partCount() )
    {
        dependFemPart = dependGeomechView->femParts()->part( 0 );
    }

    // If we have the correct mapping already, return it.
    if ( m_caseToCaseCellMapper.notNull() )
    {
        if ( masterEclGrid == m_caseToCaseCellMapper->masterGrid() && dependEclGrid == m_caseToCaseCellMapper->dependentGrid() &&
             masterFemPart == m_caseToCaseCellMapper->masterFemPart() && dependFemPart == m_caseToCaseCellMapper->dependentFemPart() )
        {
            return m_caseToCaseCellMapper.p();
        }

        m_caseToCaseCellMapper = nullptr;
    }

    // Create the mapping if needed

    if ( m_caseToCaseCellMapper.isNull() )
    {
        if ( masterEclGrid && dependFemPart )
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper( masterEclGrid, dependFemPart );
        }
        else if ( masterEclGrid && dependEclGrid )
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper( masterEclGrid, dependEclGrid );
        }
        else if ( masterFemPart && dependFemPart )
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper( masterFemPart, dependFemPart );
        }
        else if ( masterFemPart && dependEclGrid )
        {
            m_caseToCaseCellMapper = new RigCaseToCaseCellMapper( masterFemPart, dependEclGrid );
        }
    }

    return m_caseToCaseCellMapper.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RimViewController::masterView() const
{
    return ownerViewLinker()->masterView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isMasterAndDepViewDifferentType() const
{
    auto* eclipseMasterView = dynamic_cast<RimEclipseView*>( masterView() );
    auto* geoMasterView     = dynamic_cast<RimGeoMechView*>( masterView() );

    bool isMasterAndDependentViewDifferentType = false;
    if ( eclipseMasterView && !managedEclipseView() )
    {
        isMasterAndDependentViewDifferentType = true;
    }
    if ( geoMasterView && !managedGeoView() )
    {
        isMasterAndDependentViewDifferentType = true;
    }

    return isMasterAndDependentViewDifferentType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::scheduleCreateDisplayModelAndRedrawForDependentView() const
{
    if ( !isActive() ) return;

    if ( isVisibleCellsOveridden() || isCellFiltersControlled() || isPropertyFilterOveridden() || isResultColorControlled() )
    {
        if ( managedView() )
        {
            managedView()->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if ( isResultColorControlled() && managedView() )
    {
        auto gridView = dynamic_cast<RimGridView*>( managedView() );
        if ( gridView ) gridView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::scheduleGeometryRegenForDepViews( RivCellSetEnum geometryType ) const
{
    if ( !isActive() ) return;

    if ( isVisibleCellsOveridden() || isCellFiltersControlled() || isPropertyFilterOveridden() || isResultColorControlled() )
    {
        if ( managedView() )
        {
            if ( isVisibleCellsOveridden() )
            {
                managedView()->scheduleGeometryRegen( OVERRIDDEN_CELL_VISIBILITY );
            }

            managedView()->scheduleGeometryRegen( geometryType );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isActive() const
{
    return ownerViewLinker()->isActive() && m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCameraLinked() const
{
    if ( ownerViewLinker()->isActive() && m_isActive() )
    {
        return m_syncCamera;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::showCursor() const
{
    return m_showCursor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isTimeStepLinked() const
{
    if ( ownerViewLinker()->isActive() && m_isActive() )
    {
        return m_syncTimeStep;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isResultColorControlled() const
{
    if ( ownerViewLinker()->isActive() && m_isActive() )
    {
        return m_syncCellResult;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isLegendDefinitionsControlled() const
{
    if ( ownerViewLinker()->isActive() && m_isActive() )
    {
        return m_syncLegendDefinitions;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isVisibleCellsOveridden() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCellFilterMappingApplicable() const
{
    if ( !isMasterAndDepViewDifferentType() ) return false;

    // Make sure the cases are in the same domain
    auto* eclipseView = dynamic_cast<RimEclipseView*>( masterView() );
    auto* geomView    = dynamic_cast<RimGeoMechView*>( masterView() );
    if ( !geomView ) geomView = managedGeoView();
    if ( !eclipseView ) eclipseView = managedEclipseView();

    if ( eclipseView && geomView )
    {
        if ( eclipseView->eclipseCase() && eclipseView->eclipseCase()->eclipseCaseData() && geomView->geoMechCase() &&
             geomView->geoMechCase()->geoMechData() )
        {
            RigMainGrid* eclGrid = eclipseView->mainGrid();
            RigFemPart*  femPart = geomView->femParts()->part( 0 );

            if ( eclGrid && femPart )
            {
                cvf::BoundingBox fembb = femPart->boundingBox();
                cvf::BoundingBox eclbb = eclGrid->boundingBox();
                return fembb.contains( eclbb.min() ) && fembb.contains( eclbb.max() );
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCellResultControlAdvisable() const
{
    bool contourMapMasterView  = dynamic_cast<RimEclipseContourMapView*>( masterView() ) != nullptr;
    bool contourMapManagedView = dynamic_cast<RimEclipseContourMapView*>( managedEclipseView() ) != nullptr;
    return !isMasterAndDepViewDifferentType() && ( contourMapMasterView != contourMapManagedView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCellFilterControlAdvisable() const
{
    bool contourMapMasterView  = dynamic_cast<RimEclipseContourMapView*>( masterView() ) != nullptr;
    bool contourMapManagedView = dynamic_cast<RimEclipseContourMapView*>( managedEclipseView() ) != nullptr;
    return contourMapMasterView != contourMapManagedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterControlAdvisable() const
{
    bool contourMapMasterView  = dynamic_cast<RimEclipseContourMapView*>( masterView() ) != nullptr;
    bool contourMapManagedView = dynamic_cast<RimEclipseContourMapView*>( managedEclipseView() ) != nullptr;
    return isPropertyFilterControlPossible() && ( contourMapMasterView != contourMapManagedView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isCellFiltersControlled() const
{
    if ( ownerViewLinker() && ownerViewLinker()->isActive() && m_isActive() )
    {
        return m_syncCellFilters;
    }

    return false;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterControlPossible() const
{
    // The cases need to be the same
    auto* geomView = dynamic_cast<RimGeoMechView*>( masterView() );

    if ( geomView )
    {
        RimGeoMechView* depGeomView = managedGeoView();
        if ( depGeomView && geomView->geoMechCase() == depGeomView->geoMechCase() )
        {
            return true;
        }
    }

    auto* eclipseView = dynamic_cast<RimEclipseView*>( masterView() );
    if ( eclipseView )
    {
        RimEclipseView* depEclipseView = managedEclipseView();
        if ( depEclipseView && eclipseView->eclipseCase() && depEclipseView->eclipseCase() &&
             eclipseView->eclipseCase() == depEclipseView->eclipseCase() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterOveridden() const
{
    if ( !isPropertyFilterControlPossible() ) return false;

    if ( ownerViewLinker()->isActive() && m_isActive() )
    {
        return m_syncPropertyFilters;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::isPropertyFilterDuplicationActive() const
{
    return m_duplicatePropertyFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updateCellFilterOverrides( const RimCellFilter* changedFilter )
{
    auto controlledGridView = dynamic_cast<RimGridView*>( m_managedView() );
    if ( !controlledGridView ) return;

    if ( !isCellFiltersControlled() )
    {
        controlledGridView->setOverrideCellFilterCollection( nullptr );
        return;
    }
    // Copy the rangeFilterCollection
    auto masterGridView = dynamic_cast<RimGridView*>( masterView() );
    if ( !masterGridView ) return;

    RimCellFilterCollection* sourceFilterCollection = masterGridView->cellFilterCollection();
    QString                  xmlFilterCollCopy      = sourceFilterCollection->writeObjectToXmlString();
    PdmObjectHandle*         objectCopy =
        PdmXmlObjectHandle::readUnknownObjectFromXmlString( xmlFilterCollCopy, caf::PdmDefaultObjectFactory::instance(), true );
    auto* overrideFilterColl = dynamic_cast<RimCellFilterCollection*>( objectCopy );

    std::vector<RimCellFilter*> srcFilters  = sourceFilterCollection->filters();
    std::vector<RimCellFilter*> dstFilters  = overrideFilterColl->filters();
    RimEclipseView*             depEclView  = managedEclipseView();
    RimGeoMechView*             depGeomView = managedGeoView();

    // Convert the cell filter to fit in the managed view if needed
    if ( isCellFilterMappingApplicable() )
    {
        auto* eclipseMasterView = dynamic_cast<RimEclipseView*>( masterView() );
        auto* geoMasterView     = dynamic_cast<RimGeoMechView*>( masterView() );

        if ( eclipseMasterView && depGeomView )
        {
            if ( eclipseMasterView->mainGrid() )
            {
                RigMainGrid* srcEclGrid = eclipseMasterView->mainGrid();
                RigFemPart*  dstFemPart = depGeomView->femParts()->part( 0 );

                for ( size_t rfIdx = 0; rfIdx < srcFilters.size(); ++rfIdx )
                {
                    overrideFilterColl->connectToFilterUpdates( dstFilters[rfIdx] );

                    auto* srcRFilter = dynamic_cast<RimCellRangeFilter*>( srcFilters[rfIdx] );
                    auto* dstRFilter = dynamic_cast<RimCellRangeFilter*>( dstFilters[rfIdx] );

                    if ( ( srcRFilter != nullptr ) && ( dstRFilter != nullptr ) )
                    {
                        RigCaseToCaseRangeFilterMapper::convertRangeFilterEclToFem( srcRFilter, srcEclGrid, dstRFilter, dstFemPart );
                        continue;
                    }

                    RimPolygonFilter* polyDstFilter = dynamic_cast<RimPolygonFilter*>( dstFilters[rfIdx] );
                    if ( polyDstFilter != nullptr )
                    {
                        RimGeoMechCase* gCase = depGeomView->geoMechCase();
                        polyDstFilter->setCase( gCase );
                        polyDstFilter->enableKFilter( false );
                    }
                }
            }
        }
        else if ( geoMasterView && depEclView )
        {
            if ( depEclView->mainGrid() )
            {
                RigFemPart*  srcFemPart = geoMasterView->femParts()->part( 0 );
                RigMainGrid* dstEclGrid = depEclView->mainGrid();
                for ( size_t rfIdx = 0; rfIdx < srcFilters.size(); ++rfIdx )
                {
                    overrideFilterColl->connectToFilterUpdates( dstFilters[rfIdx] );

                    RimCellRangeFilter* srcRFilter = dynamic_cast<RimCellRangeFilter*>( srcFilters[rfIdx] );
                    RimCellRangeFilter* dstRFilter = dynamic_cast<RimCellRangeFilter*>( dstFilters[rfIdx] );

                    if ( ( srcRFilter != nullptr ) && ( dstRFilter != nullptr ) )
                    {
                        RigCaseToCaseRangeFilterMapper::convertRangeFilterFemToEcl( srcRFilter, srcFemPart, dstRFilter, dstEclGrid );
                        continue;
                    }

                    RimPolygonFilter* polyDstFilter = dynamic_cast<RimPolygonFilter*>( dstFilters[rfIdx] );
                    if ( polyDstFilter != nullptr )
                    {
                        RimEclipseCase* eCase = depEclView->eclipseCase();
                        polyDstFilter->setCase( eCase );
                        polyDstFilter->enableKFilter( false );
                    }
                }
            }
        }
    }
    else
    {
        for ( auto& dstFilter : dstFilters )
        {
            overrideFilterColl->connectToFilterUpdates( dstFilter );

            RimPolygonFilter* polyDstFilter = dynamic_cast<RimPolygonFilter*>( dstFilter );
            if ( polyDstFilter != nullptr )
            {
                RimCase* theCase = nullptr;
                if ( depEclView ) theCase = depEclView->eclipseCase();
                if ( depGeomView ) theCase = depGeomView->geoMechCase();
                polyDstFilter->setCase( theCase );
            }
        }
    }

    controlledGridView->setOverrideCellFilterCollection( overrideFilterColl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::updatePropertyFilterOverrides( RimPropertyFilter* changedPropertyFilter )
{
    updateOverrides();
    updateDuplicatedPropertyFilters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewController::applyCellFilterCollectionByUserChoice()
{
    auto managedGridView = dynamic_cast<RimGridView*>( m_managedView() );

    if ( !managedGridView ) return;

    if ( !managedGridView->hasOverriddenCellFilterCollection() )
    {
        return;
    }

    RimViewLinker* viewLinker = ownerViewLinker();
    auto*          masterView = dynamic_cast<RimGridView*>( viewLinker->masterView() );

    bool anyActiveCellFilter = false;

    if ( masterView )
    {
        anyActiveCellFilter = !masterView->cellFilterCollection()->filters().empty();
    }

    if ( anyActiveCellFilter && askUserToRestoreOriginalCellFilterCollection( m_managedView->name() ) )
    {
        managedGridView->setOverrideCellFilterCollection( nullptr );
    }
    else
    {
        managedGridView->replaceCellFilterCollectionWithOverride();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewController::askUserToRestoreOriginalCellFilterCollection( const QString& viewName )
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    QMessageBox msgBox( activeView->viewer()->layoutWidget() );
    msgBox.setIcon( QMessageBox::Question );

    QString questionText;
    questionText = QString( "The cell filters in the view \"%1\" are about to be unlinked." ).arg( viewName );

    msgBox.setText( questionText );
    msgBox.setInformativeText( "Do you want to keep the cell filters from the primary view?" );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

    int ret = msgBox.exec();
    return ret != QMessageBox::Yes;
}

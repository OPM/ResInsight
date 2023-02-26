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

#include "RimViewLinker.h"

#include "RiaOptionItemFactory.h"

#include "RigFemResultAddress.h"
#include "RigMainGrid.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimViewLinkerCollection.h"
#include "RimViewManipulator.h"

#include "RiuViewer.h"

#include "cafIconProvider.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfCamera.h"
#include "cvfMatrix4.h"
#include "cvfScene.h"

CAF_PDM_SOURCE_INIT( RimViewLinker, "ViewLinker" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker::RimViewLinker()
{
    CAF_PDM_InitObject( "Linked Views" );

    CAF_PDM_InitField( &m_name, "Name", QString( "View Group Name" ), "View Group Name" );
    m_name.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_masterView, "MainView", "Main View" );
    m_masterView.uiCapability()->setUiTreeChildrenHidden( true );
    m_masterView.uiCapability()->setUiTreeHidden( true );
    m_masterView.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_viewControllers, "ManagedViews", "Managed Views" );
    m_viewControllers.uiCapability()->setUiTreeHidden( true );
    m_viewControllers.uiCapability()->setUiTreeChildrenHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewLinker::~RimViewLinker()
{
    removeOverrides();

    m_viewControllers.deleteChildren();
    auto masterView = m_masterView();
    m_masterView    = nullptr;
    if ( masterView ) masterView->updateAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateTimeStep( Rim3dView* sourceView, int timeStep )
{
    CVF_ASSERT( sourceView );

    if ( !isActive() ) return;

    if ( masterView() != sourceView )
    {
        RimViewController* sourceViewLink = sourceView->viewController();
        CVF_ASSERT( sourceViewLink );

        if ( !sourceViewLink->isTimeStepLinked() )
        {
            return;
        }
    }

    if ( m_masterView && m_masterView->viewer() && sourceView != m_masterView )
    {
        m_masterView->viewer()->setCurrentFrame( timeStep );
    }

    for ( RimViewController* viewController : m_viewControllers )
    {
        if ( !viewController->isTimeStepLinked() ) continue;

        if ( viewController->managedView() && viewController->managedView() != sourceView && viewController->managedView()->viewer() )
        {
            viewController->managedView()->viewer()->setCurrentFrame( timeStep );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCellResult()
{
    Rim3dView*      rimView           = m_masterView;
    RimEclipseView* masterEclipseView = dynamic_cast<RimEclipseView*>( rimView );
    if ( masterEclipseView && masterEclipseView->cellResult() )
    {
        RimEclipseResultDefinition* eclipseCellResultDefinition = masterEclipseView->cellResult();

        for ( RimViewController* viewController : m_viewControllers )
        {
            if ( viewController->managedView() )
            {
                Rim3dView*      managedView = viewController->managedView();
                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( managedView );
                if ( eclipseView )
                {
                    if ( viewController->isResultColorControlled() )
                    {
                        eclipseView->cellResult()->simpleCopy( eclipseCellResultDefinition );
                        eclipseView->cellResult()->loadResult();

                        if ( viewController->isLegendDefinitionsControlled() )
                        {
                            eclipseView->cellResult()->legendConfig()->setUiValuesFromLegendConfig(
                                masterEclipseView->cellResult()->legendConfig() );
                            eclipseView->cellResult()->legendConfig()->updateLegend();

                            eclipseView->cellResult()->ternaryLegendConfig()->setUiValuesFromLegendConfig(
                                masterEclipseView->cellResult()->ternaryLegendConfig() );
                        }

                        eclipseView->scheduleCreateDisplayModelAndRedraw();
                        eclipseView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
                    }

                    eclipseView->cellResult()->updateIconState();
                }
            }
        }
    }

    RimGeoMechView* masterGeoView = dynamic_cast<RimGeoMechView*>( rimView );
    if ( masterGeoView && masterGeoView->cellResult() )
    {
        RimGeoMechResultDefinition* geoMechResultDefinition = masterGeoView->cellResult();

        for ( RimViewController* viewController : m_viewControllers )
        {
            if ( viewController->managedView() )
            {
                Rim3dView*      managedView = viewController->managedView();
                RimGeoMechView* geoView     = dynamic_cast<RimGeoMechView*>( managedView );
                if ( geoView )
                {
                    if ( viewController->isResultColorControlled() )
                    {
                        geoView->cellResult()->setResultAddress( geoMechResultDefinition->resultAddress() );

                        if ( viewController->isLegendDefinitionsControlled() )
                        {
                            geoView->cellResult()->legendConfig()->setUiValuesFromLegendConfig( masterGeoView->cellResult()->legendConfig() );
                            geoView->cellResult()->legendConfig()->updateLegend();
                        }

                        geoView->scheduleCreateDisplayModelAndRedraw();
                        geoView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
                    }

                    geoView->cellResult()->updateIconState();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCellFilters( const RimCellFilter* changedFilter )
{
    for ( RimViewController* viewController : m_viewControllers )
    {
        viewController->updateCellFilterOverrides( changedFilter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateOverrides()
{
    for ( RimViewController* viewController : m_viewControllers )
    {
        if ( viewController->isActive() )
        {
            viewController->updateOverrides();
        }
        else
        {
            viewController->removeOverrides();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateWindowTitles()
{
    if ( m_masterView ) m_masterView->updateMdiWindowTitle();

    for ( RimViewController* viewController : m_viewControllers )
    {
        if ( auto view = viewController->managedView() )
        {
            view->updateMdiWindowTitle();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateDuplicatedPropertyFilters()
{
    for ( RimViewController* viewController : m_viewControllers )
    {
        viewController->updateDuplicatedPropertyFilters();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::removeOverrides()
{
    for ( const auto& viewController : m_viewControllers )
    {
        if ( viewController->managedView() )
        {
            viewController->removeOverrides();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateScaleWidgetVisibility()
{
    // Create new display model that will call RimEclipseContourMapView::onUpdateLegends() where the visibility of scale
    // widgets is controlled

    if ( masterView() ) masterView()->scheduleCreateDisplayModelAndRedraw();

    for ( RimViewController* viewController : m_viewControllers )
    {
        if ( viewController->managedView() )
        {
            viewController->managedView()->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::allViewsForCameraSync( const Rim3dView* source, std::vector<Rim3dView*>& views ) const
{
    if ( !isActive() ) return;

    if ( source != m_masterView() )
    {
        views.push_back( m_masterView() );
    }

    for ( const auto& viewController : m_viewControllers )
    {
        if ( viewController->managedView() && source != viewController->managedView() )
        {
            if ( viewController->isCameraLinked() )
            {
                views.push_back( viewController->managedView() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateDependentViews()
{
    if ( m_viewControllers.empty() ) return;

    updateOverrides();
    updateDuplicatedPropertyFilters();
    updateCellResult();
    updateScaleZ( m_masterView, m_masterView->scaleZ() );
    updateCamera( m_masterView );
    updateTimeStep( m_masterView, m_masterView->currentTimeStep() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimViewLinker::displayNameForView( Rim3dView* view )
{
    QString displayName = "None";

    if ( view )
    {
        displayName = view->autoName();
    }

    return displayName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::setMasterView( Rim3dView* view )
{
    RimViewController* previousViewController = nullptr;
    if ( view ) previousViewController = view->viewController();

    // Remove the view as dependent view
    if ( previousViewController )
    {
        delete previousViewController;
        this->m_viewControllers.removeChild( nullptr );
    }

    this->removeOverrides();

    m_masterView = view;

    updateUiNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RimViewLinker::masterView() const
{
    return m_masterView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimViewLinker::allViews() const
{
    std::vector<Rim3dView*> views;

    views.push_back( m_masterView() );

    for ( const auto& viewController : m_viewControllers )
    {
        if ( viewController->managedView() )
        {
            views.push_back( viewController->managedView() );
        }
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::initAfterRead()
{
    updateUiNameAndIcon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateScaleZ( Rim3dView* sourceView, double scaleZ )
{
    if ( !isActive() ) return;

    if ( masterView() != sourceView )
    {
        RimViewController* sourceViewLink = sourceView->viewController();
        CVF_ASSERT( sourceViewLink );

        if ( !sourceViewLink->isCameraLinked() )
        {
            return;
        }
    }

    std::vector<Rim3dView*> views;
    allViewsForCameraSync( sourceView, views );

    // Make sure scale factors are identical
    for ( auto& view : views )
    {
        view->setScaleZAndUpdate( scaleZ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewLinker::isActive() const
{
    RimViewLinkerCollection* viewLinkerCollection = nullptr;
    this->firstAncestorOrThisOfType( viewLinkerCollection );

    if ( !viewLinkerCollection )
    {
        // This will happen when the all linked views are about to be deleted
        // The viewLinker is taken out of the viewLinkerCollection, and no parent can be found
        // See RicDeleteAllLinkedViewsFeature
        return false;
    }

    return viewLinkerCollection->isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateUiNameAndIcon()
{
    caf::IconProvider iconProvider;

    QString name;
    RimViewLinker::findNameAndIconFromView( &name, &iconProvider, m_masterView );
    name += " (Primary)";
    m_name.v() = name;

    if ( m_masterView ) m_masterView->updateAutoName();

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::scheduleGeometryRegenForDepViews( RivCellSetEnum geometryType )
{
    for ( const auto& viewController : m_viewControllers )
    {
        viewController->scheduleGeometryRegenForDepViews( geometryType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::scheduleCreateDisplayModelAndRedrawForDependentViews()
{
    for ( const auto& viewController : m_viewControllers )
    {
        viewController->scheduleCreateDisplayModelAndRedrawForDependentView();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::findNameAndIconFromView( QString* name, caf::IconProvider* icon, Rim3dView* view )
{
    CVF_ASSERT( name && icon );

    if ( view )
    {
        *name = displayNameForView( view );
        *icon = view->uiIconProvider();
    }
    else
    {
        *icon = caf::IconProvider();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCursorPosition( const Rim3dView* sourceView, const cvf::Vec3d& domainCoord )
{
    RimViewController* sourceViewLink = sourceView->viewController();
    if ( sourceViewLink && !sourceViewLink->showCursor() )
    {
        return;
    }

    std::vector<Rim3dView*> viewsToUpdate;
    allViewsForCameraSync( sourceView, viewsToUpdate );

    for ( Rim3dView* destinationView : viewsToUpdate )
    {
        if ( destinationView == sourceView ) continue;

        if ( destinationView != m_masterView )
        {
            RimViewController* viewLink = destinationView->viewController();
            if ( !viewLink ) continue;

            if ( !viewLink->showCursor() ) continue;
        }

        RiuViewer* destinationViewer = destinationView->viewer();
        if ( destinationViewer )
        {
            destinationViewer->setCursorPosition( domainCoord );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimViewLinkerCollection* viewLinkerCollection = nullptr;
    this->firstAncestorOrThisOfType( viewLinkerCollection );
    if ( viewLinkerCollection ) viewLinkerCollection->updateConnectedEditors();

    updateScaleWidgetVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimViewLinker::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    Rim3dView* actualComparisonView = nullptr;
    if ( m_masterView() )
    {
        actualComparisonView = m_masterView->activeComparisonView();
    }

    bool isActiveCompViewInList = false;
    for ( const auto& viewController : m_viewControllers )
    {
        if ( viewController->managedView() )
        {
            RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( viewController->managedView(), &options );
            if ( viewController->managedView() == actualComparisonView )
            {
                isActiveCompViewInList = true;
            }
        }
    }

    if ( !isActiveCompViewInList && actualComparisonView != nullptr )
    {
        // Add the actually used comparison view to the option list, even though it is not one of the linked views
        options.push_front( caf::PdmOptionItemInfo( actualComparisonView->autoName(),
                                                    actualComparisonView,
                                                    false,
                                                    actualComparisonView->uiCapability()->uiIconProvider() ) );
    }

    options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updateCamera( Rim3dView* sourceView )
{
    if ( !sourceView || !sourceView->viewer() ) return;

    if ( !isActive() ) return;

    RimViewController* viewLink = sourceView->viewController();
    if ( viewLink )
    {
        if ( !viewLink->isCameraLinked() )
        {
            return;
        }
    }

    std::vector<Rim3dView*> viewsToUpdate;
    allViewsForCameraSync( sourceView, viewsToUpdate );

    RimViewManipulator::applySourceViewCameraOnDestinationViews( sourceView, viewsToUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::addDependentView( Rim3dView* view )
{
    CVF_ASSERT( view && view != m_masterView );

    if ( !view->viewController() )
    {
        RimViewController* viewContr = new RimViewController;
        this->m_viewControllers.push_back( viewContr );

        viewContr->setManagedView( view );
    }

    updateScaleWidgetVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewLinker::isFirstViewDependentOnSecondView( const Rim3dView* firstView, const Rim3dView* secondView ) const
{
    for ( const RimViewController* controller : m_viewControllers() )
    {
        if ( controller->masterView() == secondView && controller->managedView() == firstView )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::addViewControllers( caf::PdmUiTreeOrdering& uiTreeOrdering ) const
{
    for ( const auto& viewController : m_viewControllers )
    {
        if ( viewController ) uiTreeOrdering.add( viewController );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::applyCellFilterCollectionByUserChoice()
{
    for ( const auto& viewController : m_viewControllers )
    {
        viewController->applyCellFilterCollectionByUserChoice();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::updatePropertyFilters( RimPropertyFilter* changedPropertyFilter )
{
    for ( RimViewController* viewLink : m_viewControllers )
    {
        viewLink->updatePropertyFilterOverrides( changedPropertyFilter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewLinker::removeViewController( RimViewController* viewController )
{
    m_viewControllers.removeChild( viewController );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RimViewLinker::firstControlledView()
{
    if ( m_viewControllers.empty() ) return nullptr;

    return m_viewControllers[0]->managedView();
}

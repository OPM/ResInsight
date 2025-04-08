/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RicSeismicSectionFromIntersectionFeature.h"

#include "RiaApplication.h"
#include "RiaSeismicDefines.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimPolylineTarget.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include "cvfBoundingBox.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicSeismicSectionFromIntersectionFeature, "RicSeismicSectionFromIntersectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSeismicSectionFromIntersectionFeature::isCommandEnabled() const
{
    RimExtrudedCurveIntersection* intersection = getSelectedIntersection();
    if ( intersection != nullptr )
    {
        return ( ( intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE ) ||
                 ( intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYGON ) ||
                 ( intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_WELL_PATH ) );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSeismicSectionFromIntersectionFeature::onActionTriggered( bool isChecked )
{
    RimExtrudedCurveIntersection* intersection = getSelectedIntersection();
    if ( intersection == nullptr ) return;

    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if ( activeView == nullptr ) return;

    auto bbox = activeView->domainBoundingBox();

    RimSeismicSectionCollection* seisColl   = activeView->seismicSectionCollection();
    RimSeismicSection*           newSection = seisColl->addNewSection();
    if ( !newSection ) return;

    if ( ( intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE ) ||
         ( intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYGON ) )
    {
        newSection->setSectionType( RiaDefines::SeismicSectionType::SS_POLYLINE );
        newSection->setUserDescription( intersection->name() );

        auto polyline = intersection->polyLines();
        if ( !polyline.empty() )
        {
            for ( auto& p : polyline[0] )
            {
                RimPolylineTarget* target = new RimPolylineTarget();
                target->setAsPointXYZ( p );
                newSection->addTargetNoUpdate( target );
            }
        }
    }
    else if ( ( intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_WELL_PATH ) )
    {
        newSection->setSectionType( RiaDefines::SeismicSectionType::SS_WELLPATH );
        newSection->setWellPath( intersection->wellPath() );
    }

    if ( intersection->depthFilterType() == RimIntersectionFilterEnum::INTERSECT_FILTER_NONE )
    {
        newSection->setDepthFilter( RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN,
                                    (int)( -1 * bbox.max().z() ),
                                    (int)( -1 * bbox.min().z() ) );
    }
    else
    {
        newSection->setDepthFilter( intersection->depthFilterType(),
                                    (int)-1 * intersection->upperFilterDepth( bbox.max().z() ),
                                    (int)-1 * intersection->lowerFilterDepth( ( bbox.min().z() ) ) );
    }

    Riu3DMainWindowTools::selectAsCurrentItem( newSection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSeismicSectionFromIntersectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SeismicSection16x16.png" ) );
    actionToSetup->setText( "Create as Seismic Section" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection* RicSeismicSectionFromIntersectionFeature::getSelectedIntersection() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimExtrudedCurveIntersection>();
}

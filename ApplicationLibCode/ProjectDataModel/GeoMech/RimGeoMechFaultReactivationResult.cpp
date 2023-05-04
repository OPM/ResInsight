/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimGeoMechFaultReactivationResult.h"

#include "RiaApplication.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiPushButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimGeoMechFaultReactivationResult, "RimGeoMechFaultReactivationResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechFaultReactivationResult::RimGeoMechFaultReactivationResult()
{
    // TODO: Update icon
    CAF_PDM_InitObject( "Fault Reactivation Result", ":/GeoMechCase24x24.png" );

    CAF_PDM_InitFieldNoDefault( &m_intersection, "Intersection", "Intersection" );

    CAF_PDM_InitField( &m_wellDistanceFromIntersection, "FaceDistanceFromIntersection", 0.0, "Face Distance From Intersection" );
    CAF_PDM_InitField( &m_wellWidthOutsideIntersection, "FaceWidthOutsideIntersection", 0.0, "Face Width Outside Intersection" );

    CAF_PDM_InitFieldNoDefault( &m_createFaultReactivationResult, "CreateReactivationResult", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_createFaultReactivationResult );

    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechFaultReactivationResult::~RimGeoMechFaultReactivationResult()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechFaultReactivationResult::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_intersection )
    {
        RimGridView* activeView = RiaApplication::instance()->activeGridView();
        if ( !activeView || !activeView->intersectionCollection() ) return options;

        for ( auto* intersection : activeView->intersectionCollection()->intersections() )
        {
            // Only utilize polyline intersections?
            if ( intersection && intersection->type() == RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE )
                options.push_back( caf::PdmOptionItemInfo( intersection->name(), intersection ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Fault Reactivation Result" );
    group->add( &m_intersection );
    group->add( &m_wellDistanceFromIntersection );
    group->add( &m_wellWidthOutsideIntersection );
    group->add( &m_createFaultReactivationResult );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    if ( changedField == &m_createFaultReactivationResult && m_intersection() )
    {
        // Using first two points from first polyline
        const auto polyLines = m_intersection()->polyLines();
        if ( polyLines.size() != 1 || polyLines[0].size() != 2 ) return;

        const std::vector<cvf::Vec3d> wellPoints = { polyLines[0][0], polyLines[0][1] };

        RimModeledWellPath* faceAWellPath            = new RimModeledWellPath;
        RimModeledWellPath* faceBWellPath            = new RimModeledWellPath;
        auto*               faceAWellPathGeometryDef = faceAWellPath->geometryDefinition();
        auto*               faceBWellPathGeometryDef = faceBWellPath->geometryDefinition();

        if ( !faceAWellPathGeometryDef || !faceBWellPathGeometryDef ) return;

        faceAWellPath->setName( "Fault Face A Well" );
        faceBWellPath->setName( "Fault Face B Well" );

        faceAWellPathGeometryDef->createAndInsertTargets( wellPoints );
        faceBWellPathGeometryDef->createAndInsertTargets( wellPoints );
        faceAWellPathGeometryDef->setUseAutoGeneratedTargetAtSeaLevel( false );
        faceBWellPathGeometryDef->setUseAutoGeneratedTargetAtSeaLevel( false );
        faceAWellPath->createWellPathGeometry();
        faceBWellPath->createWellPathGeometry();

        RimWellPathCollection* wellPathCollection = RimProject::current()->activeOilField()->wellPathCollection();
        if ( wellPathCollection )
        {
            wellPathCollection->addWellPath( faceAWellPath );
            wellPathCollection->addWellPath( faceBWellPath );

            wellPathCollection->uiCapability()->updateConnectedEditors();

            RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
        }

        // Add well paths to internal storage?
        // Apply m_wellDistanceFromIntersection and m_wellWidthOutsideIntersection for adjustement of well paths
        // Find vector normal onto intersection plane, i.e. cross product of z-axis and wellpath angle
        // Create vector from first and last point in well paths/ wellPoints and use w/ z-axis to create normal vector with cross product
    }
    // if ( changedField == objectToggleField() )
    //{
    //     RimGeoMechView* ownerView;
    //     firstAncestorOrThisOfType( ownerView );
    //     if ( ownerView ) ownerView->scheduleCreateDisplayModelAndRedraw();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechFaultReactivationResult::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                               QString                    uiConfigName,
                                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_createFaultReactivationResult )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Apply";
        }
    }
}

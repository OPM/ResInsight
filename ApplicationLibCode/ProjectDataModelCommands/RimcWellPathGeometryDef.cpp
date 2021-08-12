/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimcWellPathGeometryDef.h"

#include "RimModeledWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathGeometryDef,
                                   RimcRimWellPathGeometryDef_appendNewWellTarget,
                                   "AppendWellTarget" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcRimWellPathGeometryDef_appendNewWellTarget::RimcRimWellPathGeometryDef_appendNewWellTarget( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create and Add New Well Target", "", "", "Create and Add New Well Target" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinate, "Coordinate", "", "", "", "Coordinate" );
    CAF_PDM_InitScriptableField( &m_isAbsolute, "Absolute", false, "", "", "", "Relative or Absolute Coordinate" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcRimWellPathGeometryDef_appendNewWellTarget::execute()
{
    auto geoDef = self<RimWellPathGeometryDef>();

    cvf::Vec3d relativeTargetPoint = m_coordinate();
    relativeTargetPoint.z()        = -relativeTargetPoint.z();
    if ( m_isAbsolute )
    {
        cvf::Vec3d referencePoint = geoDef->anchorPointXyz();
        relativeTargetPoint -= referencePoint;
    }

    auto newTarget = new RimWellPathTarget;
    newTarget->setAsPointTargetXYD(
        cvf::Vec3d( relativeTargetPoint.x(), relativeTargetPoint.y(), -relativeTargetPoint.z() ) );
    geoDef->insertTarget( nullptr, newTarget );

    geoDef->updateConnectedEditors();
    geoDef->updateWellPathVisualization( false );

    return newTarget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcRimWellPathGeometryDef_appendNewWellTarget::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcRimWellPathGeometryDef_appendNewWellTarget::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimWellPathTarget );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimWellPathGeometryDefTools.h"

#include "RimModeledWellPath.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathGeometryDefTools::updateLinkedGeometryDefinitions( std::vector<RimWellPathGeometryDef*>& definitions,
                                                                   cvf::Vec3d                            delta )
{
    for ( auto wellPathGeoDef : definitions )
    {
        auto currentRefPointXyz = wellPathGeoDef->anchorPointXyz();
        auto newRefPointXyz     = currentRefPointXyz - delta;

        wellPathGeoDef->setReferencePointXyz( newRefPointXyz );

        wellPathGeoDef->changed.send( false );
        wellPathGeoDef->updateWellPathVisualization( true );
        for ( auto wt : wellPathGeoDef->activeWellTargets() )
        {
            wt->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathGeometryDef*> RimWellPathGeometryDefTools::linkedDefinitions()
{
    std::vector<RimWellPathGeometryDef*> linkedWellPathGeoDefs;

    for ( auto w : RimTools::wellPathCollection()->allWellPaths() )
    {
        auto modeledWellPath = dynamic_cast<RimModeledWellPath*>( w );
        if ( !modeledWellPath ) continue;

        if ( modeledWellPath->geometryDefinition()->isReferencePointUpdatesLinked() )
        {
            linkedWellPathGeoDefs.push_back( modeledWellPath->geometryDefinition() );
        }
    }

    return linkedWellPathGeoDefs;
}

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

#include "RimFishboneWellPathCollection.h"

#include "Rim3dView.h"
#include "RimFishboneWellPath.h"
#include "RimFishbonesCollection.h"
#include "RimProject.h"

#include "RigWellPath.h"

#include "RifWellPathImporter.h"

#include "Riu3DMainWindowTools.h"

CAF_PDM_SOURCE_INIT( RimFishboneWellPathCollection, "WellPathCompletionCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishboneWellPathCollection::RimFishboneWellPathCollection()
{
    CAF_PDM_InitObject( "WellPathCompletions", ":/FishBoneGroupFromFile16x16.png", "", "" );

    nameField()->uiCapability()->setUiHidden( true );
    this->setName( "Imported Laterals" );

    CAF_PDM_InitFieldNoDefault( &m_wellPaths, "WellPaths", "Imported Laterals", "", "", "" );
    m_wellPaths.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_pipeProperties, "PipeProperties", "Pipe Properties", "", "", "" );
    m_pipeProperties.uiCapability()->setUiHidden( true );
    m_pipeProperties.uiCapability()->setUiTreeHidden( true );
    m_pipeProperties.uiCapability()->setUiTreeChildrenHidden( true );
    m_pipeProperties = new RimFishbonesPipeProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::importCompletionsFromFile( const QStringList& filePaths )
{
    RifWellPathImporter wellPathImporter;

    for ( const QString& filePath : filePaths )
    {
        size_t wellDataCount = wellPathImporter.wellDataCount( filePath );

        for ( size_t i = 0; i < wellDataCount; ++i )
        {
            RifWellPathImporter::WellData wellData       = wellPathImporter.readWellData( filePath, i );
            RimFishboneWellPath*          wellCompletion = new RimFishboneWellPath();
            wellCompletion->setName( wellData.m_name );
            wellCompletion->setCoordinates( wellData.m_wellPathGeometry->m_wellPathPoints );
            wellCompletion->setMeasuredDepths( wellData.m_wellPathGeometry->m_measuredDepths );
            appendCompletion( wellCompletion );
        }
    }

    RimFishbonesCollection* fishbonesCollection;
    firstAncestorOrThisOfType( fishbonesCollection );
    if ( fishbonesCollection != nullptr )
    {
        fishbonesCollection->recalculateStartMD();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted( proj );
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimFishboneWellPath*> RimFishboneWellPathCollection::wellPaths() const
{
    std::vector<const RimFishboneWellPath*> paths;

    for ( const RimFishboneWellPath* path : m_wellPaths )
    {
        paths.push_back( path );
    }

    return paths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::setUnitSystemSpecificDefaults()
{
    m_pipeProperties->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* wellPropertiesGroup = uiOrdering.addNewGroup( "Well Properties" );
    m_pipeProperties->uiOrdering( uiConfigName, *wellPropertiesGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishboneWellPathCollection::appendCompletion( RimFishboneWellPath* completion )
{
    m_wellPaths.push_back( completion );

    updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( completion );

    uiCapability()->setUiHidden( !m_wellPaths.empty() );

    RimProject* project = nullptr;
    firstAncestorOrThisOfTypeAsserted( project );
    if ( project )
    {
        project->reloadCompletionTypeResultsInAllViews();
    }
}

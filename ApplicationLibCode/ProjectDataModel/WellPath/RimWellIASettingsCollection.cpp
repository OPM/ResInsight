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

#include "RimWellIASettingsCollection.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimGeoMechCase.h"
#include "RimProject.h"
#include "RimWellIASettings.h"
#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include "QFile"

CAF_PDM_SOURCE_INIT( RimWellIASettingsCollection, "RimWellIASettingsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIASettingsCollection::RimWellIASettingsCollection()
{
    CAF_PDM_InitObject( "Integrity Analysis Models", ":/WellIntAnalysis.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellIASettings, "WellIASettings", "Settings" );
    m_wellIASettings.uiCapability()->setUiHidden( true );
    m_wellIASettings.uiCapability()->setUiTreeHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIASettingsCollection::~RimWellIASettingsCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIASettings* RimWellIASettingsCollection::startWellIntegrationAnalysis( QString         baseDir,
                                                                              RimWellPath*    wellPath,
                                                                              double          measuredDepth,
                                                                              RimGeoMechCase* theCase,
                                                                              QString&        outErrmsg )
{
    RimWellIASettings* modelSettings = new RimWellIASettings();
    modelSettings->setGeoMechCase( theCase );

    double depthSize = 50;
    if ( theCase ) depthSize = theCase->characteristicCellSize();
    modelSettings->setDepthInterval( measuredDepth, measuredDepth + depthSize );
    modelSettings->setOutputBaseDirectory( baseDir );

    QString errmsg;
    if ( !modelSettings->initSettings( errmsg ) )
    {
        delete modelSettings;
        outErrmsg = "Unable to load default parameters from the WIA default parameter XML file:\n" + errmsg;
        return nullptr;
    }

    m_wellIASettings.push_back( modelSettings );

    return modelSettings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellIASettings*> RimWellIASettingsCollection::settings() const
{
    return m_wellIASettings.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettingsCollection::isEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettingsCollection::hasSettings() const
{
    return m_wellIASettings.size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettingsCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                                  std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    RimWellPath* wellPath;
    this->firstAncestorOrThisOfType( wellPath );
    if ( wellPath ) wellPath->updateConnectedEditors();
    RiaApplication::instance()->project()->scheduleCreateDisplayModelAndRedrawAllViews();
}

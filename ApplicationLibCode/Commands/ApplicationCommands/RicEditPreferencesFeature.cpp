/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicEditPreferencesFeature.h"

#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RimProject.h"

#include "RiuPropertyViewTabWidget.h"

#include "cafPdmSettings.h"
#include "cafPdmUiModelChangeDetector.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicEditPreferencesFeature, "RicEditPreferencesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicEditPreferencesFeature::isCommandEnabled()
{
    return true;
}

std::vector<caf::FontHolderInterface*> findFontObjects()
{
    auto                                   project = RimProject::current();
    std::vector<caf::FontHolderInterface*> allFontObjects;
    project->descendantsIncludingThisOfType( allFontObjects );

    std::vector<caf::FontHolderInterface*> defaultFontObjects;
    for ( auto fontObject : allFontObjects )
    {
        defaultFontObjects.push_back( fontObject );
    }
    return defaultFontObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditPreferencesFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RiaGuiApplication* app = RiaGuiApplication::instance();

    QStringList tabNames = app->preferences()->tabNames();

    auto defaultFontObjects = findFontObjects();

    std::unique_ptr<RiaPreferences> oldPreferences = clonePreferences( app->preferences() );

    RiuPropertyViewTabWidget propertyDialog( nullptr, app->preferences(), "Preferences", tabNames );
    propertyDialog.setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        // Write preferences using QSettings  and apply them to the application
        app->applyPreferences();
        app->applyGuiPreferences( oldPreferences.get(), defaultFontObjects );       
    }
    else
    {
        // Read back currently stored values using QSettings
        caf::PdmSettings::readFieldsFromApplicationStore( app->preferences() );
        app->preferences()->initAfterReadRecursively();
    }

    if ( !app->isProjectSavedToDisc() )
    {
        // Always reset change detector when modifying preferences, as these changes are irrelevant
        // when the project we work on is not saved to disc
        caf::PdmUiModelChangeDetector::instance()->reset();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicEditPreferencesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&Preferences..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RiaPreferences> RicEditPreferencesFeature::clonePreferences( const RiaPreferences* preferences )
{
    caf::PdmObjectHandle* pdmClone =
        preferences->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() );

    return std::unique_ptr<RiaPreferences>( dynamic_cast<RiaPreferences*>( pdmClone ) );
}

#include "RicNewValveFeature.h"
#include "Riu3DMainWindowTools.h"

#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathValve.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewValveFeature, "RicNewValveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewValveFeature::isCommandEnabled() const
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 1u ) return false;

    const RimPerforationInterval* perfInterval = dynamic_cast<RimPerforationInterval*>( selectedItems.front() );
    return perfInterval != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveFeature::onActionTriggered( bool isChecked )
{
    RimPerforationInterval* perfInterval = caf::SelectionManager::instance()->selectedItemOfType<RimPerforationInterval>();
    if ( perfInterval )
    {
        RimWellPathValve* valve = new RimWellPathValve;

        RimProject* project = RimProject::current();

        std::vector<RimWellPathValve*> existingValves = perfInterval->valves();
        valve->setName( QString( "Valve #%1" ).arg( existingValves.size() + 1 ) );

        std::vector<RimValveTemplate*> allValveTemplates = project->allValveTemplates();
        if ( !allValveTemplates.empty() )
        {
            valve->setValveTemplate( allValveTemplates.front() );
        }

        perfInterval->addValve( valve );
        valve->setMeasuredDepthAndCount( perfInterval->startMD(), perfInterval->endMD() - perfInterval->startMD(), 1 );

        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( !wellPathCollection ) return;

        wellPathCollection->uiCapability()->updateConnectedEditors();
        wellPathCollection->scheduleRedrawAffectedViews();

        Riu3DMainWindowTools::selectAsCurrentItem( valve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ICDValve16x16.png" ) );
    actionToSetup->setText( "Create Valve" );
}

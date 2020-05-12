#include "RicNewValveFeature.h"
#include "Riu3DMainWindowTools.h"

#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
#include "RimWellPathValve.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewValveFeature, "RicNewValveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewValveFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> allSelectedItems;
    caf::SelectionManager::instance()->selectedItems( allSelectedItems );
    if ( allSelectedItems.size() != 1u ) return false;

    const RimPerforationInterval* perfInterval = dynamic_cast<RimPerforationInterval*>( allSelectedItems.front() );
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

        RimProject* project = nullptr;
        perfInterval->firstAncestorOrThisOfTypeAsserted( project );

        std::vector<RimWellPathValve*> existingValves = perfInterval->valves();
        valve->setName( QString( "Valve #%1" ).arg( existingValves.size() + 1 ) );

        std::vector<RimValveTemplate*> allValveTemplates = project->allValveTemplates();
        if ( !allValveTemplates.empty() )
        {
            valve->setValveTemplate( allValveTemplates.front() );
        }

        perfInterval->addValve( valve );
        valve->setMeasuredDepthAndCount( perfInterval->startMD(), perfInterval->endMD() - perfInterval->startMD(), 1 );

        RimWellPathCollection* wellPathCollection = nullptr;
        perfInterval->firstAncestorOrThisOfType( wellPathCollection );
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

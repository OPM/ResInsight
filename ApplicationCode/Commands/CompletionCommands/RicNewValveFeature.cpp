#include "RicNewValveFeature.h"
#include "Riu3DMainWindowTools.h"

#include "RimPerforationInterval.h"
#include "RimWellPathValve.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewValveFeature, "RicNewValveFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewValveFeature::isCommandEnabled()
{
    const RimPerforationInterval* perfInterval = caf::SelectionManager::instance()->selectedItemOfType<RimPerforationInterval>();
    return perfInterval != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveFeature::onActionTriggered(bool isChecked)
{
    RimPerforationInterval* perfInterval = caf::SelectionManager::instance()->selectedItemOfType<RimPerforationInterval>();
    if (perfInterval)
    {
        RimWellPathValve* valve = new RimWellPathValve;
        perfInterval->addValve(valve);
        valve->setMeasuredDepthAndCount(perfInterval->startMD(), perfInterval->endMD() - perfInterval->startMD(), 1);

        RimWellPathCollection* wellPathCollection = nullptr;
        perfInterval->firstAncestorOrThisOfType(wellPathCollection);
        if (!wellPathCollection) return;

        wellPathCollection->uiCapability()->updateConnectedEditors();
        wellPathCollection->scheduleRedrawAffectedViews();

        Riu3DMainWindowTools::selectAsCurrentItem(valve);

    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/ICDValve16x16.png"));
    actionToSetup->setText("New Valve");
}

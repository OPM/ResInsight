#include "RicNewValveFeature.h"
#include "Riu3DMainWindowTools.h"

#include "Well/RigWellPath.h"

#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimValveCollection.h"
#include "RimWellPath.h"
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

    if ( dynamic_cast<RimPerforationInterval*>( selectedItems.front() ) ) return true;

    if ( dynamic_cast<RimValveCollection*>( selectedItems.front() ) ) return true;

    if ( dynamic_cast<RimWellPath*>( selectedItems.front() ) ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve* RicNewValveFeature::createValveForPerforation( RimPerforationInterval* perfInterval )
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

    return valve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicNewValveFeature::wellPathAvgMd( const RimWellPath* wellPath ) const
{
    double valveMd = 0.0;
    if ( ( wellPath != nullptr ) && ( wellPath->wellPathGeometry() != nullptr ) )
    {
        const auto& mds = wellPath->wellPathGeometry()->measuredDepths();
        if ( !mds.empty() )
        {
            valveMd = ( mds.front() + mds.back() ) / 2.0;
        }
    }
    return valveMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveFeature::onActionTriggered( bool isChecked )
{
    RimWellPathValve* valve = nullptr;

    if ( RimPerforationInterval* perfInterval = caf::SelectionManager::instance()->selectedItemOfType<RimPerforationInterval>() )
    {
        valve = createValveForPerforation( perfInterval );
    }
    else if ( RimValveCollection* valveColl = caf::SelectionManager::instance()->selectedItemOfType<RimValveCollection>() )
    {
        auto wellPath = valveColl->firstAncestorOrThisOfType<RimWellPath>();
        auto valveMd  = wellPathAvgMd( wellPath );

        valve = valveColl->addIcvValve( valveMd );
    }
    else if ( RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>() )
    {
        auto valveMd = wellPathAvgMd( wellPath );
        valve        = wellPath->valveCollection()->addIcvValve( valveMd );
    }

    if ( valve )
    {
        if ( auto wellPathCollection = RimTools::wellPathCollection() )
        {
            wellPathCollection->updateConnectedEditors();
            wellPathCollection->scheduleRedrawAffectedViews();
        }
        Riu3DMainWindowTools::selectAsCurrentItem( valve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ICVValve16x16.png" ) );
    actionToSetup->setText( "Create Valve" );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicFeatureExecutionRunner.h"

#include "RiaFeatureTestModelBuilder.h"
#include "RiaViewRedrawScheduler.h"
#include "RicFeatureSweepDenylist.h"

#include "RiaApplication.h"

#include "Polygons/RimPolygon.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimWellPath.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafFactory.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QApplication>
#include <QTimer>
#include <QWidget>

#include <iostream>
#include <set>
#include <vector>

namespace
{
// At most this many selections are executed per feature. A feature that is enabled for a whole class
// of objects would otherwise run dozens of times, which costs runtime and multiplies destructive side
// effects without finding much more. Hitting the cap is logged rather than passing silently.
constexpr size_t maxExecutionsPerFeature = 3;

//--------------------------------------------------------------------------------------------------
/// Closes any modal dialog that appears while a feature is executing.
///
/// A feature that opens a modal dialog is treated as a success (it ran up to the dialog without
/// crashing); the dialog is simply dismissed so the run continues.
//--------------------------------------------------------------------------------------------------
class ModalDialogWatchdog
{
public:
    ModalDialogWatchdog()
    {
        m_timer.setInterval( 200 );
        QObject::connect( &m_timer,
                          &QTimer::timeout,
                          []()
                          {
                              if ( QWidget* modal = QApplication::activeModalWidget() )
                              {
                                  modal->close();
                              }
                          } );
        m_timer.start();
    }

    ~ModalDialogWatchdog() { m_timer.stop(); }

private:
    QTimer m_timer;
};

// The combined model is reused across features to avoid the expensive Eclipse rebuild. It is only
// rebuilt when a previously executed feature broke it: closed the project or deleted one of the
// objects the scenarios select (detected via the dangling-safe caf::PdmPointer guards).
FeatureTestModel                      s_model;
caf::PdmPointer<caf::PdmObjectHandle> s_eclipseCaseGuard;
caf::PdmPointer<caf::PdmObjectHandle> s_eclipseViewGuard;
caf::PdmPointer<caf::PdmObjectHandle> s_wellPathGuard;
bool                                  s_modelBuilt = false;

void collectSelectionCandidates();

void rebuildModel()
{
    caf::SelectionManager::instance()->clearAll();
    s_model = RiaFeatureTestModelBuilder::richModel();

    s_eclipseCaseGuard = s_model.eclipseCase;
    s_eclipseViewGuard = s_model.eclipseView;
    s_wellPathGuard    = s_model.wellPath;
    s_modelBuilt       = true;

    collectSelectionCandidates();
}

bool modelNeedsRebuild()
{
    if ( !s_modelBuilt ) return true;
    if ( RimProject::current() == nullptr ) return true;
    if ( s_eclipseCaseGuard.isNull() ) return true;
    if ( s_eclipseViewGuard.isNull() ) return true;
    if ( s_wellPathGuard.isNull() ) return true;
    return false;
}

// One object per distinct class in the project, held through dangling-safe pointers so a feature that
// deletes objects does not leave the list pointing at freed memory.
std::vector<caf::PdmPointer<caf::PdmObjectHandle>> s_candidates;

//--------------------------------------------------------------------------------------------------
/// Collect the objects to use as selections, by walking the project.
///
/// Command features dispatch on the type of the selected object, so one instance per class is enough
/// and keeps the list to a size that can be tried exhaustively for every feature. Walking the tree
/// rather than hand-picking a few objects is what reaches the collections, sub-collections and
/// definition objects that most features are actually written against.
//--------------------------------------------------------------------------------------------------
void collectSelectionCandidates()
{
    s_candidates.clear();

    RimProject* project = RimProject::current();
    if ( !project ) return;

    std::set<QString> seenClasses;
    for ( caf::PdmObjectHandle* object : project->descendantsIncludingThisOfType<caf::PdmObjectHandle>() )
    {
        if ( !object ) continue;

        // Ask for the capabilities directly. The uiCapability() and xmlCapability() accessors assert
        // when the capability is missing rather than returning null, so they cannot be used to test
        // for it, and not every object in the project has both.
        auto* xmlCapability = object->capability<caf::PdmXmlObjectHandle>();
        auto* uiCapability  = object->capability<caf::PdmUiObjectHandle>();
        if ( !xmlCapability || !uiCapability ) continue;

        if ( seenClasses.insert( xmlCapability->classKeyword() ).second )
        {
            s_candidates.push_back( object );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Select a candidate object, and make the view it belongs to the active one.
///
/// Many features read the active view instead of the selection, so the two are kept consistent:
/// selecting something inside the GeoMech view must not leave the Eclipse view active.
//--------------------------------------------------------------------------------------------------
bool applySelection( caf::PdmObjectHandle* object )
{
    caf::SelectionManager::instance()->clearAll();

    if ( !object ) return false;

    auto* uiCapability = object->capability<caf::PdmUiObjectHandle>();
    if ( !uiCapability ) return false;

    RimGridView* activeView = object->firstAncestorOrThisOfType<RimGridView>();
    if ( !activeView ) activeView = s_model.eclipseView;

    RiaApplication::instance()->setActiveReservoirView( activeView );
    caf::SelectionManager::instance()->setSelectedItem( uiCapability );
    return true;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicFeatureExecutionRunner::executeFeature( const std::string& commandId )
{
    if ( isFeatureDenylisted( commandId ) ) return true;

    caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( commandId );
    if ( !feature ) return false;

    if ( modelNeedsRebuild() ) rebuildModel();

    size_t executionCount = 0;

    for ( size_t i = 0; i < s_candidates.size(); ++i )
    {
        // A previously executed feature may have deleted objects or closed the project. Rebuilding
        // invalidates the candidate list, so rebuild it too and restart from the same index.
        if ( modelNeedsRebuild() ) rebuildModel();

        caf::PdmObjectHandle* candidate = s_candidates[i].p();
        if ( !candidate ) continue;

        auto* candidateXml = candidate->capability<caf::PdmXmlObjectHandle>();
        if ( !candidateXml ) continue;
        const QString candidateName = candidateXml->classKeyword();

        if ( !applySelection( candidate ) ) continue;

        // The enabled state depends on the selection just applied. Note this call is itself part of
        // what is being tested: a feature whose isCommandEnabled() crashes is a defect too.
        if ( !feature->canFeatureBeExecuted() ) continue;

        if ( executionCount >= maxExecutionsPerFeature )
        {
            std::cout << "[capped] " << commandId << " is enabled for more than " << maxExecutionsPerFeature
                      << " object types; remaining ones are not executed" << std::endl;
            break;
        }
        ++executionCount;

        // Attribution line: printed and flushed before execution so a hard crash in a child process
        // still identifies the culprit in the captured output.
        std::cout << "[exec] " << candidateName.toStdString() << " : " << commandId << std::endl;

        ModalDialogWatchdog watchdog;

        // Trigger through the QAction rather than calling actionTriggered() directly. Features that
        // read caf::CmdFeature::userData() get it from qobject_cast<QAction*>( sender() ), which
        // asserts when the slot is invoked as a plain function call. Going through the action is also
        // how the application itself invokes a feature.
        if ( QAction* action = feature->action() )
        {
            action->trigger();
        }
        else
        {
            feature->actionTriggered( false );
        }

        // Deliberately do NOT pump the event loop here.
        //
        // RiaGuiApplication::initialize() creates and shows the main window, so the 3D viewer widgets
        // exist and Qt has paint events queued for them. Offscreen there is no OpenGL context, and
        // caf::Viewer::paintGL() dereferences the null context, which shows up as an access violation
        // blamed on whichever feature happened to run. Measured: with processEvents() the cell-filter
        // features all crash; without it they pass. Discarding the scheduled redraws is not enough,
        // because the paint events come from Qt rather than from RiaViewRedrawScheduler.
        //
        // Exercising real rendering requires the software-GL tier, not this one, so drop the pending
        // redraws and leave the event loop alone.
        RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();

        // Force a rebuild before the next selection, so every execution starts from an identical
        // model. Without this, a destructive feature leaves debris that makes a later execution fail
        // for reasons that have nothing to do with the object then selected: deleting an intersection
        // result definition and then a GeoMech view aborts, while deleting the view on a clean model
        // is fine. Order dependent failures like that are false reports, and the guards below cannot
        // catch them because they only track the few objects they name.
        s_modelBuilt = false;

        if ( RimProject::current() == nullptr ) return false;
    }

    // Phase marker: everything above is feature execution, everything after is harness teardown. A
    // child that crashes without printing this crashed inside the feature; one that crashes after it
    // crashed while closing the project, which is a harness problem and not a feature defect.
    std::cout << executionCompleteMarker << " " << commandId << std::endl;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RicFeatureExecutionRunner::allRegisteredFeatureIds()
{
    return caf::Factory<caf::CmdFeature, std::string>::instance()->allKeys();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFeatureExecutionRunner::releaseModel()
{
    caf::SelectionManager::instance()->clearAll();
    RiaFeatureTestModelBuilder::closeProject();
    s_modelBuilt = false;
}

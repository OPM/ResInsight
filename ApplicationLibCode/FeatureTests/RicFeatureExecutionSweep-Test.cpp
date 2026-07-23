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

#include "gtest/gtest.h"

#include "RiaFeatureTestModelBuilder.h"
#include "RicFeatureSweepDenylist.h"

#include "RimEclipseCase.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafFactory.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QApplication>
#include <QTimer>
#include <QWidget>

#include <string>
#include <vector>

namespace
{
enum class ScenarioKind
{
    EMPTY,
    ECLIPSE_CASE,
    ECLIPSE_VIEW,
    WELL_PATH
};

struct Scenario
{
    const char*  name;
    ScenarioKind kind;
};

const std::vector<Scenario>& executionScenarios()
{
    // Only populated scenarios are executed. Executing a feature with an empty selection is not
    // meaningful and mostly trips preconditions (see the file header note on asserts).
    static const std::vector<Scenario> scenarios = {
        { "EclipseCase", ScenarioKind::ECLIPSE_CASE },
        { "EclipseView", ScenarioKind::ECLIPSE_VIEW },
        { "WellPath", ScenarioKind::WELL_PATH },
    };
    return scenarios;
}

std::vector<std::string> allRegisteredFeatureIds()
{
    return caf::Factory<caf::CmdFeature, std::string>::instance()->allKeys();
}

//--------------------------------------------------------------------------------------------------
/// Closes any modal dialog that appears while a feature is executing.
///
/// A feature that opens a modal dialog is treated as a success (it ran up to the dialog without
/// crashing); the dialog is simply dismissed so the run continues. This keeps the denylist focused
/// on features that block in ways a dialog-close cannot recover from (external processes, network,
/// process termination).
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

} // namespace

//--------------------------------------------------------------------------------------------------
/// Executes each feature under the scenarios where it reports itself enabled, asserting that
/// execution does not crash and leaves a valid project.
///
/// One parameterized case per feature isolates a crash to the offending feature id. A single
/// combined model (Eclipse case + view + well path) is shared across the whole suite and only
/// rebuilt after a feature deletes one of the selected objects. Enabled-state checks are read-only
/// and reuse the shared model. Denylisted features are never executed.
///
/// DISABLED by default. Many features assume the framework only invokes them in a valid GUI
/// selection context (e.g. a right-click on a specific object). Executing them with a synthetic
/// selection can violate that assumption and trip a CVF_ASSERT / CAF_ASSERT. Those asserts abort the
/// process (console mode) rather than throw, so a single offender ends the whole run and cannot be
/// isolated the way an access violation can. Making this sweep a reliable, always-green gate would
/// require running each feature in its own subprocess (so an abort/hang only kills that feature's
/// process). Until then it is opt-in: run it with
///   --gtest_also_run_disabled_tests --gtest_filter=*FeatureExecutionTest*
/// to harden the denylist, and rely on the curated *-Test.cpp files for asserted execution coverage.
//--------------------------------------------------------------------------------------------------
class FeatureExecutionTest : public ::testing::TestWithParam<std::string>
{
public:
    static void SetUpTestSuite() { rebuildModel(); }

    static void TearDownTestSuite()
    {
        caf::SelectionManager::instance()->clearAll();
        RiaFeatureTestModelBuilder::closeProject();
    }

protected:
    static void rebuildModel()
    {
        caf::SelectionManager::instance()->clearAll();
        s_model = RiaFeatureTestModelBuilder::combinedModel();

        s_eclipseCaseGuard = s_model.eclipseCase;
        s_eclipseViewGuard = s_model.eclipseView;
        s_wellPathGuard    = s_model.wellPath;
    }

    // The combined model is reused across features to avoid the expensive Eclipse rebuild. A rebuild
    // is only needed when a previously executed feature broke it: closed the project or deleted one
    // of the objects the scenarios select (detected via the dangling-safe caf::PdmPointer guards).
    // Additive features (the majority) leave the base model intact and require no rebuild.
    static bool modelNeedsRebuild()
    {
        if ( RimProject::current() == nullptr ) return true;
        if ( s_eclipseCaseGuard.isNull() ) return true;
        if ( s_eclipseViewGuard.isNull() ) return true;
        if ( s_wellPathGuard.isNull() ) return true;
        return false;
    }

    // Select the objects for a scenario. Returns false if the scenario cannot be represented (e.g.
    // the model failed to build), in which case the caller skips it.
    static bool applySelection( ScenarioKind kind )
    {
        caf::SelectionManager::instance()->clearAll();

        caf::PdmUiItem* item = nullptr;
        switch ( kind )
        {
            case ScenarioKind::EMPTY:
                return true;
            case ScenarioKind::ECLIPSE_CASE:
                item = s_model.eclipseCase;
                break;
            case ScenarioKind::ECLIPSE_VIEW:
                item = s_model.eclipseView;
                break;
            case ScenarioKind::WELL_PATH:
                item = s_model.wellPath;
                break;
        }

        if ( !item ) return false;

        caf::SelectionManager::instance()->setSelectedItem( item );
        return true;
    }

    static FeatureTestModel                      s_model;
    static caf::PdmPointer<caf::PdmObjectHandle> s_eclipseCaseGuard;
    static caf::PdmPointer<caf::PdmObjectHandle> s_eclipseViewGuard;
    static caf::PdmPointer<caf::PdmObjectHandle> s_wellPathGuard;
};

FeatureTestModel                      FeatureExecutionTest::s_model;
caf::PdmPointer<caf::PdmObjectHandle> FeatureExecutionTest::s_eclipseCaseGuard;
caf::PdmPointer<caf::PdmObjectHandle> FeatureExecutionTest::s_eclipseViewGuard;
caf::PdmPointer<caf::PdmObjectHandle> FeatureExecutionTest::s_wellPathGuard;

TEST_P( FeatureExecutionTest, DISABLED_ExecuteWhenEnabledDoesNotCrash )
{
    const std::string& commandId = GetParam();

    if ( isFeatureDenylisted( commandId ) )
    {
        GTEST_SKIP() << "Denylisted: " << featureSweepDenylist().at( commandId );
    }

    for ( const Scenario& scenario : executionScenarios() )
    {
        SCOPED_TRACE( std::string( "scenario: " ) + scenario.name );

        if ( modelNeedsRebuild() ) rebuildModel();

        if ( !applySelection( scenario.kind ) ) continue;

        caf::CmdFeature* feature = caf::CmdFeatureManager::instance()->getCommandFeature( commandId );
        ASSERT_TRUE( feature != nullptr );

        if ( !feature->canFeatureBeExecuted() ) continue;

        // Attribution line: printed before execution so a hard crash identifies the culprit.
        std::cout << "[exec] " << scenario.name << " : " << commandId << std::endl;

        ModalDialogWatchdog watchdog;
        feature->actionTriggered( false );
        QApplication::processEvents();

        EXPECT_TRUE( RimProject::current() != nullptr );
    }
}

INSTANTIATE_TEST_SUITE_P( AllFeatures,
                          FeatureExecutionTest,
                          ::testing::ValuesIn( allRegisteredFeatureIds() ),
                          []( const testing::TestParamInfo<std::string>& info ) { return info.param; } );

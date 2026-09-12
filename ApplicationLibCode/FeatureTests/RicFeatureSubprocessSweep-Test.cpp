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

#include "RicFeatureExecutionRunner.h"
#include "RicFeatureSweepDenylist.h"

#include "cafCmdFeature.h"
#include "cafFactory.h"

#include <QCoreApplication>
#include <QProcess>
#include <QString>
#include <QStringList>

#include <string>
#include <vector>

namespace
{
// Generous enough for a feature that loads the mock Eclipse model in a cold process, short enough
// that a genuinely hung feature does not stall the sweep.
constexpr int featureTimeoutMs = 120000;
} // namespace

//--------------------------------------------------------------------------------------------------
/// Executes every feature in its own child process, so a feature that aborts or hangs is isolated
/// and attributed to its own test case.
///
/// The in-process sweep (RicFeatureExecutionSweep-Test.cpp) cannot be a gate: features that assume a
/// valid GUI selection context trip CVF_ASSERT/CAF_ASSERT, which abort the process rather than throw,
/// so the first offender ends the whole run and hides every feature after it. Here the parent re-runs
/// this same executable with --feature-exec=<id> (see main.cpp), which executes exactly one feature
/// and exits. The parent then classifies the child:
///
/// - exit code 0                -> feature ran (or was never enabled / is denylisted)
/// - non-zero exit or crash     -> the feature aborted or crashed
/// - no exit within the timeout -> the feature hung (blocking dialog, external process, network)
///
/// DISABLED by default: one cold process per feature is far too slow for the fast suite. Run it
/// deliberately, e.g. in a nightly job:
///   ResInsight-featuretests --gtest_also_run_disabled_tests --gtest_filter=*FeatureSubprocess*
/// A failure names the offending feature id, which is either a real bug to fix or a new denylist
/// entry with a documented reason.
//--------------------------------------------------------------------------------------------------
class FeatureSubprocessExecutionTest : public ::testing::TestWithParam<std::string>
{
};

TEST_P( FeatureSubprocessExecutionTest, DISABLED_ExecuteInIsolationDoesNotCrash )
{
    const std::string& commandId = GetParam();

    if ( isFeatureDenylisted( commandId ) )
    {
        GTEST_SKIP() << "Denylisted: " << featureSweepDenylist().at( commandId );
    }

    const QString exePath = QCoreApplication::applicationFilePath();
    ASSERT_FALSE( exePath.isEmpty() );

    QProcess    child;
    QStringList arguments;
    arguments << QString::fromStdString( RicFeatureExecutionRunner::featureExecSwitch + commandId );

    child.setProcessChannelMode( QProcess::MergedChannels );
    child.start( exePath, arguments );

    ASSERT_TRUE( child.waitForStarted( 30000 ) ) << "Could not start child process for " << commandId;

    if ( !child.waitForFinished( featureTimeoutMs ) )
    {
        child.kill();
        child.waitForFinished( 5000 );
        FAIL() << commandId << " did not finish within " << featureTimeoutMs << " ms (hung).\n"
               << "Output:\n"
               << child.readAll().toStdString();
    }

    const QString output = QString::fromUtf8( child.readAll() );

    // The marker separates feature execution from harness teardown, so a crash is attributed to the
    // right place instead of blaming the feature for a teardown problem.
    const bool featureExecutionCompleted = output.contains( RicFeatureExecutionRunner::executionCompleteMarker );

    ASSERT_EQ( QProcess::NormalExit, child.exitStatus() )
        << commandId
        << ( featureExecutionCompleted ? " crashed during harness teardown (not a feature defect)" : " crashed while executing the feature" )
        << ".\nOutput:\n"
        << output.toStdString();

    EXPECT_EQ( 0, child.exitCode() ) << commandId << " exited with a failure code.\nOutput:\n" << output.toStdString();
}

INSTANTIATE_TEST_SUITE_P( AllFeatures,
                          FeatureSubprocessExecutionTest,
                          ::testing::ValuesIn( RicFeatureExecutionRunner::allRegisteredFeatureIds() ),
                          []( const testing::TestParamInfo<std::string>& info ) { return info.param; } );

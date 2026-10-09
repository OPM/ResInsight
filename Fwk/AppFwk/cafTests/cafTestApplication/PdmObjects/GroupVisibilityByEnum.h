#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
/// Reproduction of https://github.com/OPM/ResInsight/issues/14884
///
/// Modeled directly after RiaPreferencesHpc::appendItems() (branch origin/hpc_interface), which
/// crashed in caf::PdmUiCheckBoxEditor::configureAndUpdateUi() when switching the "Batch Scheduler"
/// enum directly between SLURM and LSF: both options share the exact same three fields
/// (queue name, exclusive checkbox, scheduler options) inside two *differently named* ui groups.
///
/// The critical detail is that the shared fields must move directly from one group to the other,
/// without ever passing through an intermediate state where they are absent from the ui ordering.
/// If they pass through an absent state first, the field editors are cleanly destroyed and
/// recreated (safe). Going directly from one group to the other instead reuses/reparents the live
/// field editor widgets from the old (about-to-be-deleted) group box into the newly created one.
///
/// Steps to reproduce:
///   1. Select this object in the tree view so the property editor is shown.
///   2. Change "Mode" from "Local" to "Slurm" (no crash expected -- fresh group/fields created).
///   3. Change "Mode" directly from "Slurm" to "LSF" (skip "Local"). This reparents the shared
///      queue name / exclusive / options fields from the "Slurm Options" group into a brand new
///      "LSF Options" group within the same ui rebuild. This is the step that reproduced the
///      crash in PdmUiCheckBoxEditor::configureAndUpdateUi().
///   4. Switch back and forth directly between "Slurm" and "LSF" a few times if step 3 doesn't
///      crash immediately.
//==================================================================================================
class GroupVisibilityByEnum : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SchedulerMode
    {
        LOCAL,
        SLURM,
        LSF
    };

public:
    GroupVisibilityByEnum();

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<caf::AppEnum<SchedulerMode>> m_mode;

    // Local-only member
    caf::PdmField<int> m_maxParallelJobs;

    // Shared between the Slurm Options group and the LSF Options group.
    caf::PdmField<QString> m_queueName;
    caf::PdmField<bool>    m_exclusive;
    caf::PdmField<QString> m_schedulerOptions;
};

#include "GroupVisibilityByEnum.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiOrdering.h"

template <>
void caf::AppEnum<GroupVisibilityByEnum::SchedulerMode>::setUp()
{
    addItem( GroupVisibilityByEnum::SchedulerMode::LOCAL, "LOCAL", "Local" );
    addItem( GroupVisibilityByEnum::SchedulerMode::SLURM, "SLURM", "Slurm" );
    addItem( GroupVisibilityByEnum::SchedulerMode::LSF, "LSF", "LSF" );

    setDefault( GroupVisibilityByEnum::SchedulerMode::LOCAL );
}

CAF_PDM_SOURCE_INIT( GroupVisibilityByEnum, "GroupVisibilityByEnum" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GroupVisibilityByEnum::GroupVisibilityByEnum()
{
    CAF_PDM_InitObject( "Group Visibility By Enum", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_mode, "Mode", "Mode" );

    CAF_PDM_InitField( &m_maxParallelJobs, "MaxParallelJobs", 1, "Max Parallel Jobs", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_queueName, "QueueName", "Queue Name" );
    m_queueName = "default";

    CAF_PDM_InitField( &m_exclusive, "Exclusive", false, "Exclusive", "", "", "" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_exclusive );

    CAF_PDM_InitFieldNoDefault( &m_schedulerOptions, "SchedulerOptions", "Scheduler Options" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GroupVisibilityByEnum::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGroup = uiOrdering.addNewGroup( "General Settings" );
    genGroup->add( &m_mode );

    if ( m_mode() == SchedulerMode::LOCAL )
    {
        auto localGroup = uiOrdering.addNewGroup( "Local Options" );
        localGroup->add( &m_maxParallelJobs );
    }
    else if ( m_mode() == SchedulerMode::SLURM )
    {
        auto slurmGroup = uiOrdering.addNewGroup( "Slurm Options" );
        slurmGroup->add( &m_queueName );
        slurmGroup->add( &m_exclusive );
        slurmGroup->add( &m_schedulerOptions );
    }
    else if ( m_mode() == SchedulerMode::LSF )
    {
        auto lsfGroup = uiOrdering.addNewGroup( "LSF Options" );
        lsfGroup->add( &m_queueName );
        lsfGroup->add( &m_exclusive );
        lsfGroup->add( &m_schedulerOptions );
    }
}

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

#include "RiaPreferencesHpc.h"

#include "RiaApplication.h"
#include "RiaHpcTools.h"
#include "RiaPreferences.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"

#include <QInputDialog>

CAF_PDM_SOURCE_INIT( RiaPreferencesHpc, "RiaPreferencesHpc" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesHpc::RiaPreferencesHpc()
{
    CAF_PDM_InitFieldNoDefault( &m_maxParallelJobs, "maxParallelJobs", "Maximum number of jobs to run in parallel on local machine" );
    m_maxParallelJobs = 1;
    m_maxParallelJobs.setRange( 1, 100 );

    CAF_PDM_InitFieldNoDefault( &m_batchScheduler, "batchScheduler", "Batch Scheduler to use for submitting jobs" );
    CAF_PDM_InitField( &m_exclusive, "exclusive", false, "Exclusive - do not share assigned nodes with other jobs." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_exclusive );

    CAF_PDM_InitFieldNoDefault( &m_queueName, "queueName", "Default queue/partition name" );

    CAF_PDM_InitFieldNoDefault( &m_batchSchedulerOptions, "batchSchedulerOptions", "Optional command line arguments" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPreferencesHpc* RiaPreferencesHpc::current()
{
    return RiaApplication::instance()->preferences()->hpcPreferences();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesHpc::appendItems( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* genGrp = uiOrdering.addNewGroup( "General Settings" );
    genGrp->add( &m_batchScheduler );

    if ( m_batchScheduler() == RiaDefines::BatchSchedulerType::LOCAL_COMPUTER )
    {
        auto localGrp = uiOrdering.addNewGroup( "Local Computer Options" );
        localGrp->add( &m_maxParallelJobs );
    }
    else
    {
        auto hpcGrp = uiOrdering.addNewGroup( "Batch Scheduler Options" );
        hpcGrp->add( &m_queueName );

        hpcGrp->addNewButton( "Browse",
                              [this]()
                              {
                                  auto newName = RiaPreferencesHpc::selectQueue( m_batchScheduler(), m_queueName() );
                                  m_queueName.setValueWithFieldChanged( newName );
                              },
                              { .newRow = false, .totalColumnSpan = 1 } );
        hpcGrp->add( &m_exclusive );
        hpcGrp->add( &m_batchSchedulerOptions );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPreferencesHpc::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_queueName )
    {
        auto attr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->enableEditableContent  = true;
            attr->enableAutoComplete     = false;
            attr->adjustWidthToContents  = true;
            attr->notifyWhenTextIsEdited = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaPreferencesHpc::maxParallelJobs() const
{
    return m_maxParallelJobs();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::BatchSchedulerType RiaPreferencesHpc::batchScheduler() const
{
    return m_batchScheduler();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesHpc::queueName() const
{
    return m_queueName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesHpc::batchSchedulerOptions() const
{
    return m_batchSchedulerOptions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaPreferencesHpc::exclusiveJob() const
{
    return m_exclusive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaPreferencesHpc::selectQueue( RiaDefines::BatchSchedulerType scheduler, QString currentQueue )
{
    QStringList queues    = RiaHpcTools::availableQueues( scheduler );
    QString     labelText = "Available Queues/Partitions:";
    if ( queues.isEmpty() )
    {
        labelText = "Warning: failed to get available queues/partitions.";
        queues.append( currentQueue );
    }

    int currentQueueIndex = currentQueue.isEmpty() ? 0 : queues.indexOf( currentQueue );

    bool    ok;
    QString selectedQueue = QInputDialog::getItem( nullptr, "Select Queue/Partition", labelText, queues, currentQueueIndex, false, &ok );
    if ( ok && !selectedQueue.isEmpty() )
    {
        return selectedQueue;
    }
    return currentQueue;
}

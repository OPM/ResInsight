/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026    Equinor ASA
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

#include "RimBatchProcessMonitor.h"

#include "RimBatchQueue.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchProcessMonitor::RimBatchProcessMonitor( RimBatchQueue* batchQueue )
    : m_batchQueue( batchQueue )
    , RimProcessMonitor( 0, true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimBatchProcessMonitor::~RimBatchProcessMonitor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchProcessMonitor::finished( int exitCode, QProcess::ExitStatus exitStatus )
{
    if ( m_batchQueue.notNull() )
    {
        m_batchQueue->setFinished( exitStatus == QProcess::NormalExit && exitCode == 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimBatchProcessMonitor::started()
{
    // if ( m_batchQueue.notNull() )
    //{
    //     m_batchQueue->setStarted();
    // }

    RimProcessMonitor::started();
}

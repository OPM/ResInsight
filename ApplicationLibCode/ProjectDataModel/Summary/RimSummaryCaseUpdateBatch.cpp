/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimSummaryCaseUpdateBatch.h"

#include "RimDeltaSummaryCase.h"
#include "RimDeltaSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"

#include "cafPdmObjectHandleTools.h"

#include <algorithm>
#include <set>

RimSummaryCaseUpdateBatch* RimSummaryCaseUpdateBatch::sm_current = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseUpdateBatch::RimSummaryCaseUpdateBatch()
{
    // A nested batch contributes to the outermost one and never flushes
    if ( !sm_current ) sm_current = this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseUpdateBatch::~RimSummaryCaseUpdateBatch()
{
    if ( sm_current != this ) return;

    // Clear the ambient batch before flushing. Work triggered by the flush itself is executed immediately, which is
    // safe here, as the outermost scope is ending and no caller is holding a case list across this point.
    sm_current = nullptr;

    flush();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseUpdateBatch::isActive()
{
    return sm_current != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseUpdateBatch::orphan( const std::vector<RimDeltaSummaryCase*>& orphanedCases )
{
    if ( orphanedCases.empty() ) return;

    if ( sm_current )
    {
        sm_current->m_orphanedCases.insert( sm_current->m_orphanedCases.end(), orphanedCases.begin(), orphanedCases.end() );
        return;
    }

    auto casesToDelete = orphanedCases;
    caf::PdmObjectHandleTools::deleteObjects( casesToDelete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseUpdateBatch::markDeltaEnsembleDirty( RimDeltaSummaryEnsemble* deltaEnsemble )
{
    if ( !deltaEnsemble ) return;

    if ( sm_current )
    {
        auto& dirtyEnsembles = sm_current->m_dirtyEnsembles;

        auto isSameEnsemble = [deltaEnsemble]( const caf::PdmPointer<RimDeltaSummaryEnsemble>& candidate )
        { return candidate.p() == deltaEnsemble; };

        if ( std::none_of( dirtyEnsembles.begin(), dirtyEnsembles.end(), isSameEnsemble ) )
        {
            dirtyEnsembles.push_back( deltaEnsemble );
        }
        return;
    }

    deltaEnsemble->onSourceEnsembleChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimDeltaSummaryEnsemble*> RimSummaryCaseUpdateBatch::ensemblesToRegenerate() const
{
    std::vector<RimSummaryEnsemble*> dirtyEnsembles;
    for ( const auto& dirtyEnsemble : m_dirtyEnsembles )
    {
        // An ensemble marked dirty can have been deleted before the flush
        if ( dirtyEnsemble.notNull() ) dirtyEnsembles.push_back( dirtyEnsemble.p() );
    }

    // Everything depending on the dirty ensembles, in dependency order. A dirty ensemble depending on another dirty
    // ensemble shows up here, and must be regenerated in this order rather than as a root.
    auto dependents = RimSummaryEnsembleTools::deltaEnsemblesInUpdateOrder( dirtyEnsembles );

    std::set<const RimSummaryEnsemble*> alreadyOrdered( dependents.begin(), dependents.end() );

    std::vector<RimDeltaSummaryEnsemble*> ordered;
    for ( const auto& dirtyEnsemble : m_dirtyEnsembles )
    {
        if ( dirtyEnsemble.isNull() ) continue;
        if ( !alreadyOrdered.insert( dirtyEnsemble.p() ).second ) continue;

        ordered.push_back( dirtyEnsemble.p() );
    }

    ordered.insert( ordered.end(), dependents.begin(), dependents.end() );

    return ordered;
}

//--------------------------------------------------------------------------------------------------
/// Regenerate before destroying, so a chained delta ensemble sees the final state of its source
/// before anything is freed.
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseUpdateBatch::flush()
{
    for ( auto deltaEnsemble : ensemblesToRegenerate() )
    {
        deltaEnsemble->onSourceEnsembleChanged();
    }
    m_dirtyEnsembles.clear();

    std::vector<RimDeltaSummaryCase*> casesToDelete;
    for ( const auto& orphanedCase : m_orphanedCases )
    {
        // An orphaned case can already have been destroyed by the code that detached it
        if ( orphanedCase.notNull() ) casesToDelete.push_back( orphanedCase.p() );
    }
    m_orphanedCases.clear();

    caf::PdmObjectHandleTools::deleteObjects( casesToDelete );
}

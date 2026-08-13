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

#pragma once

#include "cafPdmPointer.h"

#include <vector>

class RimDeltaSummaryCase;
class RimDeltaSummaryEnsemble;

//==================================================================================================
/// Collects the work that must happen once a set of summary case mutations has settled. Removing a
/// case only detaches it and hands it to the batch, destruction happens when the batch flushes.
///
/// This is a plain stack object, ambient for the duration of its scope. A nested batch contributes to
/// the outermost one, which is the only one that flushes. Open a batch wherever a case list is held
/// across a mutation, so no object in that list is freed while the list is still in use.
///
/// The contribution points fall back to immediate execution when no batch is active, so call sites
/// that do not open one keep behaving as before.
//==================================================================================================
class RimSummaryCaseUpdateBatch
{
public:
    RimSummaryCaseUpdateBatch();
    ~RimSummaryCaseUpdateBatch();

    RimSummaryCaseUpdateBatch( const RimSummaryCaseUpdateBatch& )            = delete;
    RimSummaryCaseUpdateBatch& operator=( const RimSummaryCaseUpdateBatch& ) = delete;

    // Hand over detached cases for destruction at the flush. Destroys them immediately if no batch is active.
    static void orphan( const std::vector<RimDeltaSummaryCase*>& orphanedCases );

    // Schedule a regeneration of the derived cases of a delta ensemble. Regenerates immediately if no batch is active.
    static void markDeltaEnsembleDirty( RimDeltaSummaryEnsemble* deltaEnsemble );

    static bool isActive();

private:
    void flush();

    // The dirty ensembles and everything depending on them, ordered so a delta ensemble is regenerated before the delta
    // ensembles using it as a source
    std::vector<RimDeltaSummaryEnsemble*> ensemblesToRegenerate() const;

private:
    std::vector<caf::PdmPointer<RimDeltaSummaryCase>>     m_orphanedCases;
    std::vector<caf::PdmPointer<RimDeltaSummaryEnsemble>> m_dirtyEnsembles;

    static RimSummaryCaseUpdateBatch* sm_current;
};

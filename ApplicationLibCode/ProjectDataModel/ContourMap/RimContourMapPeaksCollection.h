/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

#include "cvfVector3.h"

#include <vector>

class RimContourMapPeak;

//==================================================================================================
/// Owns the set of computed "peaks" (local maxima) for a single contour map projection.
///
/// Each peak is stored as a RimContourMapPeak child, exposing its rank/value/prominence/position for
/// inspection in the project tree and property panel, and it owns (by reference) a small marker
/// polygon used for 3d visualization. See RimContourMapPeak for details.
//==================================================================================================
class RimContourMapPeaksCollection : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimContourMapPeaksCollection();

    RimContourMapPeak* addPeak( int rank, double value, double prominence, const cvf::Vec3d& domainPosition, double sphereRadiusFactor );
    void               clearPeaks();

    // Refresh the 3d views mirroring the polygon collection, so newly added/removed marker polygons
    // become visible. Call once after a batch of addPeak()/clearPeaks() calls.
    void updateVisualization();

    // Deletes the existing peaks and recomputes new ones for the owning contour map projection, using
    // the current peakCount/minDistance/minProminence field values.
    void computePeaks();

    // Flag the peaks as outdated in the name if the result, time step or value filter of the owning
    // contour map projection has changed since the peaks were computed.
    void updateOutdatedState();

    std::vector<RimContourMapPeak*> peaks() const;

private:
    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void    removePeaksWithMarkersSharedByOtherPeaks();
    QString currentResultSignature() const;

private:
    caf::PdmField<int>                          m_peakCount;
    caf::PdmField<double>                       m_minDistance;
    caf::PdmField<double>                       m_minProminence;
    caf::PdmField<QString>                      m_resultSignature;
    caf::PdmChildArrayField<RimContourMapPeak*> m_peaks;
};

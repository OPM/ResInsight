/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimReservoirGridEnsemble.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QString>

#include <vector>

class RimSumoDataSource;

//==================================================================================================
///
/// RimReservoirGridEnsembleSumo - Grid ensemble of one grid stored on Sumo, one case per realization
///
/// The realizations come from a RimSumoDataSource instead of an ensemble file set. As in the base
/// class the realization cases are not written to the project file, they are recreated from the data
/// source when the project is loaded, see createCaseObjects.
///
/// When all realizations report the same IJK dimensions, one RigMainGrid is kept in memory for the
/// whole ensemble. Every realization still downloads and parses its own grid blob to read its own
/// active cells, then releases the parsed geometry and points at the shared grid, see
/// RimRoffCaseSumo::openEclipseGridFile. Realizations are opened lazily, the first time one is
/// displayed; only the first realization is loaded when the ensemble is created.
///
//==================================================================================================
class RimReservoirGridEnsembleSumo : public RimReservoirGridEnsemble
{
    CAF_PDM_HEADER_INIT;

public:
    RimReservoirGridEnsembleSumo();

    // gridDimensionsAreIdentical decides whether the realizations can share one grid. It comes from the
    // dimensions reported by Sumo, so no grid has to be downloaded to decide.
    void setSumoSource( RimSumoDataSource*      dataSource,
                        const QString&          gridName,
                        const std::vector<int>& realizations,
                        bool                    gridDimensionsAreIdentical );

    // The realizations this grid exists for, which is not necessarily every realization of the ensemble.
    void             setGridRealizations( const std::vector<int>& realizations );
    std::vector<int> gridRealizations() const;

    QString sumoGridName() const;

    void createGridCasesFromSumoSource();

    bool doComputeMobileVolumeWeightedMean() const override;
    void setDoComputeMobileVolumeWeightedMean( bool enable ) override;

protected:
    void createCaseObjects() override;
    bool detectGridDimensionEquality() override;
    void loadGridsInSharedMode() override;
    void setupBeforeSave() override;

private:
    caf::PdmPtrField<RimSumoDataSource*> m_sumoDataSource;
    caf::PdmField<QString>               m_gridName;

    // The realizations to create cases for, in the order the cases appear. The cases themselves are not
    // written to the project file, see the class comment.
    caf::PdmField<std::vector<int>> m_realizations;

    caf::PdmField<std::vector<int>> m_gridRealizations;
    caf::PdmField<bool>             m_gridDimensionsAreIdentical;
    caf::PdmField<bool>             m_doComputeMobileVolumeWeightedMean;
};

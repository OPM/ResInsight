/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaPorosityModel.h"

#include "cvfBase.h"
#include "cvfMatrix4.h"
#include "cvfObject.h"

#include <vector>

class QString;

class RimEclipseCase;
class RigFractureCell;
class RigResultAccessor;
class RimFracture;

//==================================================================================================
///
///  Calculator used to compute the intersection areas between one RigFractureCell and Eclipse cells
///  Both active and inactive Eclipse cells are included. The transmissibility value for inactive cells are set to zero.
///  Eclipse reservoir cells open for flow is defined by reservoirCellIndicesOpenForFlow
///
//==================================================================================================
class RigEclipseToStimPlanCellTransmissibilityCalculator
{
public:
    explicit RigEclipseToStimPlanCellTransmissibilityCalculator(const RimEclipseCase*   caseToApply,
                                                                cvf::Mat4d              fractureTransform,
                                                                double                  skinFactor,
                                                                double                  cDarcy,
                                                                const RigFractureCell&  stimPlanCell,
                                                                const std::set<size_t>& reservoirCellIndicesOpenForFlow,
                                                                const RimFracture*      fracture);

    // These three vectors have the same size
    const std::vector<size_t>& globalIndiciesToContributingEclipseCells() const;
    const std::vector<double>& contributingEclipseCellTransmissibilities() const;
    const std::vector<double>& contributingEclipseCellIntersectionAreas() const;
    const std::vector<double>& contributingEclipseCellPermeabilities() const;

    double areaOpenForFlow() const;
    double aggregatedMatrixTransmissibility() const;

    const RigFractureCell& fractureCell() const;

    static std::vector<QString> requiredResultNames();
    static std::vector<QString> optionalResultNames();

private:
    void                calculateStimPlanCellsMatrixTransmissibility(const std::set<size_t>& reservoirCellIndicesOpenForFlow);
    std::vector<size_t> getPotentiallyFracturedCellsForPolygon(const std::vector<cvf::Vec3d>& polygon) const;

    static cvf::ref<RigResultAccessor> createResultAccessor(const RimEclipseCase* eclipseCase, const QString& uiResultName);

private:
    const RimEclipseCase*   m_case;
    const RimFracture*      m_fracture;

    double                 m_cDarcy;
    double                 m_fractureSkinFactor;
    cvf::Mat4d             m_fractureTransform;
    const RigFractureCell& m_stimPlanCell;

    // These three vectors have the same size
    std::vector<size_t> m_globalIndiciesToContributingEclipseCells;
    std::vector<double> m_contributingEclipseCellTransmissibilities;
    std::vector<double> m_contributingEclipseCellAreas;
    std::vector<double> m_contributingEclipseCellPermeabilities;
};

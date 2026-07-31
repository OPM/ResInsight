/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <utility>
#include <vector>

class RimIntersectionResultDefinition;
class RivIntersectionHexGridInterface;
class RimIntersectionResultsDefinitionCollection;
class RimSurfaceIntersectionBand;
class RimSurfaceIntersectionCollection;
class RimSurfaceIntersectionCurve;
class RivIntersectionGeometryGeneratorInterface;

namespace caf
{
class PdmUiTreeOrdering;
}

class RimIntersection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersection();
    ~RimIntersection() override;

    virtual QString name() const = 0;

    bool isActive() const;
    void setActive( bool isActive );
    bool isInactiveCellsVisible() const;

    RimIntersectionResultDefinition*          activeSeparateResultDefinition();
    cvf::ref<RivIntersectionHexGridInterface> createHexGridInterface();

    virtual const RivIntersectionGeometryGeneratorInterface* intersectionGeometryGenerator() const = 0;

    RimSurfaceIntersectionCollection*         surfaceIntersectionCollection() const;
    std::vector<RimSurfaceIntersectionCurve*> surfaceIntersectionCurves() const;
    std::vector<RimSurfaceIntersectionBand*>  surfaceIntersectionBands() const;
    RimSurfaceIntersectionCurve*              addIntersectionCurve();
    RimSurfaceIntersectionBand*               addIntersectionBand();

    /// True if the intersection is a vertical curtain, and therefore can display surface intersection
    /// curves and bands. Horizontal sections would require contouring instead of a vertical ray cast.
    virtual bool supportsSurfaceIntersectionCurves() const;

    /// The trace of the intersection in the XY plane, used to look up the surface below each point
    virtual std::vector<cvf::Vec3d> surfaceCurtainFootprint() const;

    /// The depth extent of the intersection, used to clip the surface intersection curves
    virtual std::pair<double, double> surfaceCurtainZRange() const;

    virtual void rebuildGeometryAndScheduleCreateDisplayModel();

protected:
    virtual RimIntersectionResultsDefinitionCollection* findSeparateResultsCollection();

    caf::PdmFieldHandle*          objectToggleField() final;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineSeparateDataSourceUi( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void updateDefaultSeparateDataSource();

    void appendSurfaceIntersectionsToTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering );

    caf::PdmField<bool>                                m_isActive;
    caf::PdmField<bool>                                m_showInactiveCells;
    caf::PdmField<bool>                                m_useSeparateDataSource;
    caf::PdmPtrField<RimIntersectionResultDefinition*> m_separateDataSource;

    caf::PdmChildField<RimSurfaceIntersectionCollection*> m_surfaceIntersections;

private:
    void onSurfaceIntersectionsChanged( const caf::SignalEmitter* emitter );
};
